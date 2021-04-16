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

#include "arraybuffer-deleter.h"
#include "es-helper.h"
#include "extra-data.h"
#include "isolate.h"

namespace EscargotShim {

ArrayBufferAllocatorDeleter::ArrayBufferAllocatorDeleter(
    v8::ArrayBuffer::Allocator* allocator) {
  LWNODE_CHECK_NOT_NULL(allocator);
  allocator_ = allocator;
}

void ArrayBufferAllocatorDeleter::Free(void* data, size_t length) {
  allocator_->Free(data, length);
}

ExternalBufferDeleter::ExternalBufferDeleter(
    v8::BackingStore::DeleterCallback deleter, void* deleter_data) {
  LWNODE_CHECK_NOT_NULL(deleter);
  LWNODE_CHECK_NOT_NULL(deleter_data);
  deleter_ = deleter;
  deleter_data_ = deleter_data;
}

void ExternalBufferDeleter::Free(void* data, size_t length) {
  deleter_(data, length, deleter_data_);
}

}
