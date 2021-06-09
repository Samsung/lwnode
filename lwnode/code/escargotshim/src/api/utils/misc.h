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
#include "debug.h"
#include "flags.h"
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

/* CHECK */
// Use CHECK when abort should occurs if the condition fails
#define CHECK_FMT COLOR_REDBG "CHECK FAILED" COLOR_RESET " "

#define _LWNODE_CHECK_FAILED_HANDLER(msg, ...)                                 \
  LWNODE_LOG_RAW(CHECK_FMT msg "\n\t" TRACE_FMT, ##__VA_ARGS__, TRACE_ARGS);   \
  EscargotShim::DebugUtils::printStackTrace();                                 \
  std::abort();

#define LWNODE_CHECK_MSG(condition, msg, ...)                                  \
  do {                                                                         \
    if (LWNODE_UNLIKELY(!(condition))) {                                       \
      _LWNODE_CHECK_FAILED_HANDLER(msg, ##__VA_ARGS__);                        \
    }                                                                          \
  } while (0)

#define LWNODE_CHECK(condition) LWNODE_CHECK_MSG(condition, #condition)
#define LWNODE_CHECK_NULL(x) LWNODE_CHECK((x) == nullptr)
#define LWNODE_CHECK_NOT_NULL(x) LWNODE_CHECK((x) != nullptr)
#define LWNODE_CHECK_NOT_REACH_HERE()                                          \
  LWNODE_CHECK_MSG(                                                            \
      false, "MUST NOT REACH HERE at %s (%d)\n", __FILE__, __LINE__)

#if !defined(NDEBUG)
#define LWNODE_DCHECK(condition) LWNODE_CHECK_MSG(condition, #condition)
#define LWNODE_DCHECK_NULL(x) LWNODE_CHECK((x) == nullptr)
#define LWNODE_DCHECK_NOT_NULL(x) LWNODE_CHECK((x) != nullptr)
#define LWNODE_DCHECK_MSG(condition, msg, ...)                                 \
  LWNODE_CHECK_MSG(condition, msg, ##__VA_ARGS__)
#else
#define LWNODE_DCHECK(condition)
#define LWNODE_DCHECK_NULL(x)
#define LWNODE_DCHECK_NOT_NULL(x)
#define LWNODE_DCHECK_MSG(condition, msg, ...)
#endif

#if !defined(NDEBUG)
#define LWNODE_CALL_TRACE_ID(id, prefix, msg, ...)                             \
  if (EscargotShim::Flags::isTraceCallEnabled(id)) {                           \
    LWNODE_DLOG_RAW(COLOR_DIM "TRACE" prefix TRACE_FMT                         \
                              " " COLOR_RESET msg COLOR_RESET,                 \
                    TRACE_ARGS,                                                \
                    ##__VA_ARGS__);                                            \
  }
#define LWNODE_CALL_TRACE(msg, ...)                                            \
  LWNODE_CALL_TRACE_ID("1", "", msg, ##__VA_ARGS__);

#define LWNODE_CALL_TRACE_2(msg, ...)                                          \
  LWNODE_CALL_TRACE_ID("2", "\t", msg, ##__VA_ARGS__);

#define LWNODE_CALL_TRACE_3(msg, ...)                                          \
  LWNODE_CALL_TRACE_ID("3", "\t\t", msg, ##__VA_ARGS__);
#else
#define LWNODE_CALL_TRACE_ID(id, prefix, msg, ...)
#define LWNODE_CALL_TRACE(msg, ...)
#define LWNODE_CALL_TRACE_2(msg, ...)
#define LWNODE_CALL_TRACE_3(msg, ...)
#endif

#if !defined(NDEBUG)
#define LWNODE_ONCE(operation)                                                 \
  do {                                                                         \
    static bool once##__LINE__ = false;                                        \
    if (!once##__LINE__) {                                                     \
      operation;                                                               \
      once##__LINE__ = true;                                                   \
    }                                                                          \
  } while (0)
#else
#define LWNODE_ONCE(operation)
#endif
