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

typedef GCContainer<void*> ExtraData;

class HandleWrap : public gc {
 public:
  enum Type : uint8_t {
    JsValue = 0,
    Context,
    ObjectTemplate,
    FunctionTemplate,
    // Only types having an ExtraData are allowed after this point
    ExtraDataPresent,
    Script,
    // NotPresent should be at last
    NotPresent,
  };

  uint8_t type() const { return type_; }
  bool isValid() const { return (type_ < HandleWrap::Type::NotPresent); }

 protected:
  HandleWrap() = default;
  uint8_t type_ = NotPresent;
};

class ValueWrap : public HandleWrap {
 public:
  ValueWrap(const ValueWrap& src) = delete;
  const ValueWrap& operator=(const ValueWrap& src) = delete;
  const ValueWrap& operator=(ValueWrap&& src) = delete;

  // Extra
  void setExtra(ExtraData&& other) {
    LWNODE_CHECK(type() >= ExtraDataPresent);
    auto newHolder = new ExtendedHolder(holder_, std::move(other));
    holder_ = reinterpret_cast<Escargot::ValueRef*>(newHolder);
  }

  template <typename E>
  Optional<E> getExtra(const size_t idx) const {
    LWNODE_CHECK(type() >= ExtraDataPresent);
    auto extended = reinterpret_cast<ExtendedHolder*>(holder_);
    return reinterpret_cast<E*>((*extended->extra())[idx]);
  }

  // Value
  static ValueWrap* createValue(Escargot::ValueRef* esValue);
  Escargot::ValueRef* value() const;

  // Context
  // @todo: use factory to create Escargot instances
  static ValueWrap* createContext(IsolateWrap* lwIsolate);
  ContextWrap* context() const;

  // Script
  static ValueWrap* createScript(Escargot::ScriptRef* esScript);
  Escargot::ScriptRef* script() const;

 private:
  class ExtendedHolder : public gc {
   public:
    ExtendedHolder(void* ptr, ExtraData&& other) {
      LWNODE_CHECK(extra_ == nullptr);
      LWNODE_CHECK(holder_ == nullptr);
      holder_ = ptr;
      extra_ = new ExtraData(std::move(other));
    }
    inline void* holder() { return holder_; }
    inline ExtraData* extra() { return extra_; }

   private:
    void* holder_ = nullptr;
    ExtraData* extra_ = nullptr;
  };

  ValueWrap(void* ptr, HandleWrap::Type type) {
    LWNODE_CHECK_NOT_NULL(ptr);
    type_ = type;
    holder_ = ptr;
  }

  void* holder_ = nullptr;
};

}  // namespace EscargotShim
