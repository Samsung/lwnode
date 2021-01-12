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
#include "utils/gc.h"

namespace EscargotShim {

class HandleWrap : public gc {
 public:
  enum Type : uint8_t {
    Context,
    JsValue,
    ObjectTemplate,
    FunctionTemplate,
  };

  enum MemType : uint8_t {
    Local,
    Persistent,
    PersistentWeak,
  };

  HandleWrap(Type type, MemType memType)
      : m_type(static_cast<uint8_t>(type)),
        m_memType(static_cast<uint8_t>(memType)) {}

  Type type() const { return static_cast<Type>(m_type); }
  MemType memType() const { return static_cast<MemType>(m_memType); }

 protected:
  uint8_t m_type;
  uint8_t m_memType;
};

class JsValue : public HandleWrap {
 public:
  JsValue(Escargot::ValueRef* jvalue, MemType memType)
      : HandleWrap(HandleWrap::Type::JsValue, memType = HandleWrap::MemType::Local),
        m_value(reinterpret_cast<void*>(jvalue)) {}

  JsValue(Escargot::ContextRef* jvalue, MemType memType = HandleWrap::MemType::Local)
      : HandleWrap(HandleWrap::Type::Context, memType),
        m_value(reinterpret_cast<void*>(jvalue)) {}

 private:
  void* m_value = nullptr;
};

}  // namespace EscargotShim
