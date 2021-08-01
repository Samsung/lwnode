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
class ModuleWrap;
class ExternalStringWrap;
class GCHeap;

class HandleWrap : public gc {
 public:
  enum Type : uint8_t {
    NotPresent = 0,
    JsValue = 101,
    Context,
    ObjectTemplate,
    FunctionTemplate,
    Script,
    Module,
    EndOfType,
  };

  enum Location : uint8_t {
    Local = 0,
    Strong,
    Weak,
    NearDeath,
  };

  enum ValueType : uint8_t {
    None,
    ExternalString,
  };

  uint8_t type() const;
  uint8_t valueType() const;
  bool isValid() const;
  bool isStrongOrWeak() const;
  uint8_t location() const;
  HandleWrap* clone(Location location = Local);
  std::string getHandleInfoString() const;
  static HandleWrap* as(void* address);

 protected:
  HandleWrap() = default;
  void copy(HandleWrap* that, Location location);

  void* val_ = nullptr;
  uint8_t type_ = Type::NotPresent;
  uint8_t valueType_ = ValueType::None;  // TODO: remove this variable
  uint8_t location_ = Location::Local;
};

class ValueWrap : public HandleWrap {
 public:
  ValueWrap(const ValueWrap& src) = delete;
  const ValueWrap& operator=(const ValueWrap& src) = delete;
  const ValueWrap& operator=(ValueWrap&& src) = delete;

  bool isExternalString() const;
  ExternalStringWrap* asExternalString() const;

  // Value
  static ValueWrap* createValue(Escargot::ValueRef* esValue);
  Escargot::ValueRef* value() const;

  // Context
  static ValueWrap* createContext(ContextWrap* lwContext);
  ContextWrap* context() const;

  // Script
  static ValueWrap* createScript(Escargot::ScriptRef* esScript);
  Escargot::ScriptRef* script() const;

  // Template
  Escargot::TemplateRef* tpl() const;

  // FunctionTemplate
  static ValueWrap* createFunctionTemplate(
      Escargot::FunctionTemplateRef* esTemplate);
  Escargot::FunctionTemplateRef* ftpl() const;

  // ObjectTemplate
  static ValueWrap* createObjectTemplate(
      Escargot::ObjectTemplateRef* esTemplate);
  Escargot::ObjectTemplateRef* otpl() const;

  // Module
  static ValueWrap* createModule(ModuleWrap* esModule);
  ModuleWrap* module() const;

 protected:
  ValueWrap(void* ptr,
            HandleWrap::Type type,
            HandleWrap::ValueType valueType = HandleWrap::ValueType::None);
  ValueWrap() = default;
};

class PersistentWrap : public ValueWrap {
 public:
  static void* GlobalizeReference(v8::Isolate* isolate, void* address);
  static void DisposeGlobal(void* address);
  static void MakeWeak(void* address,
                       void* parameter,
                       v8::WeakCallbackInfo<void>::Callback weak_callback);
  static void* ClearWeak(void* address);

  static PersistentWrap* as(void* address);
  std::string getPersistentInfoString();
  void invokeFinalizer();

 private:
  PersistentWrap(ValueWrap* ptr);

  void dispose();
  void makeWeak(void* parameter,
                v8::WeakCallbackInfo<void>::Callback weak_callback);
  void* clearWeak();
  void* getTracingAddress();

  ValueWrap* holder_{nullptr};
  v8::Isolate* v8Isolate_{nullptr};
  v8::WeakCallbackInfo<void>::Callback weak_callback_{nullptr};
  void* parameter_{nullptr};
  bool isFinalizerCalled{false};
  friend class GCHeap;
};

}  // namespace EscargotShim
