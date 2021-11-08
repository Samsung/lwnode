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
#include "api/utils/string-util.h"
#include "base.h"

using namespace EscargotShim;
using namespace Escargot;
using namespace v8;

namespace LWNode {

void* allocateStringBuffer(size_t size) {
  return malloc(size);
}

void freeStringBuffer(void* ptr) {
  free(ptr);
}

bool convertUTF8ToUTF16le(char** buffer,
                          size_t* bufferSize,
                          const char* utf8Buffer,
                          const size_t utf8BufferSize) {
  LWNODE_CHECK_NOT_NULL(buffer);
  LWNODE_CHECK_NOT_NULL(bufferSize);

  std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t,
                              0x10ffff,
                              std::codecvt_mode::little_endian>,
      char16_t>
      convertor;
  std::u16string utf16 = convertor.from_bytes(utf8Buffer);

  if (convertor.converted() < utf8BufferSize) {
    LWNODE_LOG_ERROR("Invalid conversion");
    return false;
  }

  size_t utf16Size = utf16.size() * 2;

  *buffer = (char*)allocateStringBuffer(utf16Size + 1);
  memcpy(*buffer, utf16.data(), utf16Size);
  (*buffer)[utf16Size] = '\0';

  *bufferSize = utf16Size;
  return true;
}

class FileScope {
 public:
  FileScope(const char* path, const char* mode) {
    file_ = std::fopen(path, mode);
  }
  ~FileScope() { std::fclose(file_); }
  std::FILE* file() { return file_; }

 private:
  std::FILE* file_{nullptr};
};

static void tryConvertUTF8ToLatin1(
    std::basic_string<uint8_t, std::char_traits<uint8_t>>& latin1String,
    Encoding& encoding,
    const uint8_t* buffer,
    const size_t bufferSize,
    const Encoding encodingHint) {
  bool isOneByteString = true;

  if (encodingHint == Encoding::kUnknown || encodingHint == Encoding::kLatin1) {
    if (UTF8Sequence::convertUTF8ToLatin1(
            latin1String, buffer, buffer + bufferSize) == false) {
      isOneByteString = false;
    }
  } else if (encodingHint == Encoding::kUtf16) {
    isOneByteString = false;
  }

  encoding = Encoding::kUnknown;

  if (isOneByteString) {
    if (latin1String.length() == bufferSize) {
      encoding = Encoding::kAscii;
    } else {
      encoding = Encoding::kLatin1;
    }
  } else {
    encoding = Encoding::kUtf16;
  }
}

FileData Loader::createFileDataForReloadableString(
    std::string filename,
    std::unique_ptr<void, std::function<void(void*)>> bufferHolder,
    size_t bufferSize,
    const Encoding encodingHint) {
  std::basic_string<uint8_t, std::char_traits<uint8_t>> latin1String;
  Encoding encoding = Encoding::kUnknown;

  if (encodingHint == Encoding::kAscii) {
    encoding = Encoding::kAscii;
  } else if (encodingHint == Encoding::kUtf16) {
    encoding = Encoding::kUtf16;
  } else if (encodingHint == Encoding::kLatin1) {
    tryConvertUTF8ToLatin1(latin1String,
                           encoding,
                           (uint8_t*)bufferHolder.get(),
                           bufferSize,
                           encodingHint);
  } else {
    LWNODE_CHECK(encodingHint == Encoding::kUnknown);
    tryConvertUTF8ToLatin1(latin1String,
                           encoding,
                           (uint8_t*)bufferHolder.get(),
                           bufferSize,
                           encodingHint);
  }

  if (encoding == Encoding::kUtf16) {
    // Treat non-latin1 as UTF-8 and encode it as UTF-16 Little Endian.
    if (encodingHint == Encoding::kUnknown) {
      LWNODE_LOG_INFO("%s contains characters outside of the Latin1 range.",
                      filename.c_str());
    }

    char* newStringBuffer = nullptr;
    size_t newStringBufferSize = 0;

    bool isConverted = convertUTF8ToUTF16le(&newStringBuffer,
                                            &newStringBufferSize,
                                            (const char*)bufferHolder.get(),
                                            bufferSize);
    if (isConverted == false) {
      return FileData();
    }

    bufferHolder.reset(newStringBuffer);
    bufferSize = newStringBufferSize;
  } else {
    if (encoding == Encoding::kLatin1) {
      if (encodingHint == Encoding::kUnknown) {
        LWNODE_LOG_INFO("%s contains Latin1 characters.", filename.c_str());
      }

      bufferSize = latin1String.length();
      bufferHolder.reset(allocateStringBuffer(bufferSize + 1));
      ((uint8_t*)bufferHolder.get())[bufferSize] = '\0';

      memcpy(bufferHolder.get(), latin1String.data(), bufferSize);
    }
  }

  return FileData(bufferHolder.release(), bufferSize, encoding, filename);
}

SourceReader* SourceReader::getInstance() {
  static SourceReader s_singleton;
  return &s_singleton;
}

FileData SourceReader::read(std::string filename, const Encoding encodingHint) {
  FileScope fileScope(filename.c_str(), "rb");

  std::FILE* file = fileScope.file();

  if (!file) {
    return FileData();
  }

  std::fseek(file, 0, SEEK_END);
  long pos = std::ftell(file);
  std::rewind(file);

  LWNODE_CHECK(pos >= 0);

  size_t bufferSize = (size_t)pos;
  uint8_t* buffer = (uint8_t*)allocateStringBuffer(bufferSize + 1);
  buffer[bufferSize] = '\0';

  std::unique_ptr<void, std::function<void(void*)>> bufferHolder(
      buffer, freeStringBuffer);

  if (std::fread(buffer, sizeof(uint8_t), bufferSize, file) == 0) {
    return FileData();
  }

  return Loader::createFileDataForReloadableString(
      filename, std::move(bufferHolder), bufferSize, encodingHint);
}

Loader::ReloadableSourceData* Loader::ReloadableSourceData::create(
    const FileData fileData, SourceReaderInterface* sourceReader) {
  // NOTE: data and data->path should be managed by gc
  auto data = new (Memory::gcMalloc(sizeof(ReloadableSourceData)))
      ReloadableSourceData();

  auto& sourcePath = fileData.filePath;
  data->path_ = (char*)Memory::gcMallocAtomic(sourcePath.size() + 1);
  std::copy(sourcePath.begin(), sourcePath.end(), data->path_);
  data->path_[sourcePath.size()] = '\0';

  data->preloadedData = fileData.buffer;
  data->preloadedDataLength_ = fileData.size;
  data->encoding_ = fileData.encoding;
  data->sourceReader_ = sourceReader;

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

  auto sourceReader = SourceReader::getInstance();
  FileData fileData = sourceReader->read(fileName, Encoding::kUnknown);

  if (fileData.buffer == nullptr) {
    return ValueRef::createUndefined();
  }

  HandleScope handleScope(isolate);

  v8::Local<v8::String> source =
      NewReloadableString(isolate,
                          ReloadableSourceData::create(fileData, sourceReader))
          .ToLocalChecked();

  return CVAL(*source)->value()->asString();
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
    if (loadCallback == nullptr && unloadCallback == nullptr) {
      // set default load/unload callbacks
      loadCallback = [](void* userData) -> void* {
        auto data = reinterpret_cast<Loader::ReloadableSourceData*>(userData);

        LWNODE_LOG_INFO("  Load: %d (%d) %p %s (+%.2f kB)",
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

        FileData fileData =
            data->sourceReader()->read(data->path(), data->encoding());

        LWNODE_CHECK_NOT_NULL(fileData.buffer);
        return fileData.buffer;
      };

      unloadCallback = [](void* preloadedData, void* userData) -> void {
        auto data = reinterpret_cast<Loader::ReloadableSourceData*>(userData);

        LWNODE_LOG_INFO(" Unload: %d (%d) %p %s (-%.2f kB)",
                        --s_stat.loaded,
                        s_stat.reloaded,
                        preloadedData,
                        data->path(),
                        (float)data->preloadedDataLength() / 1024);

        if (data->preloadedData) {
          freeStringBuffer(data->preloadedData);
          data->preloadedData = nullptr;
        }
        freeStringBuffer(preloadedData);
      };
    }

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
