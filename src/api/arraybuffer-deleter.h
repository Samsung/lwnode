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

class BufferDeleter {
 public:
  virtual ~BufferDeleter() = default;
  virtual void Free(void* data, size_t length) = 0;
};

class ArrayBufferAllocatorDeleter : public BufferDeleter {
 public:
  ArrayBufferAllocatorDeleter(v8::ArrayBuffer::Allocator* allocator);

  void Free(void* data, size_t length) override;

 private:
  v8::ArrayBuffer::Allocator* allocator_ = nullptr;
};

class ExternalBufferDeleter : public BufferDeleter {
 public:
  ExternalBufferDeleter(v8::BackingStore::DeleterCallback deleter,
                        void* deleter_data);
  void Free(void* data, size_t length) override;

 private:
  v8::BackingStore::DeleterCallback deleter_ = nullptr;
  void* deleter_data_ = nullptr;
};

}  // namespace EscargotShim
