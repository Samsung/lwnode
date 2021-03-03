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

namespace EscargotShim {

// Whether the backing store is shared or not.
enum class SharedFlag : uint8_t { kNotShared, kShared };

// Whether the backing store memory is initialied to zero or not.
enum class InitializedFlag : uint8_t { kUninitialized, kZeroInitialized };

class IsolateWrap;
class BackingStoreWrap;

// --- BackingStoreWrapHolder ---

class BackingStoreWrapHolder : public v8::internal::BackingStoreBase {
 public:
  BackingStoreWrapHolder(BackingStoreWrap* backingStore = nullptr);
  virtual ~BackingStoreWrapHolder();

  std::shared_ptr<BackingStoreWrapHolder> clone();
  BackingStoreWrap* backingStore() const { return backingStore_; }

 private:
  BackingStoreWrap* backingStore_ = nullptr;
  static std::map<BackingStoreWrap*, u_int8_t> map_;
};

// --- BackingStoreWrap ---

class BackingStoreWrap : public v8::internal::BackingStoreBase {
 public:
  BackingStoreWrap(void* data,
                   size_t byte_length,
                   SharedFlag shared,
                   v8::ArrayBuffer::Allocator* allocator);
  virtual ~BackingStoreWrap();

  void* Data() const { return data_; }
  size_t ByteLength() const { return byteLength_; }
  bool IsShared() const { return isShared_; }

  static std::unique_ptr<BackingStoreWrap> create(IsolateWrap* isolate,
                                                  size_t byte_length,
                                                  SharedFlag shared,
                                                  InitializedFlag initialized);
  bool attachTo(Escargot::ExecutionStateRef* state,
                Escargot::ArrayBufferObjectRef* arrayBuffer);

  static const BackingStoreWrap* fromV8(const v8::BackingStore* backing_store) {
    return reinterpret_cast<const BackingStoreWrapHolder*>(backing_store)
        ->backingStore();
  }

  static BackingStoreWrap* fromV8(v8::BackingStore* backing_store) {
    return reinterpret_cast<BackingStoreWrapHolder*>(backing_store)
        ->backingStore();
  }

 private:
  void clear();

  void* data_ = nullptr;
  size_t byteLength_ = 0;
  bool isShared_ = false;
  v8::ArrayBuffer::Allocator* allocator_ = nullptr;
};

}  // namespace EscargotShim
