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

#include <v8.h>
#include <functional>
#include <memory>
#include <string>

namespace Escargot {
class ValueRef;
class ExecutionStateRef;
}  // namespace Escargot

namespace LWNode {

enum class Encoding : uint8_t { kUnknown, kAscii, kLatin1, kUtf16 };

struct FileData {
  void* buffer{nullptr};
  long int size{0};
  Encoding encoding{Encoding::kUnknown};
  std::string filePath;

  FileData(void* buffer_,
           long int size_,
           Encoding encoding_,
           const std::string& filePath_) {
    buffer = buffer_;
    size = size_;
    encoding = encoding_;
    filePath = filePath_;
  }
  FileData() = default;
};

class Loader {
 public:
  static FileData readFile(std::string filename, const Encoding fileEncoding);

  static FileData createFileDataForReloadableString(
      std::string filename,
      std::unique_ptr<void, std::function<void(void*)>> bufferHolder,
      size_t bufferSize,
      const Encoding encodingHint);

  // should return string buffer
  typedef void* (*LoadCallback)(void* callbackData);
  // should free memoryPtr
  typedef void (*UnloadCallback)(void* memoryPtr, void* callbackData);

  class ReloadableSourceData {
   public:
    void* preloadedData{nullptr};

    const char* path() { return path_; }
    size_t preloadedDataLength() { return preloadedDataLength_; }
    size_t stringLength() {
      return isOneByteString() ? preloadedDataLength_
                               : preloadedDataLength_ / 2;
    }
    bool isOneByteString() {
      return (encoding_ == Encoding::kAscii) ||
             (encoding_ == Encoding::kLatin1);
    }
    Encoding encoding() { return encoding_; }

    static ReloadableSourceData* create(const FileData fileData);

   private:
    char* path_{nullptr};
    size_t preloadedDataLength_{0};
    Encoding encoding_{Encoding::kUnknown};
    ReloadableSourceData() = default;
  };

  static Escargot::ValueRef* CreateReloadableSourceFromFile(
      Escargot::ExecutionStateRef* state, std::string fileName);

  static v8::MaybeLocal<v8::String> NewReloadableString(
      v8::Isolate* isolate,
      ReloadableSourceData* data,
      LoadCallback loadCallback,
      UnloadCallback unloadCallback);
};

bool convertUTF8ToUTF16le(char** buffer,
                          size_t* bufferSize,
                          const char* utf8Buffer,
                          const size_t utf8BufferSize);

void* allocateStringBuffer(size_t size);
void freeStringBuffer(void* ptr);

}  // namespace LWNode
