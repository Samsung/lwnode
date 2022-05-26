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

#include <algorithm>

#include "api/utils/string-util.h"

namespace EscargotShim {

void Flags::initFlags() {
  // v8 flags
  addFlag<Flag>("--expose-gc", Flag::Type::ExposeGC);
  addFlag<Flag>("--use-strict", Flag::Type::UseStrict);
  addFlag<Flag>("--off-idlegc", Flag::Type::DisableIdleGC);
  addFlag<Flag>("--harmony-top-level-await", Flag::Type::TopLevelWait);
  addFlag<Flag>("--disallow-code-generation-from-strings",
                Flag::Type::DisallowCodeGenerationFromStrings);
  addFlag<Flag>("--abort-on-uncaught-exception",
                Flag::Type::AbortOnUncaughtException);
  addFlag<Flag>("--expose-externalize-string",
                Flag::Type::ExposeExternalizeString);
  addFlag<FlagWithValues>("--unhandled-rejections=",
                          Flag::Type::UnhandledRejections);
  addFlag<Flag>("--trace-debug", Flag::Type::LWNodeOther, true);
  addFlag<Flag>("--debug", Flag::Type::LWNodeOther, true);
  addFlag<Flag>("--stack-size=", Flag::Type::LWNodeOther, true);
  addFlag<Flag>("--nolazy", Flag::Type::LWNodeOther, true);

  // lwnode flags
  addFlag<Flag>("--trace-gc", Flag::Type::TraceGC);
  addFlag<FlagWithNegativeValues>("--trace-call=", Flag::Type::TraceCall, true);
  addFlag<Flag>("--internal-log", Flag::Type::InternalLog);
  addFlag<Flag>("--start-debug-server", Flag::Type::DebugServer);
}

bool Flag::isPrefixOf(const std::string& name) {
  if (useAsPrefix_ && (name.find(name_) != std::string::npos)) {
    return true;
  }

  return false;
}

Flag* Flags::findFlagObject(const std::string& name) {
  std::string normalized = name;
  std::replace(normalized.begin(), normalized.end(), '_', '-');

  for (size_t i = 0; i < validFlags_.size(); i++) {
    Flag* flag = validFlags_[i].get();
    if ((flag->name() == normalized) || flag->isPrefixOf(normalized)) {
      return flag;
    }
  }

  return nullptr;
}

Flag* Flags::findFlagObject(Flag::Type type) {
  for (size_t i = 0; i < validFlags_.size(); i++) {
    Flag* flag = validFlags_[i].get();
    if (flag->type() == type) {
      return flag;
    }
  }

  return nullptr;
}

void Flags::add(const std::string& userOption) {
  // @check https://github.sec.samsung.net/lws/node-escargot/issues/394
  Flag* flag = findFlagObject(userOption);
  if (!flag || flag->type() == Flag::Empty) {
    LWNODE_DLOG_WARN("Ignore flag: '%s'\n", flag ? flag->name().c_str() : "");
    return;
  }

  add(flag);

  if (flag->type() == Flag::Type::TraceCall ||
      flag->type() == Flag::Type::UnhandledRejections) {
    std::string optionValues = userOption.substr(userOption.find_first_of('=') +
                                                 1);  // +1 for skipping '='
    auto tokens = strSplit(optionValues, ',');
    for (auto token : tokens) {
      flag->addValue(token);
    }
  }
}

void Flags::add(Flag::Type type) {
  add(findFlagObject(type));
}

void Flags::add(Flag* flag) {
  if (!flag || flag->type() == Flag::Type::LWNodeOther) {
    return;
  }

  flags_.insert(flag);
}

Flag* Flags::getFlag(Flag::Type type) {
  Flag flag("", type);
  auto itr = flags_.find(&flag);
  if (itr == flags_.end()) {
    return nullptr;
  }

  return *itr;
}

bool Flags::isOn(Flag::Type type, const std::string& value) {
  Flag* flag = getFlag(type);
  if (!flag) {
    return false;
  }

  if (value == "") {
    return true;
  }

  return flag->hasValue(value);
}

void Flags::shrinkArgumentList(int* argc, char** argv) {
  int count = 0;
  for (int idx = 0; idx < *argc; idx++) {
    if (argv[idx]) {
      argv[count++] = argv[idx];
    }
  }
  *argc = count;
}

}  // namespace EscargotShim
