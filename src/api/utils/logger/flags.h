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
#include <memory>
#include <set>
#include <string>
#include <vector>

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

  Flag(const std::string& name, Type type, bool useAsPrefix = false)
      : name_(name), type_(type), useAsPrefix_(useAsPrefix) {}

  virtual std::string name() { return name_; }
  virtual Type type() const { return type_; }
  virtual bool isPrefixOf(const std::string& name);

  virtual void addValue(const std::string& value){};
  virtual bool hasValue(const std::string& value) { return false; }

  virtual void addNegativeValue(const std::string& value) {}
  virtual bool hasNegativeValue(const std::string& value) { return false; }

 protected:
  std::string name_;
  Type type_ = Type::Empty;
  bool useAsPrefix_ = false;
};

class FlagWithValues : public Flag {
 public:
  FlagWithValues(const std::string& name, Type type, bool useAsPrefix = false)
      : Flag(name, type, useAsPrefix) {}

  virtual void addValue(const std::string& value) override {
    values_.insert(value);
  }

  virtual bool hasValue(const std::string& value) override {
    return values_.find(value) != values_.end();
  }

 private:
  std::set<std::string> values_;
};

class FlagWithNegativeValues : public FlagWithValues {
 public:
  FlagWithNegativeValues(const std::string& name,
                         Type type,
                         bool useAsPrefix = false)
      : FlagWithValues(name, type, useAsPrefix) {}

  void addNegativeValue(const std::string& value) override {
    negativeValues_.insert(value);
  }

  bool hasNegativeValue(const std::string& value) override {
    return negativeValues_.find(value) != negativeValues_.end();
  }

 private:
  std::set<std::string> negativeValues_;
};

class LWNODE_EXPORT Flags {
 public:
  static std::vector<Flag> s_validFlags;
  struct FlagComparator {
    bool operator()(const Flag* a, const Flag* b) const {
      return a->type() < b->type();
    }
  };

  void add(const std::string& flag);
  void add(Flag::Type type);
  void add(Flag* flag);

  bool isOn(Flag::Type type, const std::string& value = "");
  void shrinkArgumentList(int* argc, char** argv);

  std::set<Flag*, FlagComparator> get() { return flags_; };
  void set(std::set<Flag*, FlagComparator> flags) { flags_ = flags; }

 private:
  Flag* findFlagObject(const std::string& name);
  Flag* findFlagObject(Flag::Type type);

  Flag* getFlag(Flag::Type type);
  std::set<Flag*, FlagComparator> flags_;
};

}  // namespace EscargotShim
