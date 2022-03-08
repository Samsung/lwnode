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

#if !defined(LWNODE_EXPORT)
#define LWNODE_EXPORT __attribute__((visibility("default")))
#define LWNODE_LOCAL __attribute__((visibility("hidden")))
#endif

namespace EscargotShim {

typedef uint16_t flag_t;

class Flag {
 public:
  enum Type : flag_t {
    Empty = 0,
    ExposeGC = 1 << 1,
    UseStrict = 1 << 2,
    DisableIdleGC = 1 << 3,
    TopLevelWait = 1 << 4,
    AllowCodeGenerationFromString = 1 << 5,
    AbortOnUncaughtException = 1 << 6,
    ExposeExternalizeString = 1 << 7,
    UnhandledRejections = 1 << 8,
    // lwnode
    TraceCall = 1 << 9,
    TraceGC = 1 << 10,
    InternalLog = 1 << 11,
    LWNodeOther = 1 << 14,
  };

  Flag(std::string name, Type type, bool useAsPrefix = false)
      : name_(name), type_(type), useAsPrefix_(useAsPrefix) {}

  std::string name() { return name_; }
  Type type() { return type_; }
  bool isPrefixOf(const std::string& name);

 private:
  std::string name_;
  Type type_ = Type::Empty;
  bool useAsPrefix_ = false;
};

class LWNODE_EXPORT Flags {
 public:
  static void set(flag_t flags) { s_flags = flags; }
  static void add(flag_t flags) { s_flags |= flags; }
  static flag_t get() { return s_flags; };

  static bool isTraceCallEnabled(std::string id = "*");
  static bool isTraceGCEnabled() { return s_flags & Flag::Type::TraceGC; }
  static bool isInternalLogEnabled() {
    return s_flags & Flag::Type::InternalLog;
  }
  static bool isCodeGenerationFromStringAllowed() {
    return s_flags & Flag::Type::AllowCodeGenerationFromString;
  }
  static bool isAbortOnUncaughtException() {
    return s_flags & Flag::Type::AbortOnUncaughtException;
  }
  static bool isExposeExternalizeString() {
    return s_flags & Flag::Type::ExposeExternalizeString;
  }
  static bool isExposeGCEnabled() { return s_flags & Flag::Type::ExposeGC; }

  static bool isUnhandledRejectionsEnabled() {
    return s_flags & Flag::Type::UnhandledRejections;
  }
  static void setUnhandledRejections(const std::string& value) {
    s_unhandled_rejections.insert(value);
  }

  static void setTraceCallId(const std::string& id);

  static void shrinkArgumentList(int* argc, char** argv);

 private:
  static std::set<std::string> s_trace_ids;
  static std::set<std::string> s_negative_trace_ids;
  static std::set<std::string> s_unhandled_rejections;
  static flag_t s_flags;
};

}  // namespace EscargotShim
