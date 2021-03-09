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
#include "utils/gc.h"

namespace EscargotShim {

class ArrayBufferAllocatorDecorator : public v8::ArrayBuffer::Allocator,
                                      public gc {
 public:
  void* Allocate(size_t length) override;
  void* AllocateUninitialized(size_t length) override;
  void* Reallocate(void* data, size_t old_length, size_t new_length) override;
  void Free(void* data, size_t length) override;

  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocate);
  v8::ArrayBuffer::Allocator* array_buffer_allocator() {
    return array_buffer_allocator_;
  }
  void printState();

 private:
  size_t currentMemorySize_ = 0;
  size_t peakMemorySize_ = 0;
  v8::ArrayBuffer::Allocator* array_buffer_allocator_ = nullptr;
};

}  // namespace EscargotShim
