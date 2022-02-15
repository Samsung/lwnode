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

// LWNODE_CALL_TRACE with ID
#define LWNODE_CALL_TRACE_ID_LOG(id, ...)                                      \
  Logger(LogTRACE(#id, __PRETTY_FUNCTION__, __FILE_NAME__, __LINE__))          \
      .print(__VA_ARGS__)

#define LWNODE_CALL_TRACE_ID(id, ...)                                          \
  IndentCounter __counter(#id);                                                \
  LWNODE_CALL_TRACE_ID_LOG(id, __VA_ARGS__)

#define LWNODE_CALL_TRACE_ID_INDENT(id) IndentCounter::indent(#id);
#define LWNODE_CALL_TRACE_ID_UNINDENT(id) IndentCounter::unIndent(#id);

// LWNODE_CALL_TRACE == LWNODE_CALL_TRACE_ID(COMMON, ...)
#define LWNODE_CALL_TRACE(msg, ...)                                            \
  LWNODE_CALL_TRACE_ID(COMMON, msg, ##__VA_ARGS__);

#define LWNODE_CALL_TRACE_LOG(msg, ...)                                        \
  LWNODE_CALL_TRACE_ID_LOG(COMMON, msg, ##__VA_ARGS__);

#define LWNODE_CALL_TRACE_INDENT() LWNODE_CALL_TRACE_ID_INDENT(COMMON);
#define LWNODE_CALL_TRACE_UNINDENT() LWNODE_CALL_TRACE_ID_UNINDENT(COMMON);

// GC
#define LWNODE_CALL_TRACE_GC_START(msg, ...)                                   \
  LWNODE_CALL_TRACE_ID_LOG(GC, "GC: %s", ##__VA_ARGS__)

#define LWNODE_CALL_TRACE_GC_END(msg, ...)                                     \
  LWNODE_CALL_TRACE_ID_LOG(GC, "GC: /%s", ##__VA_ARGS__)

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
