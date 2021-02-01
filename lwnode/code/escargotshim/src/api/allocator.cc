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

#include "allocator.h"
#include "utils/misc.h"

namespace EscargotShim {

size_t Allocator::currentMemorySize = 0;
size_t Allocator::peakMemorySize = 0;

void* Allocator::Allocate(size_t length) {
  currentMemorySize += length;
  if (currentMemorySize > peakMemorySize) peakMemorySize = currentMemorySize;

  return calloc(length, sizeof(void*));
}

void* Allocator::AllocateUninitialized(size_t length) {
  currentMemorySize += length;
  if (currentMemorySize > peakMemorySize) peakMemorySize = currentMemorySize;

  return malloc(length, sizeof(void*));
}

void Allocator::Free(void* data, size_t length) {
  currentMemorySize -= length;

  free(data);
}

void* Allocator::Reallocate(void* data, size_t old_length, size_t new_length) {
  LWNODE_CHECK_NOT_NULL(data);
  currentMemorySize -= old_length;
  currentMemorySize += new_length;
  if (currentMemorySize > peakMemorySize) peakMemorySize = currentMemorySize;

  return realloc(data, new_length);
}

Allocator* Allocator::NewDefaultAllocator() {
  return new Allocator();
}

}  // namespace EscargotShim
