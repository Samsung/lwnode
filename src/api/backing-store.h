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
#include <memory>

namespace EscargotShim {

// Whether the backing store is shared or not.
enum class SharedFlag : uint8_t { kNotShared, kShared };

// Whether the backing store memory is initialied to zero or not.
enum class InitializedFlag : uint8_t { kUninitialized, kZeroInitialized };

class IsolateWrap;

class BackingStoreWrap : public v8::internal::BackingStoreBase {
 public:
  ~BackingStoreWrap();

  void* buffer() const { return buffer_; }
  size_t byteLength() const { return byteLength_; }
  bool isShared() const { return isShared_; }

  static std::unique_ptr<BackingStoreWrap> create(IsolateWrap* isolate,
                                                  size_t byte_length,
                                                  SharedFlag shared,
                                                  InitializedFlag initialized);
  bool attachTo(Escargot::ExecutionStateRef* state,
                Escargot::ArrayBufferObjectRef* arrayBuffer);

 private:
  BackingStoreWrap(void* buffer, size_t byte_length, SharedFlag shared);
  void clear();

  void* buffer_;
  size_t byteLength_;
  bool isShared_;
};

}  // namespace EscargotShim
