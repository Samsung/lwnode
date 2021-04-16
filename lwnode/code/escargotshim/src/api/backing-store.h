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

#include <EscargotPublic.h>
#include <v8-internal.h>
#include <v8.h>
#include <map>
#include <memory>
#include "arraybuffer-deleter.h"

namespace EscargotShim {

// Whether the backing store is shared or not.
enum class SharedFlag : uint8_t { kNotShared, kShared };

// Whether the backing store memory is initialied to zero or not.
enum class InitializedFlag : uint8_t { kUninitialized, kZeroInitialized };

class IsolateWrap;

class BackingStoreWrap : public v8::internal::BackingStoreBase {
 public:
  BackingStoreWrap(void* data,
                   size_t byte_length,
                   SharedFlag shared,
                   std::unique_ptr<BufferDeleter> buffer_deleter);
  virtual ~BackingStoreWrap();

  void* Data() const { return data_; }
  size_t ByteLength() const { return byteLength_; }
  bool IsShared() const { return isShared_; }

  static std::unique_ptr<BackingStoreWrap> create(
      void* data,
      size_t byte_length,
      v8::BackingStore::DeleterCallback deleter,
      void* deleter_data);

  bool attachTo(Escargot::ExecutionStateRef* state,
                Escargot::ArrayBufferObjectRef* arrayBuffer);

  static const BackingStoreWrap* fromV8(const v8::BackingStore* backing_store) {
    return reinterpret_cast<const BackingStoreWrap*>(backing_store);
  }

  static BackingStoreWrap* fromV8(v8::BackingStore* backing_store) {
    return reinterpret_cast<BackingStoreWrap*>(backing_store);
  }

 private:
  void* data_ = nullptr;
  size_t byteLength_ = 0;
  bool isShared_ = false;
  std::unique_ptr<BufferDeleter> bufferDeleter_ = nullptr;
};

}  // namespace EscargotShim
