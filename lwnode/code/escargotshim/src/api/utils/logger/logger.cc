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
#include <assert.h>
#include <regex>
#include <set>
#include <sstream>

std::string getPrettyFunctionName(const std::string fullname) {
  std::smatch match;
  const std::regex re(R"(([\w\:~]+)\()");

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

thread_local int s_indentCount = 0;
thread_local int s_deltaCount = 0;
thread_local std::set<std::string> s_counterIds;

#define FORCE_ENABLE_INDENT_ID "NODE"

static bool isIndentIdEnabled(std::string id) {
  if (id == FORCE_ENABLE_INDENT_ID) {
    return true;
  }
  return EscargotShim::Flags::isTraceCallEnabled(id);
}

void IndentCounter::indent(std::string id) {
  if (isIndentIdEnabled(id) == false) return;
  s_deltaCount++;
}

void IndentCounter::unIndent(std::string id) {
  if (isIndentIdEnabled(id) == false) return;
  s_deltaCount--;
}

IndentCounter::IndentCounter(std::string id) {
  id_ = id;

  if (isIndentIdEnabled(id) == false) return;

  s_indentCount++;
  s_counterIds.insert(id);
}

IndentCounter::~IndentCounter() {
  if (isIndentIdEnabled(id_) == false) return;

  s_indentCount--;
  s_counterIds.erase(id_);
}

std::string IndentCounter::getString(std::string id) {
  if (s_counterIds.find(id) == s_counterIds.end()) {
    return "";
  }

  assert(s_indentCount >= 0);

  std::ostringstream oss;
  int indentCount = s_indentCount + s_deltaCount;

  if (s_deltaCount > 0) {
    oss << s_deltaCount << " ";
  }

  for (int i = 1; i < std::min(30, indentCount); ++i) {
    oss << "  ";
  }

  return oss.str();
}
