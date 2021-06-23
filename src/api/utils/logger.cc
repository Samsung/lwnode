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

#include "logger.h"
#include <regex>
#include <set>
#include <sstream>

std::string getPrettyFunctionName(const std::string fullname) {
  std::smatch match;
  const std::regex re(R"(([\w\:]+)\()");

  if (std::regex_search(fullname, match, re) && match.size() > 1) {
    return match.str(1);
  }
  return "";
}

std::string createCodeLocation(const char* functionName,
                               const char* filename,
                               const int line) {
  std::ostringstream oss;
  oss << getPrettyFunctionName(functionName) << " (" << filename << ":" << line
      << ")" << std::endl;
  return oss.str();
}

thread_local int s_callDepth = 0;
thread_local std::set<std::string> s_counterIds;

IndentCounter::IndentCounter(std::string id) {
  s_callDepth++;
  s_counterIds.insert(id);
  id_ = id;
}

IndentCounter::~IndentCounter() {
  s_callDepth--;
  s_counterIds.erase(id_);
}

std::string IndentCounter::getString(std::string id) {
  if (s_counterIds.find(id) == s_counterIds.end()) {
    return "";
  }

  assert(s_callDepth >= 0);

  std::string indent;
  for (int i = 1; i < std::min(5, s_callDepth); ++i) {
    indent += "\t";
  }
  return indent;
}
