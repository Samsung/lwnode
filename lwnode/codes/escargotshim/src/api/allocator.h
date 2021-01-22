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

namespace EscargotShim {

/*
    Note:
    NodeArrayBufferAllocator, DebuggingArrayBufferAllocator (Node.js)
*/

class Allocator : public v8::ArrayBuffer::Allocator {
 public:
  void* Allocate(size_t length) override;
  void* AllocateUninitialized(size_t length) override;
  void Free(void* data, size_t length) override;
  void* Reallocate(void* data, size_t old_length, size_t new_length) override;

  static Allocator* NewDefaultAllocator();

  static v8::ArrayBuffer::Allocator* toV8(Allocator* alloc) {
    return reinterpret_cast<v8::ArrayBuffer::Allocator*>(alloc);
  }

  static Allocator* fromV8(v8::ArrayBuffer::Allocator* alloc) {
    return reinterpret_cast<Allocator*>(alloc);
  }

 private:
  static size_t currentMemorySize = 0;
  static size_t peakMemorySize = 0;
};

}  // namespace EscargotShim
