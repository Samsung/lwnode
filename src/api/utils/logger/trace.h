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

#include "flags.h"
#include "logger.h"

#if !defined(NDEBUG)

#define FIRST_ARG(N, ...) N
#define LEFT_ARGS(N, ...) , ##__VA_ARGS__
#define TRACE_TAG_FMT CLR_DIM "TRACE (%-10s)"
#define TRACE_TAG_ARG(id) std::string(id).substr(0, 10).c_str()
#define COUNTER_FMT "%s"
#define COUNTER_ARG(id) IndentCounter::getString(id).c_str()

#define LWNODE_CALL_TRACE_ID_LOG(id, ...)                                      \
  if (EscargotShim::Flags::isTraceCallEnabled(#id)) {                          \
    LWNODE_DLOG_RAW(TRACE_TAG_FMT " " COUNTER_FMT TRACE_FMT                    \
                                  " " CLR_RESET FIRST_ARG(__VA_ARGS__)         \
                                      CLR_RESET,                               \
                    TRACE_TAG_ARG(#id),                                        \
                    COUNTER_ARG(#id),                                          \
                    TRACE_ARGS2 LEFT_ARGS(__VA_ARGS__));                       \
  }

#define LWNODE_CALL_TRACE_ID(id, ...)                                          \
  IndentCounter __counter(#id);                                                \
  LWNODE_CALL_TRACE_ID_LOG(id, __VA_ARGS__)

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
