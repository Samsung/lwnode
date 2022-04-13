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
    ExposeGC,
    UseStrict,
    DisableIdleGC,
    TopLevelWait,
    AllowCodeGenerationFromString,
    AbortOnUncaughtException,
    ExposeExternalizeString,
    UnhandledRejections,
    // lwnode
    TraceCall,
    TraceGC,
    InternalLog,
    LWNodeOther,
    DebugServer,
  };

  Flag(const std::string& name, Type type, bool useAsPrefix = false)
      : name_(name), type_(type), useAsPrefix_(useAsPrefix) {}
  virtual ~Flag() {}

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
  virtual ~FlagWithValues() {}

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
  virtual ~FlagWithNegativeValues() {}

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
  struct FlagComparator {
    bool operator()(const Flag* a, const Flag* b) const {
      return a->type() < b->type();
    }
  };

  Flags() { initFlags(); }
  virtual ~Flags() { validFlags_.clear(); }

  void add(const std::string& flag);
  void add(Flag::Type type);
  void add(Flag* flag);

  bool isOn(Flag::Type type, const std::string& value = "");
  void shrinkArgumentList(int* argc, char** argv);

  // NOTE: get() and set() are only used in cctest
  std::set<Flag*, FlagComparator> get() { return flags_; };
  void set(std::set<Flag*, FlagComparator> flags) { flags_ = flags; }

 private:
  void initFlags();
  Flag* findFlagObject(const std::string& name);
  Flag* findFlagObject(Flag::Type type);

  Flag* getFlag(Flag::Type type);

  template <class T>
  void addFlag(const char* name, Flag::Type type, bool useAsPrefix = false) {
    validFlags_.push_back(std::make_unique<T>(name, type, useAsPrefix));
  }

  std::vector<std::unique_ptr<Flag>> validFlags_;
  std::set<Flag*, FlagComparator> flags_;
};

}  // namespace EscargotShim
