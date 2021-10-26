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

bool readFileFromArchive(const std::string& archiveFilename,
                         const std::string& filename,
                         char** buffer,
                         size_t* fileSize) {
  DCHECK_NOT_NULL(buffer);
  DCHECK_NOT_NULL(fileSize);

  ArchiveFileScope scope(archiveFilename.c_str());
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
      *buffer = (char*)allocateStringBuffer(fileInfo.uncompressed_size + 1);

      if (unzReadCurrentFile(file, *buffer, *fileSize) < 0) {
        freeStringBuffer(buffer);
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

FileData readFileFromArchive(std::string filename,
                             const Encoding fileEncoding) {
  CHECK(fileEncoding != UNKNOWN);

  size_t fileSize = 0;
  char* buffer = nullptr;

  static std::string s_externalBuiltinsPath;

  if (s_externalBuiltinsPath.empty()) {
    std::string executablePath = getSelfProcPath();
    executablePath = executablePath.substr(0, executablePath.rfind('/') + 1);
    s_externalBuiltinsPath = executablePath + LWNODE_EXTERNAL_BUILTINS_FILENAME;
  }

  if (readFileFromArchive(
          s_externalBuiltinsPath, filename, &buffer, &fileSize) == false) {
    return FileData();
  }

  if (fileEncoding == TWO_BYTE) {
    // treat non-ASCII as UTF-8 and encode as UTF-16 Little Endian.
    char* newStringBuffer = nullptr;
    size_t newStringBufferSize = 0;

    bool isConverted = convertUTF8ToUTF16le(
        &newStringBuffer, &newStringBufferSize, buffer, fileSize);

    // builtin scripts should be successfully converted.
    CHECK(isConverted);

    freeStringBuffer(buffer);
    buffer = newStringBuffer;
    fileSize = newStringBufferSize;
  }

  return FileData(buffer, fileSize, fileEncoding);
}

bool NativeModuleLoader::IsOneByte(const char* id) {
  const auto& it = source_.find(id);
  if (it == source_.end()) {
    CHECK(false);
  }
  return it->second.is_one_byte();
}

static std::string getFileNameOnArchive(const char* id) {
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

struct Stat {
  int loaded{0};
  int reloaded{0};
};
static Stat s_stat;

MaybeLocal<String> NativeModuleLoader::LoadExternalBuiltinSource(
    Isolate* isolate, const char* id) {
  std::string filename = getFileNameOnArchive(id);

  FileData fileData =
      readFileFromArchive(filename, (IsOneByte(id) ? ONE_BYTE : TWO_BYTE));

  if (fileData.buffer == nullptr) {
    ERROR_AND_ABORT("Failed to open builtins");
    return MaybeLocal<String>();
  }

  auto data = Loader::ReloadableSourceData::create(
      filename,
      fileData.buffer,
      fileData.size,
      (fileData.encoding == ONE_BYTE ? true : false));

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

        FileData fileData = readFileFromArchive(
            data->path(), (data->isOneByteString() ? ONE_BYTE : TWO_BYTE));

        CHECK_NOT_NULL(fileData.buffer);
        return fileData.buffer;
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
          freeStringBuffer(data->preloadedData);
          data->preloadedData = nullptr;
        }
        freeStringBuffer(preloadedData);
      });
}

}  // namespace native_module
}  // namespace node
