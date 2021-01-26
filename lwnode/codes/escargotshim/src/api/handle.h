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
#include <type_traits>

#include "utils/gc.h"
#include "utils/misc.h"
#include "utils/optional.h"

namespace EscargotShim {

class ContextWrap;
class IsolateWrap;

typedef GCContainer<void*> ExtraValues;

class HandleWrap : public gc {
 public:
  enum Type : uint8_t {
    Context = 0,
    Script,
    JsValue,
    ObjectTemplate,
    FunctionTemplate,
    Unknown,
  };

  Type type() const { return m_type; }

 protected:
  HandleWrap() = default;
  Type m_type = Unknown;
};

class ValueWrap : public HandleWrap {
 public:
  ValueWrap(Escargot::ValueRef* __value) {
    LWNODE_CHECK_NOT_NULL(__value);
    m_type = Type::JsValue;
    m_holder = __value;
  }

  ValueWrap(ValueWrap* src) {
    LWNODE_CHECK_NOT_NULL(src);
    m_type = src->m_type;
    m_holder = src->m_holder;
    m_extra = src->m_extra;
  }

  ValueWrap(ValueWrap&& src) {
    m_holder = src.m_holder;
    m_extra = src.m_extra;

    src.m_holder = nullptr;
    src.m_extra = nullptr;
  }

  ValueWrap(const ValueWrap& src) = delete;
  const ValueWrap& operator=(const ValueWrap& src) = delete;
  const ValueWrap& operator=(ValueWrap&& src) = delete;

  // Extra
  void setExtra(ExtraValues&& other) {
    m_extra = new ExtraValues(std::move(other));
  }
  template <typename E>
  Optional<E> getExtra(const size_t idx) const {
    return reinterpret_cast<E*>((*m_extra)[idx]);
  }

  // Value
  Escargot::ValueRef* value() const { return m_holder; }

  // Context
  // @todo: use factory to create Escargot instances
  static ValueWrap* createContext(IsolateWrap* _isolate);
  ContextWrap* context() const;

  // Script
  static ValueWrap* createScript(Escargot::ScriptRef* __script);
  Escargot::ScriptRef* script() const;

  // Value
  static ValueWrap* createValue(Escargot::ValueRef* __value);

 private:
  // `void*` must be wrapped along with a type inside ValueWrap
  ValueWrap(void* ptr, HandleWrap::Type type) {
    m_type = type;
    m_holder = reinterpret_cast<Escargot::ValueRef*>(ptr);
  }

  Escargot::ValueRef* m_holder = nullptr;
  ExtraValues* m_extra = nullptr;
};

}  // namespace EscargotShim
