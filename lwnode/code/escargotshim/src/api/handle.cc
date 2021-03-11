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

#include "handle.h"
#include "context.h"
#include "isolate.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

// --- HandleWrap ---

uint8_t HandleWrap::type() const {
  return type_;
}

bool HandleWrap::isValid() const {
  return (type_ < HandleWrap::Type::NotPresent);
}

// --- ValueWrap ---

ValueWrap::ValueWrap(void* ptr, HandleWrap::Type type) {
  LWNODE_CHECK_NOT_NULL(ptr);
  type_ = type;
  holder_ = ptr;
}

ValueWrap* ValueWrap::createValue(Escargot::ValueRef* esValue) {
  auto value = new ValueWrap(esValue, Type::JsValue);
  LWNODE_CALL_TRACE("%p | %p", esValue, value);
  return value;
}

ValueRef* ValueWrap::value() const {
  LWNODE_CHECK(type() == Type::JsValue);
  return reinterpret_cast<ValueRef*>(holder_);
}

ValueWrap* ValueWrap::createContext(ContextWrap* lwContext) {
  auto value = new ValueWrap(lwContext, Type::Context);
  LWNODE_CALL_TRACE("%p | %p", lwContext, value);
  return value;
};

ContextWrap* ValueWrap::context() const {
  LWNODE_CHECK(type() == Type::Context);
  return reinterpret_cast<ContextWrap*>(holder_);
}

ValueWrap* ValueWrap::createScript(ScriptRef* esScript) {
  auto value = new ValueWrap(esScript, Type::Script);
  LWNODE_CALL_TRACE("%p | %p", esScript, value);
  return value;
};

ScriptRef* ValueWrap::script() const {
  LWNODE_CHECK(type() == Type::Script);
  return reinterpret_cast<ScriptRef*>(holder_);
}

Escargot::TemplateRef* ValueWrap::tpl() const {
  LWNODE_CHECK(type() == Type::ObjectTemplate ||
               type() == Type::FunctionTemplate);
  return reinterpret_cast<TemplateRef*>(holder_);
}

ValueWrap* ValueWrap::createFunctionTemplate(
    Escargot::FunctionTemplateRef* esTemplate) {
  return new ValueWrap(esTemplate, Type::FunctionTemplate);
}

Escargot::FunctionTemplateRef* ValueWrap::ftpl() const {
  LWNODE_CHECK(type() == Type::FunctionTemplate);
  return reinterpret_cast<FunctionTemplateRef*>(holder_);
}

ValueWrap* ValueWrap::createObjectTemplate(
    Escargot::ObjectTemplateRef* esTemplate) {
  return new ValueWrap(esTemplate, Type::ObjectTemplate);
}

Escargot::ObjectTemplateRef* ValueWrap::otpl() const {
  LWNODE_CHECK(type() == Type::ObjectTemplate);
  return reinterpret_cast<ObjectTemplateRef*>(holder_);
}

}  // namespace EscargotShim
