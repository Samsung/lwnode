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

#include "misc.h"

namespace EscargotShim {

template <typename T>
class Optional {
 public:
  Optional() : m_value(nullptr) {}

  Optional(T* value) : m_value(value) {}

  Optional(std::nullptr_t value) : m_value(nullptr) {}

  Optional(const Optional<T>& src) : m_value(src.m_value) {}

  T* value() { return m_value; }

  const T* value() const { return m_value; }

  T* get() { return m_value; }

  const T* get() const { return m_value; }

  // If this value is empty, it will crash the process.
  T* getChecked() {
    LWNODE_CHECK_NOT_NULL(m_value);
    return m_value;
  }

  bool hasValue() const { return !!m_value; }

  operator bool() const { return hasValue(); }

  T* operator->() { return m_value; }

  const Optional<T>& operator=(const Optional<T>& other) {
    m_value = other.m_value;
    return *this;
  }

  bool operator==(const Optional<T>& other) const {
    return m_value == other.m_value;
  }

  bool operator!=(const Optional<T>& other) const {
    return !this->operator==(other);
  }

  bool operator==(const T*& other) const {
    if (hasValue()) {
      return *value() == *other;
    }
    return false;
  }

  bool operator!=(const T*& other) const { return !operator==(other); }

 protected:
  T* m_value = nullptr;
};

}  // namespace EscargotShim
