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

#include <cstdint>
#include <set>
#include <string>
#include "compiler.h"

namespace EscargotShim {

typedef uint16_t flag_t;

enum FlagType : flag_t {
  Empty = 0,
  ExposeGC = 1 << 1,
  UseStrict = 1 << 2,
  DisableIdleGC = 1 << 3,
  TopLevelWait = 1 << 4,
  // lwnode
  TraceCall = 1 << 5,
  TraceGC = 1 << 6,
};

class LWNODE_EXPORT Flags {
 public:
  static void set(flag_t flags) { s_flags = flags; }
  static void add(flag_t flags) { s_flags |= flags; }
  static flag_t get() { return s_flags; };

  static bool isTraceCallEnabled(std::string id = "*");
  static bool isTraceGCEnabled() { return s_flags & FlagType::TraceGC; }

  static void setTraceCallId(std::string id) { s_trace_ids.insert(id); }

 private:
  static std::set<std::string> s_trace_ids;
  static flag_t s_flags;
};

}  // namespace EscargotShim
