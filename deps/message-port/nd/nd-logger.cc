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

#include "nd-logger.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>  // setfill and setw
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#if defined(ENABLE_TRACE)

#define TYPE_LENGTH_LIMIT 5
#define TRACE_ID_LENGTH_LIMIT 10
#define COLOR_RESET "\033[0m"
#define COLOR_DIM "\033[0;2m"

const char* GetPriorityString(const unsigned priority) {
  // clang-format off
  switch (priority) {
    case LOG_PRIO_UNKNOWN: return "UNKNOWN";
    case LOG_PRIO_DEFAULT: return "DEFAULT";
    case LOG_PRIO_VERBOSE: return "VERBOSE";
    case LOG_PRIO_DEBUG: return "DEBUG";
    case LOG_PRIO_INFO: return "INFO";
    case LOG_PRIO_WARN: return "WARN";
    case LOG_PRIO_ERROR: return "ERROR";
    case LOG_PRIO_FATAL: return "FATAL";
    case LOG_PRIO_SILENT: return "SILENT";
    case LOG_PRIO_MAX: return "MAX";
    default: return "UNKNOWN";
  }
  // clang-format on
}

static const auto program_start_time = std::chrono::system_clock::now();

static std::ostream& WriteTimestamp(std::ostream& os) {
  const auto now = std::chrono::system_clock::now();
  const auto elapsed = now - program_start_time;
  const auto s = std::chrono::duration_cast<std::chrono::seconds>(elapsed) %
                 std::chrono::minutes(1);
  const auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed) %
      std::chrono::seconds(1);

  os << std::right << std::setfill('0') << std::setw(2) << s.count() << "."
     << std::setw(3) << ms.count() << " " << std::setw(0) << std::setfill(' ');

  return os;
}

static void WriteTag(std::ostream& os, const std::string& tag) {
  os << std::setw(TYPE_LENGTH_LIMIT) << tag << std::setw(0) << " ";
}

static void writeHeader(std::ostream& ss,
                        const std::string& tag,
                        const std::string& id) {
  WriteTimestamp(ss);
  if (!tag.empty()) {
    WriteTag(ss, tag);
  }
  if (!id.empty()) {
    ss << std::left << std::setfill(' ') << "("
       << std::setw(TRACE_ID_LENGTH_LIMIT)
       << std::string(id).substr(0, TRACE_ID_LENGTH_LIMIT) << ") ";
  }
}
#endif  // ENABLE_TRACE

void LogOutput(const unsigned priority,
               const char* id,
               const std::string& message,
               bool newline) {
#if defined(ENABLE_TRACE)
  if (id && *id) {
    // Format for TRACE
    std::stringstream ss;
    ss << COLOR_DIM;
    writeHeader(ss, "TRACE", id);
    ss << message;
    ss << COLOR_RESET << (newline ? "\n" : "");
    std::cout << ss.str();
    return;
  }
#endif  // ENABLE_TRACE

  std::cout << message << (newline ? "\n" : "");
}

void PrintF(const unsigned priority,
            const char* id,
            const std::string& value1,
            const char* value2) {
  std::ostringstream oss;
  oss << (!value1.empty() ? value1 + " " : "") << value2;
  LogOutput(priority, id, oss.str(), true);
}

// --- Formatter ---

std::string GetPrettyFunctionName(const std::string fullname,
                                  std::string prefixPattern) {
  std::stringstream ss;
  if (!prefixPattern.empty()) {
    ss << "(?:" << prefixPattern << ")|";
  }
  ss << R"((?::\()|([\w:~]+)\()";

  try {
    std::smatch match;
    const std::regex re(ss.str());

    std::stringstream result;
    std::string suffix = fullname;
    while (std::regex_search(suffix, match, re)) {
      result << match[1];
      suffix = match.suffix();
    }
    return result.str();
  } catch (std::regex_error& e) {
    return "";
  }
}

std::string CreateCodeLocation(const char* functionName,
                               const char* filename,
                               const int line,
                               std::string prefixPattern) {
  std::ostringstream oss;
  oss << GetPrettyFunctionName(functionName, prefixPattern) << " (" << filename
      << ":" << line << ")";
  return oss.str();
}

bool IsTraceEnabled(const char* key) {
  static std::map<std::string, bool> trace_map;
  static bool is_trace_map_initialized = false;
  static bool allow_all = false;
  static bool no_trace_env_set = false;
  static bool registered_cleanup = false;
  static bool exiting = false;
  if (!registered_cleanup) {
    // When putting TRACE inside the destructor of a static object and run a
    // program with Valgrind, the static variables above might get cleaned up
    // (removed) before any destructor has a chance to execute. This can lead to
    // unexpected behavior. Normally, all trace logs are printed before program
    // is returning from the main function,
    std::atexit([] { exiting = true; });
    registered_cleanup = true;
  }

  if (exiting || no_trace_env_set) {
    return false;
  }

  if (!is_trace_map_initialized) {
    const char* trace_env = std::getenv("TRACE");
    if (!trace_env) {
      no_trace_env_set = true;
    } else {
      std::string trace(trace_env);
      std::stringstream ss(trace);
      std::string token;
      while (std::getline(ss, token, ',')) {
        if (token == "*") {
          allow_all = true;
        } else if (!token.empty() && token[0] == '-') {
          trace_map[token.substr(1)] = false;
        } else {
          trace_map[token] = true;
        }
      }

      if (allow_all == false && trace_map.empty()) {
        no_trace_env_set = true;
      }
    }
    is_trace_map_initialized = true;
  }

  std::string key_str(key);
  if (trace_map.find(key_str) != trace_map.end()) {
    return trace_map[key_str];
  }
  return allow_all;
}
