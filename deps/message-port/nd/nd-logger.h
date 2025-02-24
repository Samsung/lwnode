/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <cstring>
#include <sstream>
#include <string>

#define LOG_RAW(...) PrintF(LOG_PRIO_VERBOSE, "", "", ##__VA_ARGS__)
#define LOG_DEBUG(...) PrintF(LOG_PRIO_DEBUG, "", "", ##__VA_ARGS__)
#define LOG_INFO(...) PrintF(LOG_PRIO_INFO, "", "", ##__VA_ARGS__)
#define LOG_WARN(...) PrintF(LOG_PRIO_WARN, "", "", ##__VA_ARGS__)
#define LOG_ERROR(...) PrintF(LOG_PRIO_ERROR, "", "", ##__VA_ARGS__)

enum LOG_PRIORITY {
  LOG_PRIO_UNKNOWN = 0,
  LOG_PRIO_DEFAULT,
  LOG_PRIO_VERBOSE,
  LOG_PRIO_DEBUG,
  LOG_PRIO_INFO,
  LOG_PRIO_WARN,
  LOG_PRIO_ERROR,
  LOG_PRIO_FATAL,
  LOG_PRIO_SILENT,
  LOG_PRIO_MAX
};

#ifndef __FILE_NAME__
#define __FILE_NAME__                                                          \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef __CODE_LOCATION__
#define __CODE_LOCATION__                                                      \
  CreateCodeLocation(__PRETTY_FUNCTION__, __FILE_NAME__, __LINE__).c_str()
#endif

// TRACE -----------------------------------------------------------------------

#if !defined(NDEBUG) || defined(DEV)
#define ENABLE_TRACE
#endif

#if !defined(ENABLE_TRACE)

#define TRACE(id, ...)
#define TRACE0(id, ...)
#define TRACEF(id, ...)
#define TRACEF0(id, ...)
#define S0(x)
#define KV(x)

#else

#define TRACE(id, ...)                                                         \
  do {                                                                         \
    if (IsTraceEnabled(#id)) {                                                 \
      Print(LOG_PRIO_DEBUG,                                                    \
            #id,                                                               \
            CreateCodeLocation(__PRETTY_FUNCTION__, __FILE_NAME__, __LINE__),  \
            ##__VA_ARGS__);                                                    \
    }                                                                          \
  } while (0)

#define TRACEF(id, ...)                                                        \
  do {                                                                         \
    if (IsTraceEnabled(#id)) {                                                 \
      PrintF(LOG_PRIO_DEBUG,                                                   \
             #id,                                                              \
             CreateCodeLocation(__PRETTY_FUNCTION__, __FILE_NAME__, __LINE__), \
             ##__VA_ARGS__);                                                   \
    }                                                                          \
  } while (0)

#define TRACE0(id, ...)                                                        \
  do {                                                                         \
    if (IsTraceEnabled(#id)) {                                                 \
      Print(LOG_PRIO_DEBUG, #id, "", ##__VA_ARGS__);                           \
    }                                                                          \
  } while (0)

#define TRACEF0(id, ...)                                                       \
  do {                                                                         \
    if (IsTraceEnabled(#id)) {                                                 \
      PrintF(LOG_PRIO_DEBUG, #id, "", ##__VA_ARGS__);                          \
    }                                                                          \
  } while (0)

#define S0(x) #x ":"
#define KV(x) S0(x), x

#endif  // end of !defined(ENABLE_TRACE)

// Logger Implementation ------------------------------------------------------

void LogOutput(const unsigned priority,
               const char* id,
               const std::string& message,
               bool newline);

inline void SPrintF(std::ostringstream& oss,
                    bool& has_fmt_specifier,
                    const char* format) {
  oss << format;
}

template <typename T, typename... Args>
void SPrintF(std::ostringstream& oss,
             bool& has_fmt_specifier,
             const char* format,
             T value,
             Args... args) {
  while (*format) {
    if (*format == '%' && *(format + 1) != '%') {
      has_fmt_specifier = true;
      format++;
      if ((*format == 'z')) {
        format++;
      } else if ((*format == 'l') || (*format == 'h')) {
        format++;
        if (*format == *(format + 1)) {
          format++;
        }
      }
      oss << value;
      SPrintF(oss, has_fmt_specifier, format + 1, args...);
      return;
    }
    oss << *format++;
  }
}

template <typename T>
void AppendToStream(std::ostringstream& oss, T value) {
  oss << value;
}

template <typename T, typename... Args>
void AppendToStream(std::ostringstream& oss, T value, Args... args) {
  oss << value << ' ';
  AppendToStream(oss, args...);
}

template <typename T, typename... Args>
void Print(const unsigned priority,
           const char* id,
           const std::string& prefix,
           T value,
           Args... args) {
  std::ostringstream oss;
  oss << (!prefix.empty() ? prefix + " " : "");
  AppendToStream(oss, value, args...);
  LogOutput(priority, id, oss.str(), true);
}

void PrintF(const unsigned priority,
            const char* id,
            const std::string& value1,
            const char* value2 = "");

template <typename T, typename... Args>
void PrintF(const unsigned priority,
            const char* id,
            const std::string& prefix,
            const char* format,
            T value,
            Args... args) {
  std::ostringstream oss_fmt;
  bool has_fmt_specifier = false;
  SPrintF(oss_fmt, has_fmt_specifier, format, value, args...);

#ifdef PRINTF_SUPPORT_ARGS_WITHOUT_SPECIFIERS
  if (!has_fmt_specifier) {
    Print(priority, id, prefix, oss_fmt.str(), args...);
  } else {
    PrintF(priority, id, prefix, oss_fmt.str().c_str());
  }
#else
  PrintF(priority, id, prefix, oss_fmt.str().c_str());
#endif
}

std::string CreateCodeLocation(const char* functionName,
                               const char* filename,
                               const int line,
                               std::string prefixPattern = "");

bool IsTraceEnabled(const char* key);
