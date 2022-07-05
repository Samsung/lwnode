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

#include "color.h"
#include "logger-impl.h"
#include "logger-util.h"

// loggers (release)
#define LWNODE_LOG(tag) Logger(LogTYPED(LogTYPED::Type::tag))
#define LWNODE_LOGF(tag, fmt, ...) LWNODE_LOG(tag).print(fmt, ##__VA_ARGS__)

#define LWNODE_LOGR(fmt, ...) LWNODE_LOGF(RAW, fmt, ##__VA_ARGS__)
#define LWNODE_LOGI(fmt, ...) LWNODE_LOGF(INFO, fmt, ##__VA_ARGS__)
#define LWNODE_LOGW(fmt, ...) LWNODE_LOGF(WARN, fmt, ##__VA_ARGS__)
#define LWNODE_LOGE(fmt, ...) LWNODE_LOGF(ERROR, fmt, ##__VA_ARGS__)

// loggers (internal)
// enabled if running with debug build or force-logging option
#define LWNODE_LOG_INTERNAL(tag, fmt, ...)                                     \
  Logger(LogINTERNAL(LogTYPED::Type::tag)).print(fmt, ##__VA_ARGS__)

#define LWNODE_LOG_RAW(fmt, ...) LWNODE_LOG_INTERNAL(RAW, fmt, ##__VA_ARGS__)
#define LWNODE_LOG_INFO(fmt, ...) LWNODE_LOG_INTERNAL(INFO, fmt, ##__VA_ARGS__)
#define LWNODE_LOG_WARN(fmt, ...) LWNODE_LOG_INTERNAL(WARN, fmt, ##__VA_ARGS__)
#define LWNODE_LOG_ERROR(fmt, ...)                                             \
  LWNODE_LOG_INTERNAL(ERROR, fmt, ##__VA_ARGS__)

#define LWNODE_UNIMPLEMENT                                                     \
  LWNODE_LOG_INTERNAL(RAW, CLR_RED "UNIMPLEMENTED " CLR_RESET)                 \
      << CLR_RESET << __CODE_LOCATION__

#define LWNODE_UNIMPLEMENT_IGNORED                                             \
  LWNODE_LOG_INTERNAL(RAW, CLR_DIM "UNIMPLEMENTED (IGNORED) ")                 \
      << CLR_RESET << __CODE_LOCATION__

#define LWNODE_UNIMPLEMENT_WORKAROUND                                          \
  LWNODE_LOG_INTERNAL(RAW, CLR_DIM "UNIMPLEMENTED (USE WORKAROUND) ")          \
      << CLR_RESET << __CODE_LOCATION__

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

struct DLogConfig : Logger::Output::Config {
  DLogConfig(const std::string& t) : tag(t) {}
  std::string tag;
};

class LWNodeLogger : public Logger {
 public:
  LWNodeLogger(std::shared_ptr<DLogConfig> config) { outConfig_ = config; }
};

class DlogOut : public Logger::Output {
 public:
  void flush(std::stringstream& ss,
             std::shared_ptr<Output::Config> config = nullptr) override;
  void appendEndOfLine(std::stringstream& ss) override;
};

class LogKind {
 public:
  static std::shared_ptr<DLogConfig> user() { return getInstance()->user_; }
  static std::shared_ptr<DLogConfig> lwnode() { return getInstance()->lwnode_; }
  static LogKind* getInstance();

 private:
  LogKind();
  std::shared_ptr<DLogConfig> user_;
  std::shared_ptr<DLogConfig> lwnode_;
};
