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

#include "arraybuffer-allocator.h"
#include "utils/misc.h"

namespace EscargotShim {

void ArrayBufferAllocatorDecorator::set_array_buffer_allocator(
    v8::ArrayBuffer::Allocator* allocate) {
  array_buffer_allocator_ = allocate;
}

void* ArrayBufferAllocatorDecorator::Allocate(size_t length) {
  LWNODE_CHECK_NOT_NULL(array_buffer_allocator_);
  currentMemorySize_ += length;
  if (currentMemorySize_ > peakMemorySize_) {
    peakMemorySize_ = currentMemorySize_;
  }

  LWNODE_DLOG_INFO(
      "malc: ab=%zuB | peak=%zuB", currentMemorySize_, peakMemorySize_);

  return array_buffer_allocator_->Allocate(length);
}

void* ArrayBufferAllocatorDecorator::AllocateUninitialized(size_t length) {
  LWNODE_CHECK_NOT_NULL(array_buffer_allocator_);
  currentMemorySize_ += length;
  if (currentMemorySize_ > peakMemorySize_) {
    peakMemorySize_ = currentMemorySize_;
  }

  LWNODE_DLOG_INFO(
      "malc: ab=%zuB | peak=%zuB", currentMemorySize_, peakMemorySize_);

  return array_buffer_allocator_->AllocateUninitialized(length);
}

void* ArrayBufferAllocatorDecorator::Reallocate(void* data,
                                                size_t old_length,
                                                size_t new_length) {
  LWNODE_CHECK_NOT_NULL(array_buffer_allocator_);
  currentMemorySize_ -= old_length;
  currentMemorySize_ += new_length;
  if (currentMemorySize_ > peakMemorySize_) {
    peakMemorySize_ = currentMemorySize_;
  }

  LWNODE_DLOG_INFO(
      "malc: ab=%zuB | peak=%zuB", currentMemorySize_, peakMemorySize_);

  return array_buffer_allocator_->Reallocate(data, old_length, new_length);
}

void ArrayBufferAllocatorDecorator::Free(void* data, size_t length) {
  LWNODE_CHECK_NOT_NULL(array_buffer_allocator_);
  currentMemorySize_ -= length;

  LWNODE_DLOG_INFO(
      "free: ab=%zuB | peak=%zuB", currentMemorySize_, peakMemorySize_);

  return array_buffer_allocator_->Free(data, length);
}

void ArrayBufferAllocatorDecorator::printState() {
  LWNODE_DLOG_INFO(
      "stat: ab=%zuB | peak: %zuB", currentMemorySize_, peakMemorySize_);
}

}  // namespace EscargotShim
