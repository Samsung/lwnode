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

#include <EscargotPublic.h>
#include <v8.h>
#include "utils/gc.h"

namespace EscargotShim {

class ContextWrap;
class IsolateWrap;

class HandleWrap : public gc {
 public:
  enum Type : uint8_t {
    JsValue = 0,
    Context,
    ObjectTemplate,
    FunctionTemplate,
    Script,
    // NotPresent should be at last
    NotPresent,
  };

  uint8_t type() const;
  bool isValid() const;

 protected:
  HandleWrap() = default;
  uint8_t type_ = NotPresent;
};

class ValueWrap : public HandleWrap {
 public:
  ValueWrap(const ValueWrap& src) = delete;
  const ValueWrap& operator=(const ValueWrap& src) = delete;
  const ValueWrap& operator=(ValueWrap&& src) = delete;

  // Value
  static ValueWrap* createValue(Escargot::ValueRef* esValue);
  Escargot::ValueRef* value() const;

  // Context
  static ValueWrap* createContext(ContextWrap* lwContext);
  ContextWrap* context() const;

  // Script
  static ValueWrap* createScript(Escargot::ScriptRef* esScript);
  Escargot::ScriptRef* script() const;

  // FunctionTemplate
  static ValueWrap* createFunctionTemplate(
      Escargot::FunctionTemplateRef* esTemplate);
  Escargot::FunctionTemplateRef* functionTemplate() const;

 private:
  ValueWrap(void* ptr, HandleWrap::Type type);
  void* holder_ = nullptr;
};

}  // namespace EscargotShim
