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
#include <string.h>
#include <cstdio>

#define COLOR_RESET "\033[0m"
#define COLOR_DIM "\033[0;2m"
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_GREY "\033[0;37m"
#define COLOR_BLACK "\033[0;30m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_DARKGREY "\033[01;30m"
#define COLOR_BRED "\033[01;31m"
#define COLOR_BYELLOW "\033[01;33m"
#define COLOR_BBLUE "\033[01;34m"
#define COLOR_BMAGENTA "\033[01;35m"
#define COLOR_BCYAN "\033[01;36m"
#define COLOR_BGREEN "\033[01;32m"
#define COLOR_WHITE "\033[01;37m"
#define COLOR_REDBG "\033[0;41m"

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE_FMT " %s (%s:%d)"
#define TRACE_ARGS __PRETTY_FUNCTION__, __FILENAME__, __LINE__

#define LWNODE_LOG_RAW(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__);

#define LWNODE_LOG_INFO(fmt, ...)                                              \
  do {                                                                         \
    fprintf(stdout, "INFO " fmt "\n", ##__VA_ARGS__);                          \
  } while (0);

#define LWNODE_LOG_WARN(fmt, ...)                                              \
  do {                                                                         \
    fprintf(stderr, COLOR_YELLOW "WARN " fmt COLOR_RESET, ##__VA_ARGS__);      \
  } while (0);

#define LWNODE_LOG_ERROR(fmt, ...)                                             \
  do {                                                                         \
    fprintf(stderr, COLOR_BRED "ERROR " fmt COLOR_RESET, ##__VA_ARGS__);       \
  } while (0);

#define LWNODE_UNIMPLEMENT                                                     \
  do {                                                                         \
    LWNODE_LOG_RAW(COLOR_RED "UNIMPLEMENTED" TRACE_FMT COLOR_RESET,            \
                   TRACE_ARGS);                                                \
  } while (0)

// conditional loggers

#if !defined(NDEBUG)
#define LWNODE_DLOG_RAW(fmt, ...) LWNODE_LOG_RAW(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_INFO(fmt, ...) LWNODE_LOG_INFO(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_WARN(fmt, ...) LWNODE_LOG_WARN(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_ERROR(fmt, ...) LWNODE_LOG_ERROR(fmt, ##__VA_ARGS__)
#else
#define LWNODE_DLOG_RAW(fmt, ...)
#define LWNODE_DLOG_INFO(fmt, ...)
#define LWNODE_DLOG_WARN(fmt, ...)
#define LWNODE_DLOG_ERROR(fmt, ...)
#endif

// enable this when call tracing is needed.
// #define LWNODE_ENABLE_CALL_TRACE

#if !defined(NDEBUG) && defined(LWNODE_ENABLE_CALL_TRACE)
#define LWNODE_CALL_TRACE()                                                    \
  do {                                                                         \
    LWNODE_DLOG_RAW("TRACE" TRACE_FMT, TRACE_ARGS);                            \
  } while (0)
#else
#define LWNODE_CALL_TRACE()
#endif
