/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lwnode-loader.h"

#include <EscargotPublic.h>
#include <codecvt>
#include <fstream>
#include <locale>
#include <string>
#include "api.h"
#include "api/context.h"
#include "api/es-helper.h"
#include "api/isolate.h"
#include "api/utils/misc.h"
#include "api/utils/smaps.h"
#include "base.h"

using namespace EscargotShim;
using namespace Escargot;
using namespace v8;

static inline void* allocateMemory(size_t size) {
  return malloc(size);
}

static inline void freeMemory(void* ptr) {
  free(ptr);
}

namespace LWNode {

FileData Loader::readFile(std::string filename) {
  FILE* file = std::fopen(filename.c_str(), "rb");
  if (!file) {
    return FileData();
  }

  std::fseek(file, 0, SEEK_END);
  long pos = std::ftell(file);

  LWNODE_CHECK(pos != -1L);

  size_t fileSize = (size_t)pos;

  uint8_t* buffer = (uint8_t*)allocateMemory(fileSize + 1);

  std::rewind(file);
  std::fread(buffer, sizeof(uint8_t), fileSize, file);
  std::fclose(file);

  buffer[fileSize] = '\0';

  // 1. check if non-ascii
  bool isOneByteString = true;
  Encoding encoding = ONE_BYTE;

  for (size_t i = 0; i < fileSize; ++i) {
    if (buffer[i] > 127) {
      isOneByteString = false;
      break;
    }
  }

  void* stringBuffer = buffer;

  if (isOneByteString == false) {
    // 2. Treat non-ASCII as UTF-8 and encode as UTF-16 Little Endian.
    std::wstring_convert<
        std::codecvt_utf8_utf16<char16_t,
                                0x10ffff,
                                std::codecvt_mode::little_endian>,
        char16_t>
        convertor;
    std::u16string utf16 = convertor.from_bytes((const char*)buffer);

    LWNODE_DCHECK(convertor.converted() == fileSize);

    // 3. allocate buffer for utf16 string
    fileSize = utf16.size() * 2;
    uint8_t* newStringBuffer = (uint8_t*)allocateMemory(fileSize + 1);
    memcpy(newStringBuffer, utf16.data(), fileSize);
    newStringBuffer[fileSize] = '\0';

    freeMemory(buffer);
    stringBuffer = newStringBuffer;
    encoding = TWO_BYTE;
  }

  return FileData(stringBuffer, fileSize, encoding);
}

Loader::ReloadableSourceData* Loader::ReloadableSourceData::create(
    std::string sourcePath,
    void* preloadedData,
    size_t preloadedDataLength,
    bool isOneByteString) {
  // NOTE: data and data->path should be managed by gc
  auto data = new (Memory::gcMalloc(sizeof(ReloadableSourceData)))
      ReloadableSourceData();
  data->path_ = (char*)Memory::gcMallocAtomic(sourcePath.size() + 1);
  std::copy(sourcePath.begin(), sourcePath.end(), data->path_);
  data->path_[sourcePath.size()] = '\0';

  data->preloadedData = preloadedData;
  data->preloadedDataLength_ = preloadedDataLength;
  data->isOneByteString_ = isOneByteString;

  return data;
}

struct Stat {
  int loaded{0};
  int reloaded{0};
};
static Stat s_stat;

ValueRef* Loader::CreateReloadableSourceFromFile(ExecutionStateRef* state,
                                                 std::string fileName) {
  auto lwContext = ContextWrap::fromEscargot(state->context());
  auto isolate = lwContext->GetIsolate()->toV8();

  FileData dest = Loader::readFile(fileName);

  if (dest.buffer) {
    auto data = Loader::ReloadableSourceData::create(
        fileName, dest.buffer, dest.size, (dest.encoding == ONE_BYTE));

    HandleScope handleScope(isolate);

    v8::Local<v8::String> source =
        Loader::NewReloadableString(
            isolate,
            data,
            // Load-ReloadableSource
            [](void* userData) -> void* {
              auto data =
                  reinterpret_cast<Loader::ReloadableSourceData*>(userData);

              LWNODE_LOG_INFO("  * Load: %d (%d) %p %s (+%.2f kB)",
                              ++s_stat.loaded,
                              s_stat.reloaded,
                              data->preloadedData,
                              data->path(),
                              (float)data->preloadedDataLength() / 1024);

              if (data->preloadedData) {
                auto buffer = data->preloadedData;
                data->preloadedData = nullptr;
                return buffer;  // move memory ownership to js engine
              }
              s_stat.reloaded++;
              FileData dest = Loader::readFile(data->path());
              LWNODE_CHECK_NOT_NULL(dest.buffer);
              return dest.buffer;
            },
            // Unload-ReloadableSource
            [](void* preloadedData, void* userData) -> void {
              auto data =
                  reinterpret_cast<Loader::ReloadableSourceData*>(userData);

              LWNODE_LOG_INFO("* Unload: %d (%d) %p %s (-%.2f kB)",
                              --s_stat.loaded,
                              s_stat.reloaded,
                              preloadedData,
                              data->path(),
                              (float)data->preloadedDataLength() / 1024);

              if (data->preloadedData) {
                freeMemory(data->preloadedData);
                data->preloadedData = nullptr;
              }
              freeMemory(preloadedData);
            })
            .ToLocalChecked();

    return CVAL(*source)->value()->asString();
  }

  return ValueRef::createUndefined();
}

MaybeLocal<String> Loader::NewReloadableString(Isolate* isolate,
                                               ReloadableSourceData* data,
                                               LoadCallback loadCallback,
                                               UnloadCallback unloadCallback) {
  MaybeLocal<String> result;

  if (data->stringLength() == 0) {
    result = String::Empty(isolate);
  } else if (data->stringLength() > v8::String::kMaxLength) {
    result = MaybeLocal<String>();
  } else {
    Escargot::StringRef* reloadableString =
        Escargot::StringRef::createReloadableString(
            IsolateWrap::fromV8(isolate)->vmInstance(),
            data->isOneByteString(),
            data->stringLength(),
            data,  // data should be gc-managed.
            loadCallback,
            unloadCallback);
    result = v8::Utils::NewLocal<String>(isolate, reloadableString);
  }

  return result;
}

}  // namespace LWNode
