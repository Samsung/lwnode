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

using namespace Escargot;

namespace EscargotShim {

ValueWrap* ValueWrap::createContext(IsolateWrap* _isolate) {
  LWNODE_CHECK_NOT_NULL(_isolate);
  auto _context = ContextWrap::New(_isolate);
  return new ValueWrap(_context, Type::Context);
};

ContextWrap* ValueWrap::context() const {
  LWNODE_CHECK(type() == Type::Context);
  return reinterpret_cast<ContextWrap*>(m_holder);
}

ValueWrap* ValueWrap::createScript(ScriptRef* __script) {
  return new ValueWrap(__script, Type::Script);
};

ScriptRef* ValueWrap::script() const {
  LWNODE_CHECK(type() == Type::Script);
  auto extended = reinterpret_cast<ExtendedHolder*>(m_holder);
  return reinterpret_cast<ScriptRef*>(extended->holder());
}

ValueWrap* ValueWrap::createValue(Escargot::ValueRef* __value) {
  return new ValueWrap(__value, Type::JsValue);
}

ValueRef* ValueWrap::value() const {
  LWNODE_CHECK(type() == Type::JsValue);
  return reinterpret_cast<ValueRef*>(m_holder);
}

}  // namespace EscargotShim
