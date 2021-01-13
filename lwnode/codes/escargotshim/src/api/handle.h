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
#include <type_traits>

#include "utils/gc.h"
#include "utils/misc.h"

namespace EscargotShim {

class ContextWrap;

class HandleWrap : public gc {
 public:
  enum Type {
    Context,
    JsValue,
    ObjectTemplate,
    FunctionTemplate,
    Unknown,
  };

  HandleWrap(Type type) : m_type(type) {}

  Type type() const { return m_type; }
  void setType(Type type) { m_type = type; }

 protected:
  HandleWrap() = default;

 private:
  Type m_type = Type::Unknown;
};

template <typename T, typename V8>
class ValueWrap : public HandleWrap {
 public:
  virtual ~ValueWrap() {
    m_holder = nullptr;
  };

  ValueWrap(T* ptr) {
    if (std::is_same<T, ContextWrap>::value) {
      setType(HandleWrap::Type::Context);
    } else if (std::is_same<T, Escargot::ValueRef>::value) {
      setType(HandleWrap::Type::JsValue);
    } else {
      setType(HandleWrap::Type::Unknown);
    }

    m_holder = ptr;
  }

  ValueWrap(V8* ptr) {
    auto src = reinterpret_cast<ValueWrap<T, V8>*>(ptr);

    if (std::is_same<V8, v8::Context>::value) {
      LWNODE_ASSERT(src->type() == HandleWrap::Type::Context);
    } else if (std::is_same<V8, v8::Value>::value) {
      LWNODE_ASSERT(src->type() == HandleWrap::Type::JsValue);
    } else {
      LWNODE_ASSERT(0);
    }

    setType(src->type());
    m_holder = src->m_holder;
  }

  ValueWrap(ValueWrap<T, V8>&& src) {
    m_holder = src.m_holder;
    src.m_holder = nullptr;
  }

  const ValueWrap<T, V8>& operator=(ValueWrap<T, V8>&& src) {
    m_holder = src.m_holder;
    src.m_holder = nullptr;
    return *this;
  }

  ValueWrap(const ValueWrap<T, V8>&) = delete;
  const ValueWrap<T, V8>& operator=(const ValueWrap<T, V8>&) = delete;

  V8* toV8() {
    // This is safe since checking type is done at creation time.
    return reinterpret_cast<V8*>(this);
  }

  operator T*() { return m_holder; }
  T* get() { return m_holder; }
  T* reset() {
    if (m_holder) {
      T* ptr = m_holder;
      m_holder = nullptr;
      return ptr;
    }
    return nullptr;
  }

 private:
  T* m_holder = nullptr;
};

}  // namespace EscargotShim
