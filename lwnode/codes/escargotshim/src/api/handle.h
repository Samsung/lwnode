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

class HandleWrap {
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

  void* operator new(size_t size) { return Escargot::Memory::gcMalloc(size); }
  void* operator new(size_t, void* ptr) { return ptr; }
  void* operator new[](size_t size) = delete;
  void operator delete(void* ptr) { Escargot::Memory::gcFree(ptr); }
  void operator delete[](void* obj) = delete;

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
    } else if (std::is_base_of<Escargot::ValueRef, T>::value) {
      LWNODE_CHECK((std::is_base_of<v8::Value, V8>::value));
      setType(HandleWrap::Type::JsValue);
    } else {
      setType(HandleWrap::Type::Unknown);
    }

    m_holder = ptr;
  }

  static ValueWrap<T, V8>* New(T* that);

  ValueWrap(V8* ptr) {
    auto base = reinterpret_cast<HandleWrap*>(ptr);
    auto src = static_cast<ValueWrap<T, V8>*>(base);

    if (std::is_same<v8::Context, V8>::value) {
      LWNODE_CHECK((std::is_same<ContextWrap, T>::value));
      LWNODE_CHECK(src->type() == HandleWrap::Type::Context);
    } else if (std::is_base_of<v8::Value, V8>::value) {
      LWNODE_CHECK((std::is_base_of<Escargot::ValueRef, T>::value));
      LWNODE_CHECK(src->type() == HandleWrap::Type::JsValue);
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

template <typename T, typename V8>
ValueWrap<T, V8>* ValueWrap<T, V8>::New(T* that) {
  return new ValueWrap<T, V8>(that);
}

}  // namespace EscargotShim
