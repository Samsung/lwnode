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

#include "flags.h"

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

std::string getPrettyFunctionName(const std::string fullname);
std::string createCodeLocation(const char* functionName,
                               const char* filename,
                               const int line);
inline const char* strBool(bool value) {
  return value ? "True" : "False";
}

class IndentCounter {
 public:
  IndentCounter(std::string id);
  ~IndentCounter();
  static std::string getString(std::string id = "");

 private:
  std::string id_;
};

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE_FMT " %s (%s:%d)"
#define TRACE_ARGS __PRETTY_FUNCTION__, __FILENAME__, __LINE__
#define TRACE_ARGS2                                                            \
  getPrettyFunctionName(__PRETTY_FUNCTION__).c_str(), __FILENAME__, __LINE__

#if !defined(NDEBUG)
#define LWNODE_LOG_RAW(fmt, ...)                                               \
  do {                                                                         \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__);                                  \
  } while (0);
#else
#define LWNODE_LOG_RAW(fmt, ...)                                               \
  do {                                                                         \
    if (EscargotShim::Flags::isInternalLogEnabled() == true) {                 \
      fprintf(stderr, fmt "\n", ##__VA_ARGS__);                                \
    }                                                                          \
  } while (0);
#endif

#define LWNODE_LOG_INFO(fmt, ...) LWNODE_LOG_RAW("INFO " fmt, ##__VA_ARGS__);

#define LWNODE_LOG_WARN(fmt, ...)                                              \
  LWNODE_LOG_RAW(COLOR_YELLOW "WARN " fmt COLOR_RESET, ##__VA_ARGS__);

#define LWNODE_LOG_ERROR(fmt, ...)                                             \
  LWNODE_LOG_RAW(COLOR_BRED "ERROR " fmt COLOR_RESET, ##__VA_ARGS__);

#define LWNODE_UNIMPLEMENT                                                     \
  LWNODE_LOG_RAW(COLOR_RED "UNIMPLEMENTED" TRACE_FMT COLOR_RESET, TRACE_ARGS2);

#define LWNODE_UNIMPLEMENT_IGNORED                                             \
  LWNODE_LOG_RAW(COLOR_DIM "UNIMPLEMENTED (IGNORED)" TRACE_FMT COLOR_RESET,    \
                 TRACE_ARGS2);

#define LWNODE_UNIMPLEMENT_WORKAROUND                                          \
  LWNODE_LOG_RAW(COLOR_DIM                                                     \
                 "UNIMPLEMENTED (USE WORKAROUND)" TRACE_FMT COLOR_RESET,       \
                 TRACE_ARGS2);

// conditional loggers

#if !defined(NDEBUG)
#define LWNODE_DLOG_RAW(fmt, ...) LWNODE_LOG_RAW(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_INFO(fmt, ...) LWNODE_LOG_INFO(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_WARN(fmt, ...) LWNODE_LOG_WARN(fmt, ##__VA_ARGS__)
#define LWNODE_DLOG_ERROR(fmt, ...) LWNODE_LOG_ERROR(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) LWNODE_LOG_ERROR(fmt, ##__VA_ARGS__)
#else
#define LWNODE_DLOG_RAW(fmt, ...)
#define LWNODE_DLOG_INFO(fmt, ...)
#define LWNODE_DLOG_WARN(fmt, ...)
#define LWNODE_DLOG_ERROR(fmt, ...)
#define FATAL(fmt, ...)
#endif
