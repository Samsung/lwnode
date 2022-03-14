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

std::vector<Flag> Flags::s_validFlags = {
    // v8 flags
    Flag("--expose-gc", Flag::Type::ExposeGC),
    Flag("--use-strict", Flag::Type::UseStrict),
    Flag("--off-idlegc", Flag::Type::DisableIdleGC),
    Flag("--harmony-top-level-await", Flag::Type::TopLevelWait),
    Flag("--allow-code-generation-from-strings",
         Flag::Type::AllowCodeGenerationFromString),
    Flag("--abort-on-uncaught-exception", Flag::Type::AbortOnUncaughtException),
    Flag("--expose-externalize-string", Flag::Type::ExposeExternalizeString),
    FlagWithValues("--unhandled-rejections=", Flag::Type::UnhandledRejections),
    Flag("--trace-debug", Flag::Type::LWNodeOther, true),
    Flag("--debug", Flag::Type::LWNodeOther, true),
    Flag("--stack-size=", Flag::Type::LWNodeOther, true),
    Flag("--nolazy", Flag::Type::LWNodeOther, true),
    // lwnode flags
    Flag("--trace-gc", Flag::Type::TraceGC),
    FlagWithNegativeValues("--trace-call=", Flag::Type::TraceCall, true),
    Flag("--internal-log", Flag::Type::InternalLog),
};

bool Flag::isPrefixOf(const std::string& name) {
  if (useAsPrefix_ && (name.find(name_) != std::string::npos)) {
    return true;
  }

  return false;
}

Flag* Flags::findFlagObject(const std::string& name) {
  std::string normalized = name;
  std::replace(normalized.begin(), normalized.end(), '_', '-');

  for (size_t i = 0; i < s_validFlags.size(); i++) {
    Flag* flag = &s_validFlags[i];
    if ((flag->name() == normalized) || flag->isPrefixOf(normalized)) {
      return flag;
    }
  }

  return nullptr;
}

Flag* Flags::findFlagObject(Flag::Type type) {
  for (size_t i = 0; i < s_validFlags.size(); i++) {
    Flag* flag = &s_validFlags[i];
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
