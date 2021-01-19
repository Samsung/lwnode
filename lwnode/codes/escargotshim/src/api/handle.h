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

namespace EscargotShim {

class ContextWrap;

class HandleWrap : public gc {
 public:
  enum Type {
    Context,
    Script,
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
  virtual ~ValueWrap() { m_holder = nullptr; };

  ValueWrap(T* ptr) {
    if (std::is_same<ContextWrap, T>::value) {
      LWNODE_CHECK((std::is_same<v8::Context, V8>::value));
      setType(HandleWrap::Type::Context);

    } else if (std::is_same<Escargot::ScriptRef, T>::value) {
      LWNODE_CHECK((std::is_same<v8::Script, V8>::value) ||
                   (std::is_same<v8::UnboundScript, V8>::value));
      setType(HandleWrap::Type::Script);

    } else if (std::is_base_of<Escargot::ValueRef, T>::value) {
      LWNODE_CHECK((std::is_base_of<v8::Value, V8>::value));
      setType(HandleWrap::Type::JsValue);

    } else {
      LWNODE_CHECK_MSG(false, "No matched type");
    }

    m_holder = ptr;
  }

  ValueWrap(V8* ptr) {
    auto base = reinterpret_cast<HandleWrap*>(ptr);
    auto src = static_cast<ValueWrap<T, V8>*>(base);

    if (std::is_same<v8::Context, V8>::value) {
      LWNODE_CHECK((std::is_same<ContextWrap, T>::value));
      LWNODE_CHECK(src->type() == HandleWrap::Type::Context);

    } else if (std::is_same<v8::Script, V8>::value ||
               std::is_same<v8::UnboundScript, V8>::value) {
      LWNODE_CHECK((std::is_same<Escargot::ScriptRef, T>::value));
      LWNODE_CHECK(src->type() == HandleWrap::Type::Script);

    } else if (std::is_base_of<v8::Value, V8>::value) {
      LWNODE_CHECK((std::is_base_of<Escargot::ValueRef, T>::value));
      LWNODE_CHECK(src->type() == HandleWrap::Type::JsValue);

    } else {
      LWNODE_CHECK_MSG(false, "No matched type");
    }

    setType(src->type());
    m_holder = src->m_holder;
  }

  ValueWrap(ValueWrap<T, V8>&& src) {
    m_holder = src.m_holder;
    src.m_holder = nullptr;
  }

  ValueWrap(const ValueWrap<T, V8>&) = delete;

  const ValueWrap<T, V8>& operator=(const ValueWrap<T, V8>&) = delete;
  const ValueWrap<T, V8>& operator=(ValueWrap<T, V8>&& src) {
    m_holder = src.m_holder;
    src.m_holder = nullptr;
    return *this;
  }

  operator T*() { return m_holder; }

  static ValueWrap<T, V8>* New(T* that);
  static T* fromV8(v8::Local<V8> local);
  static T* fromV8(V8* ptr);

  v8::Local<V8> toLocal(v8::Isolate* isolate) {
    return v8::Local<V8>::New(isolate, this);
  }

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

template <typename T, typename V8>
ValueWrap<T, V8>* ValueWrap<T, V8>::New(T* that) {
  return new ValueWrap<T, V8>(that);
}

template <typename T, typename V8>
T* ValueWrap<T, V8>::fromV8(v8::Local<V8> local) {
  ValueWrap<T, V8> value(*local);
  return value.get();
}

template <typename T, typename V8>
T* ValueWrap<T, V8>::fromV8(V8* ptr) {
  ValueWrap<T, V8> value(ptr);
  return value.get();
}

}  // namespace EscargotShim
