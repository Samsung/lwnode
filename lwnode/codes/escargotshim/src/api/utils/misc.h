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

#include <assert.h>
#include "logger.h"

/* UNLIKELY */
#ifndef LWNODE_UNLIKELY
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define LWNODE_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define LWNODE_UNLIKELY(x) (x)
#endif
#endif

/* LIKELY */
#ifndef LWNODE_LIKELY
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define LWNODE_LIKELY(x) __builtin_expect((x), 1)
#else
#define LWNODE_LIKELY(x) (x)
#endif
#endif

#define LWNODE_ASSERT(assertion) assert(assertion)

// Use CHECK when abort should occurs if the condition fails
#define LWNODE_CHECK(condition)                                                \
  do {                                                                         \
    if (LWNODE_UNLIKELY(!(condition))) {                                       \
      LWNODE_LOG_ERROR("CHECK FAILED at %s (%s:%d)\n",                         \
                       __PRETTY_FUNCTION__,                                    \
                       __FILE__,                                               \
                       __LINE__);                                              \
      LWNODE_ASSERT(0);                                                        \
    }                                                                          \
  } while (false)

#define LWNODE_CHECK_NULL(x) LWNODE_CHECK((x) == nullptr)
#define LWNODE_CHECK_NOT_NULL(x) LWNODE_CHECK((x) != nullptr)
#define LWNODE_CHECK_GE(lhs, rhs) LWNODE_CHECK((lhs >= rhs))
