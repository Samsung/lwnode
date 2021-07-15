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

#include "flags.h"

namespace EscargotShim {

flag_t Flags::s_flags = FlagType::Empty;
std::set<std::string> Flags::s_trace_ids;
std::set<std::string> Flags::s_negative_trace_ids;

bool Flags::isTraceCallEnabled(std::string id) {
  if (!(s_flags & FlagType::TraceCall)) {
    return false;
  }

  if (!s_trace_ids.empty()) {
    if (s_trace_ids.find(id) == s_trace_ids.end()) {
      return false;
    }
  }

  if (!s_negative_trace_ids.empty()) {
    if (s_negative_trace_ids.find(id) != s_negative_trace_ids.end()) {
      return false;
    }
  }

  return true;
}

}  // namespace EscargotShim
