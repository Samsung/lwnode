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
  LWNODE_LOG_RAW(CHECK_FMT msg "\n\t " TRACE_FMT, ##__VA_ARGS__, TRACE_ARGS);  \
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

#define FIRST_ARG(N, ...) N
#define LEFT_ARGS(N, ...) , ##__VA_ARGS__
#define TRACE_TAG_FMT COLOR_DIM "TRACE (%-10s)"
#define TRACE_TAG_ARG(id) std::string(id).substr(0, 10).c_str()
#define COUNTER_FMT "%s"
#define COUNTER_ARG(id) IndentCounter::getString(id).c_str()

#define LWNODE_CALL_TRACE_ID(id, ...)                                          \
  IndentCounter __counter(#id);                                                \
  if (EscargotShim::Flags::isTraceCallEnabled(#id)) {                          \
    LWNODE_DLOG_RAW(TRACE_TAG_FMT " " COUNTER_FMT TRACE_FMT                    \
                                  " " COLOR_RESET FIRST_ARG(__VA_ARGS__)       \
                                      COLOR_RESET,                             \
                    TRACE_TAG_ARG(#id),                                        \
                    COUNTER_ARG(#id),                                          \
                    TRACE_ARGS2 LEFT_ARGS(__VA_ARGS__));                       \
  }

#define LWNODE_CALL_TRACE_ID_LOG(id, ...)                                      \
  if (EscargotShim::Flags::isTraceCallEnabled(#id)) {                          \
    LWNODE_DLOG_RAW(TRACE_TAG_FMT                                              \
                    " " COUNTER_FMT COLOR_RESET FIRST_ARG(__VA_ARGS__)         \
                        COLOR_RESET,                                           \
                    TRACE_TAG_ARG(#id),                                        \
                    COUNTER_ARG(#id) LEFT_ARGS(__VA_ARGS__));                  \
  }

#define LWNODE_CALL_TRACE_ID_INDENT(id) IndentCounter::indent(#id);
#define LWNODE_CALL_TRACE_ID_UNINDENT(id) IndentCounter::unIndent(#id);

#define LWNODE_CALL_TRACE(msg, ...)                                            \
  LWNODE_CALL_TRACE_ID(COMMON, msg, ##__VA_ARGS__);
#define LWNODE_CALL_TRACE_LOG(msg, ...)                                        \
  LWNODE_CALL_TRACE_ID_LOG(COMMON, msg, ##__VA_ARGS__);
#define LWNODE_CALL_TRACE_INDENT() LWNODE_CALL_TRACE_ID_INDENT(COMMON);
#define LWNODE_CALL_TRACE_UNINDENT() LWNODE_CALL_TRACE_ID_UNINDENT(COMMON);

#define LWNODE_CALL_TRACE_GC_START(msg, ...)                                   \
  if (EscargotShim::Flags::isTraceCallEnabled("gc")) {                         \
    LWNODE_DLOG_INFO("GC: %s (%s:%d): " msg, TRACE_ARGS, ##__VA_ARGS__);       \
  }

#define LWNODE_CALL_TRACE_GC_END(msg, ...)                                     \
  if (EscargotShim::Flags::isTraceCallEnabled("gc")) {                         \
    LWNODE_DLOG_INFO("GC: /%s (%s:%d): " msg, TRACE_ARGS, ##__VA_ARGS__);      \
  }

#else
#define LWNODE_CALL_TRACE_ID(...)
#define LWNODE_CALL_TRACE_ID_LOG(...)
#define LWNODE_CALL_TRACE_ID_INDENT(...)
#define LWNODE_CALL_TRACE_ID_UNINDENT(...)

#define LWNODE_CALL_TRACE(...)
#define LWNODE_CALL_TRACE_LOG(...)
#define LWNODE_CALL_TRACE_INDENT()
#define LWNODE_CALL_TRACE_UNINDENT()
#define LWNODE_CALL_TRACE_GC_START(...)
#define LWNODE_CALL_TRACE_GC_END(...)
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
