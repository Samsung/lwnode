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
#include <map>
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

class ArchiveFileScope {
 public:
  ArchiveFileScope() = default;
  ArchiveFileScope(const char* path) { open(path); }
  ~ArchiveFileScope() {
    if (file_) {
      unzClose(file_);
    }
  }

  void open(const char* path) {
    if (file_ == nullptr) {
      file_ = unzOpen(path);
    }
  }

  bool isFileOpened() { return (file_ != nullptr) ? true : false; }
  unzFile file() { return file_; }

 private:
  unzFile file_{nullptr};
};

struct UnzFileCachedInfo {
  unz_file_pos position{0, 0};
  uLong uncompressedSize{0};
};

enum class ReaderError {
  NO_ERROR = 0,
  UNZ_OPEN_CURRENTFILE,
  UNZ_GOTO_FIRSTFILE,
  UNZ_GET_CURRENTFILEINFO,
  READ_CURRENTFILE_FROMARCHIVE,
  READ_FILE_FROMARCHIVE
};

static thread_local ArchiveFileScope s_archiveFileScope;
static thread_local std::map<std::string, UnzFileCachedInfo>
    s_unzFileInfoDictionary;
static thread_local ReaderError s_lastError = ReaderError::NO_ERROR;

std::string getSelfProcPath() {
  char path[PATH_MAX + 1];
  ssize_t length = readlink("/proc/self/exe", path, PATH_MAX);
  if (length < 0) {
    ERROR_AND_ABORT("readlink fails");
  }
  path[length] = '\0';
  return std::string(path);
}

void setError(ReaderError error) {
  s_lastError = error;
  ERROR_AND_ABORT(s_lastError);
}

bool readCurrentFileFromArchive(const unzFile file,
                                uLong uncompressedSize,
                                char** buffer,
                                size_t* fileSize) {
  if (unzOpenCurrentFile(file) < 0) {
    setError(ReaderError::UNZ_OPEN_CURRENTFILE);
    return false;
  }

  *fileSize = uncompressedSize;
  *buffer = (char*)allocateStringBuffer(uncompressedSize + 1);

  bool result = true;

  if (unzReadCurrentFile(file, *buffer, *fileSize) < 0) {
    freeStringBuffer(buffer);
    buffer = nullptr;
    result = false;
  } else {
    (*buffer)[*fileSize] = '\0';
  }

  unzCloseCurrentFile(file);
  return result;
}

bool readFileFromArchive(const std::string& archiveFilename,
                         const std::string& filename,
                         char** buffer,
                         size_t* fileSize) {
  DCHECK_NOT_NULL(buffer);
  DCHECK_NOT_NULL(fileSize);

  if (s_archiveFileScope.isFileOpened() == false) {
    s_archiveFileScope.open(archiveFilename.c_str());
  }

  const unzFile file = s_archiveFileScope.file();
  CHECK_NOT_NULL(file);

  // 1.1 check if the cache on this filename exists
  const auto& it = s_unzFileInfoDictionary.find(filename);
  if (it != s_unzFileInfoDictionary.end()) {
    UnzFileCachedInfo cache = it->second;

    // 1.2 move the file position using the cached info and read the data
    unzGoToFilePos(file, &cache.position);
    return readCurrentFileFromArchive(
        file, cache.uncompressedSize, buffer, fileSize);
  }

  // 2. read the data by searching the file position from the first one.
  if (unzGoToFirstFile(file) < 0) {
    setError(ReaderError::UNZ_OPEN_CURRENTFILE);
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
      setError(ReaderError::UNZ_GET_CURRENTFILEINFO);
      return false;
    }

    if (filename.compare(currentFileName) == 0) {
      isFileFound = true;

      // 2.1 read the data from the current file poistion
      if (readCurrentFileFromArchive(
              file, fileInfo.uncompressed_size, buffer, fileSize) == false) {
        setError(ReaderError::READ_CURRENTFILE_FROMARCHIVE);
        return false;
      }

      // 2.2 create the cache for this file and register it to the dictionary
      UnzFileCachedInfo cache;
      auto result = unzGetFilePos(file, &cache.position);
      CHECK(result == UNZ_OK);
      cache.uncompressedSize = fileInfo.uncompressed_size;

      s_unzFileInfoDictionary[filename] = cache;
      break;
    }

  } while (unzGoToNextFile(file) != UNZ_END_OF_LIST_OF_FILE);

  return isFileFound;
}

FileData readFileFromArchive(std::string filename,
                             const Encoding encodingHint) {
  CHECK(encodingHint != Encoding::kUnknown);

  size_t bufferSize = 0;
  char* buffer = nullptr;

  static std::string s_externalBuiltinsPath;

  if (s_externalBuiltinsPath.empty()) {
    std::string executablePath = getSelfProcPath();
    executablePath = executablePath.substr(0, executablePath.rfind('/') + 1);
    s_externalBuiltinsPath = executablePath + LWNODE_EXTERNAL_BUILTINS_FILENAME;
  }

  if (readFileFromArchive(
          s_externalBuiltinsPath, filename, &buffer, &bufferSize) == false) {
    setError(ReaderError::READ_FILE_FROMARCHIVE);
    return FileData();
  }

  std::unique_ptr<void, std::function<void(void*)>> bufferHolder(
      buffer, freeStringBuffer);

  return Loader::createFileDataForReloadableString(
      filename, std::move(bufferHolder), bufferSize, encodingHint);
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

class SourceReaderOnArchive : public SourceReaderInterface {
 public:
  static SourceReaderOnArchive* getInstance() {
    static SourceReaderOnArchive s_singleton;
    return &s_singleton;
  }

  FileData read(std::string filename, const Encoding encodingHint) {
    CHECK(encodingHint != Encoding::kUnknown);

    size_t bufferSize = 0;
    char* buffer = nullptr;

    static std::string s_externalBuiltinsPath;

    if (s_externalBuiltinsPath.empty()) {
      std::string executablePath = getSelfProcPath();
      executablePath = executablePath.substr(0, executablePath.rfind('/') + 1);
      s_externalBuiltinsPath =
          executablePath + LWNODE_EXTERNAL_BUILTINS_FILENAME;
    }

    if (readFileFromArchive(
            s_externalBuiltinsPath, filename, &buffer, &bufferSize) == false) {
      LWNODE_LOG_ERROR("readFileFromArchive (%s) failed:", filename);
      setError(ReaderError::READ_FILE_FROMARCHIVE);
      return FileData();
    }

    std::unique_ptr<void, std::function<void(void*)>> bufferHolder(
        buffer, freeStringBuffer);

    return Loader::createFileDataForReloadableString(
        filename, std::move(bufferHolder), bufferSize, encodingHint);
  }

 private:
  SourceReaderOnArchive() = default;
};

MaybeLocal<String> NativeModuleLoader::LoadExternalBuiltinSource(
    Isolate* isolate, const char* id) {
  std::string filename = getFileNameOnArchive(id);

  auto sourceReader = SourceReaderOnArchive::getInstance();

  FileData fileData = sourceReader->read(
      filename, (IsOneByte(id) ? Encoding::kAscii : Encoding::kUtf16));

  if (fileData.buffer == nullptr) {
    ERROR_AND_ABORT("Failed to open builtins");
    return MaybeLocal<String>();
  }

  return Loader::NewReloadableString(
      isolate, Loader::ReloadableSourceData::create(fileData, sourceReader));
}

}  // namespace native_module
}  // namespace node
