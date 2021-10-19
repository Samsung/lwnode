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

#pragma once

#include <unzip.h>
#include <codecvt>
#include <locale>
#include "lwnode-loader.h"
#include "lwnode.h"
#include "node_native_module.h"
#include "trace.h"

using namespace LWNode;

namespace node {
namespace native_module {
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::String;

std::string getSelfProcPath() {
  char path[PATH_MAX];
  ssize_t length = readlink("/proc/self/exe", path, PATH_MAX);
  if (length < 0) {
    ERROR_AND_ABORT("readlink fails");
  }
  path[length] = '\0';
  return std::string(path);
}

class ArchiveFileScope {
 public:
  ArchiveFileScope(const char* path) { file_ = unzOpen(path); }
  ~ArchiveFileScope() { unzClose(file_); }
  unzFile file() { return file_; }

 private:
  unzFile file_{nullptr};
};

bool readFileFromArchive(std::string filename,
                         char** buffer,
                         size_t* fileSize) {
  DCHECK_NOT_NULL(buffer);
  DCHECK_NOT_NULL(fileSize);

  static std::string s_externalBuiltinsPath;
  if (s_externalBuiltinsPath.empty()) {
    std::string executablePath = getSelfProcPath();
    executablePath = executablePath.substr(0, executablePath.rfind('/') + 1);
    s_externalBuiltinsPath = executablePath + LWNODE_EXTERNAL_BUILTINS_FILENAME;
  }

  ArchiveFileScope scope(s_externalBuiltinsPath.c_str());
  const unzFile file = scope.file();
  CHECK_NOT_NULL(file);

  if (unzGoToFirstFile(file) < 0) {
    return false;
  }

  bool isFileFound = false;
  do {
    unz_file_info fileInfo;
    char currentFileName[PATH_MAX];

    if (unzGetCurrentFileInfo(file,
                              &fileInfo,
                              currentFileName,
                              sizeof(currentFileName),
                              nullptr,
                              0,
                              nullptr,
                              0) < 0) {
      return false;
    }

    if (filename.compare(currentFileName) == 0) {
      isFileFound = true;

      if (unzOpenCurrentFile(file) < 0) {
        return false;
      }

      *fileSize = fileInfo.uncompressed_size;
      *buffer = (char*)malloc(fileInfo.uncompressed_size + 1);

      if (unzReadCurrentFile(file, *buffer, *fileSize) < 0) {
        free(buffer);
        buffer = nullptr;
        return false;
      }
      (*buffer)[*fileSize] = '\0';

      unzCloseCurrentFile(file);
      break;
    }

  } while (unzGoToNextFile(file) != UNZ_END_OF_LIST_OF_FILE);

  return isFileFound;
}

bool NativeModuleLoader::IsOneByte(const char* id) {
  const auto& it = source_.find(id);
  if (it == source_.end()) {
    CHECK(false);
  }
  return it->second.is_one_byte();
}

static std::string OnArchiveFileName(const char* id) {
  std::string filename;
  if (strncmp(id, "internal/deps", strlen("internal/deps")) == 0) {
    id += strlen("internal/");
  } else {
    filename += "lib/";
  }
  filename += id;
  filename += ".js";
  return filename;
}

static void convert_utf8_to_utf16le(char** buffer,
                                    size_t* bufferSize,
                                    const char* utf8Buffer,
                                    const size_t utf8BufferSize) {
  DCHECK_NOT_NULL(buffer);
  DCHECK_NOT_NULL(bufferSize);

  std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t,
                              0x10ffff,
                              std::codecvt_mode::little_endian>,
      char16_t>
      convertor;
  std::u16string utf16 = convertor.from_bytes(utf8Buffer);

  if (convertor.converted() < utf8BufferSize) {
    LWNODE_LOG_ERROR("Invalid conversion");
    std::abort();
  }

  size_t utf16Size = utf16.size() * 2;

  *buffer = (char*)malloc(utf16Size + 1);
  memcpy(*buffer, utf16.data(), utf16Size);
  (*buffer)[utf16Size] = '\0';

  *bufferSize = utf16Size;
}

MaybeLocal<String> NativeModuleLoader::LoadExternalBuiltinSource(
    Isolate* isolate, const char* id) {
  std::string filename = OnArchiveFileName(id);

  size_t fileSize = 0;
  char* buffer = nullptr;
  if (readFileFromArchive(filename, &buffer, &fileSize) == false) {
    ERROR_AND_ABORT("Failed to open builtins");
    return MaybeLocal<String>();
  }

  struct Stat {
    int loaded{0};
    int reloaded{0};
  };
  static Stat s_stat;

  bool is8BitString = true;
  if (IsOneByte(id) == false) {
    is8BitString = false;

    // treat non-ASCII as UTF-8 and encode as UTF-16 Little Endian.
    char* newStringBuffer = nullptr;
    size_t newStringBufferSize = 0;

    convert_utf8_to_utf16le(
        &newStringBuffer, &newStringBufferSize, buffer, fileSize);

    free(buffer);
    buffer = newStringBuffer;
    fileSize = newStringBufferSize;
  }

  auto data = Loader::ReloadableSourceData::create(
      filename, buffer, fileSize, is8BitString);

  return Loader::NewReloadableString(
      isolate,
      data,
      // Load-ReloadableSource
      [](void* userData) -> void* {
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
        size_t fileSize = 0;
        char* buffer = nullptr;
        bool result = readFileFromArchive(data->path(), &buffer, &fileSize);
        if (data->isOneByteString() == false) {
          // treat non-ASCII as UTF-8 and encode as UTF-16 Little Endian.
          char* newStringBuffer = nullptr;
          size_t newStringBufferSize = 0;

          convert_utf8_to_utf16le(
              &newStringBuffer, &newStringBufferSize, buffer, fileSize);

          free(buffer);
          buffer = newStringBuffer;
          fileSize = newStringBufferSize;
        }

        CHECK(result);
        return buffer;
      },
      // Unload-ReloadableSource
      [](void* preloadedData, void* userData) -> void {
        auto data = reinterpret_cast<Loader::ReloadableSourceData*>(userData);

        LWNODE_LOG_INFO("Unload: %d (%d) %p %s (-%.2f kB)",
                        --s_stat.loaded,
                        s_stat.reloaded,
                        preloadedData,
                        data->path(),
                        (float)data->preloadedDataLength() / 1024);

        if (data->preloadedData) {
          free(data->preloadedData);
          data->preloadedData = nullptr;
        }
        free(preloadedData);
      });
}

}  // namespace native_module
}  // namespace node
