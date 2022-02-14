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

#include <string.h>
#include <cstdio>

#include "color.h"
#include "flags.h"
#include "logger-impl.h"

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
  static void indent(std::string id);
  static void unIndent(std::string id);

 private:
  std::string id_;
};

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE_FMT "%s (%s:%d)"
#define TRACE_ARGS __PRETTY_FUNCTION__, __FILENAME__, __LINE__
#define TRACE_ARGS2                                                            \
  getPrettyFunctionName(__PRETTY_FUNCTION__).c_str(), __FILENAME__, __LINE__

// loggers (release)
#define LWNODE_LOG(tag) Logger(LogTYPED(LogTYPED::Type::tag).header())
#define LWNODE_LOGF(tag, fmt, ...)                                             \
  Logger(LogTYPED(LogTYPED::Type::tag).header()).print(fmt, ##__VA_ARGS__)

#define LWNODE_LOGI(fmt, ...) LWNODE_LOGF(INFO, fmt, ##__VA_ARGS__)
#define LWNODE_LOGW(fmt, ...) LWNODE_LOGF(WARN, fmt, ##__VA_ARGS__)
#define LWNODE_LOGE(fmt, ...) LWNODE_LOGF(ERROR, fmt, ##__VA_ARGS__)

#define LWNODE_LOG_RAW(fmt, ...) LWNODE_LOGF(RAW, fmt, ##__VA_ARGS__)

// loggers (internal)
#if !defined(NDEBUG)
#define LWNODE_LOG_INTERNAL(fmt, ...) LWNODE_LOGF(RAW, fmt, ##__VA_ARGS__)
#define LWNODE_LOG_INTERNAL_TAG(tag, fmt, ...)                                 \
  LWNODE_LOGF(tag, fmt, ##__VA_ARGS__)
#else
#define LWNODE_LOG_INTERNAL(fmt, ...)                                          \
  if (EscargotShim::Flags::isInternalLogEnabled()) {                           \
    LWNODE_LOGF(RAW, fmt, ##__VA_ARGS__);                                      \
  }
#define LWNODE_LOG_INTERNAL_TAG(tag, fmt, ...)                                 \
  if (EscargotShim::Flags::isInternalLogEnabled()) {                           \
    LWNODE_LOGF(tag, fmt, ##__VA_ARGS__);                                      \
  }
#endif

#define LWNODE_LOG_INFO(fmt, ...)                                              \
  LWNODE_LOG_INTERNAL_TAG(INFO, fmt, ##__VA_ARGS__)

#define LWNODE_LOG_WARN(fmt, ...)                                              \
  LWNODE_LOG_INTERNAL_TAG(WARN, fmt, ##__VA_ARGS__)

#define LWNODE_LOG_ERROR(fmt, ...)                                             \
  LWNODE_LOG_INTERNAL_TAG(ERROR, fmt, ##__VA_ARGS__)

#define LWNODE_UNIMPLEMENT                                                     \
  LWNODE_LOG_INTERNAL(CLR_RED "UNIMPLEMENTED " TRACE_FMT CLR_RESET,            \
                      TRACE_ARGS2);

#define LWNODE_UNIMPLEMENT_IGNORED                                             \
  LWNODE_LOG_INTERNAL(CLR_DIM "UNIMPLEMENTED (IGNORED) " TRACE_FMT CLR_RESET,  \
                      TRACE_ARGS2);

#define LWNODE_UNIMPLEMENT_WORKAROUND                                          \
  LWNODE_LOG_INTERNAL(CLR_DIM                                                  \
                      "UNIMPLEMENTED (USE WORKAROUND) " TRACE_FMT CLR_RESET,   \
                      TRACE_ARGS2);

// loggers (debug)
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
