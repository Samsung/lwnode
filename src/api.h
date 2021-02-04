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

#include <inttypes.h>  // For PRIu64
#include <signal.h>
#include <algorithm>  // For min
#include <cmath>      // For isnan.
#include <limits>
#include <string>
#include <utility>  // For move
#include <vector>

#include "v8-profiler.h"
#include "v8-util.h"

#include "api/context.h"
#include "api/flags.h"
#include "api/handle.h"
#include "api/isolate.h"
#include "api/es-helper.h"
#include "api/utils/string.h"

namespace i = v8::internal;

namespace v8 {}

namespace {

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  void* Allocate(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_calloc(length, 1);
#else
    void* data = calloc(length, 1);
#endif
    return data;
  }

  void* AllocateUninitialized(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_malloc(length);
#else
    void* data = malloc(length);
#endif
    return data;
  }

  void Free(void* data, size_t) override { free(data); }

  void* Reallocate(void* data, size_t old_length, size_t new_length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* new_data = __linux_realloc(data, new_length);
#else
    void* new_data = realloc(data, new_length);
#endif
    if (new_length > old_length) {
      memset(reinterpret_cast<uint8_t*>(new_data) + old_length,
             0,
             new_length - old_length);
    }
    return new_data;
  }
};
}  // namespace
