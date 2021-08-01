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
#include <sstream>
#include "api.h"
#include "context.h"
#include "engine.h"
#include "isolate.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

// --- HandleWrap ---

uint8_t HandleWrap::type() const {
  return type_;
}

uint8_t HandleWrap::valueType() const {
  return valueType_;
}

uint8_t HandleWrap::location() const {
  return location_;
}

bool HandleWrap::isValid() const {
  return (type_ >= Type::JsValue && type_ < Type::EndOfType);
}

bool HandleWrap::isStrongOrWeak() const {
  return (location_ == Strong || location_ == Weak || location_ == NearDeath);
}

void HandleWrap::copy(HandleWrap* that, Location location) {
  val_ = that->val_;
  type_ = that->type_;
  valueType_ = that->valueType_;
  location_ = location;
}

HandleWrap* HandleWrap::clone(Location location) {
  auto handle = new HandleWrap();
  handle->val_ = val_;
  handle->type_ = type_;
  handle->valueType_ = valueType_;
  handle->location_ = location;

  LWNODE_CALL_TRACE_ID(
      HANDLE, "%p is cloned from %s", handle, getHandleInfoString().c_str());
  return handle;
}

HandleWrap* HandleWrap::as(void* address) {
  auto p = reinterpret_cast<HandleWrap*>(address);
  LWNODE_CHECK(p->isValid());
  return p;
}

std::string HandleWrap::getHandleInfoString() const {
  std::stringstream ss;
  ss << "(addr: " << this << " es: " << val_;
  if (location_ != Location::Local) {
    ss << " loc: " << std::to_string(location_);
  }
  ss << " type: " << std::to_string(type_) << ")";
  return ss.str();
}

// --- ValueWrap ---

ValueWrap::ValueWrap(void* ptr,
                     HandleWrap::Type type,
                     HandleWrap::ValueType valueType) {
  LWNODE_CHECK_NOT_NULL(ptr);
  type_ = type;
  val_ = ptr;
  valueType_ = valueType;
  LWNODE_CALL_TRACE_ID(HANDLE, "%s", getHandleInfoString().c_str());
}

bool ValueWrap::isExternalString() const {
  return valueType_ == ValueWrap::ExternalString;
}

ExternalStringWrap* ValueWrap::asExternalString() const {
  LWNODE_CHECK(isExternalString());
  return reinterpret_cast<ExternalStringWrap*>(const_cast<ValueWrap*>(this));
}

ValueWrap* ValueWrap::createValue(Escargot::ValueRef* esValue) {
  return new ValueWrap(esValue, Type::JsValue);
}

ValueRef* ValueWrap::value() const {
  LWNODE_CHECK_MSG(type() == Type::JsValue,
                   "type should be %d but %d. (this: %p, val_: %p)",
                   Type::JsValue,
                   type(),
                   this,
                   val_);
  return reinterpret_cast<ValueRef*>(val_);
}

ValueWrap* ValueWrap::createContext(ContextWrap* lwContext) {
  return new ValueWrap(lwContext, Type::Context);
};

ContextWrap* ValueWrap::context() const {
  LWNODE_CHECK(type_ == Type::Context);
  return reinterpret_cast<ContextWrap*>(val_);
}

ValueWrap* ValueWrap::createScript(ScriptRef* esScript) {
  return new ValueWrap(esScript, Type::Script);
};

ScriptRef* ValueWrap::script() const {
  LWNODE_CHECK(type_ == Type::Script);
  return reinterpret_cast<ScriptRef*>(val_);
}

Escargot::TemplateRef* ValueWrap::tpl() const {
  LWNODE_CHECK(type_ == Type::ObjectTemplate ||
               type_ == Type::FunctionTemplate);
  return reinterpret_cast<TemplateRef*>(val_);
}

ValueWrap* ValueWrap::createFunctionTemplate(
    Escargot::FunctionTemplateRef* esTemplate) {
  return new ValueWrap(esTemplate, Type::FunctionTemplate);
}

Escargot::FunctionTemplateRef* ValueWrap::ftpl() const {
  LWNODE_CHECK(type_ == Type::FunctionTemplate);
  return reinterpret_cast<FunctionTemplateRef*>(val_);
}

ValueWrap* ValueWrap::createObjectTemplate(
    Escargot::ObjectTemplateRef* esTemplate) {
  return new ValueWrap(esTemplate, Type::ObjectTemplate);
}

Escargot::ObjectTemplateRef* ValueWrap::otpl() const {
  LWNODE_CHECK(type_ == Type::ObjectTemplate);
  return reinterpret_cast<ObjectTemplateRef*>(val_);
}

ValueWrap* ValueWrap::createModule(ModuleWrap* esModule) {
  return new ValueWrap(esModule, Type::Module);
}

ModuleWrap* ValueWrap::module() const {
  LWNODE_CHECK(type_ == Type::Module);
  return reinterpret_cast<ModuleWrap*>(val_);
}

// --- PersistentWrap ---

inline static GCHeap* GCHeap() {
  return Engine::current()->gcHeap();
}

inline static void acquireStrong(void* address, void* data = nullptr) {
  GCHeap()->acquire(address, GCHeap::STRONG, data);
}

inline static void releaseStrong(void* address) {
  GCHeap()->release(address, GCHeap::STRONG);
}

inline static void acquireWeak(void* address, void* data = nullptr) {
  GCHeap()->acquire(address, GCHeap::WEAK, data);
}

inline static void releaseWeak(void* address) {
  GCHeap()->release(address, GCHeap::WEAK);
}

inline static void makeStrongToWeak(void* address) {
  acquireWeak(address);
  releaseStrong(address);
}
/*
  1. val_ : HandleWrap MUST be compatible with in V8 other apis.

  ex1)
    v8::Persistent<String> global1;
    global1.Reset(isolate, v8_str("str"));
    Local<String> local1 = Local<String>::New(isolate, global1);
    CHECK(global1 == local1);

  ex2)
    v8::Context* operator->() {
      return *reinterpret_cast<v8::Context**>(&context_);
    }
    v8::Context* operator*() { return operator->(); }
    v8::Persistent<v8::Context> context_;

  2. member values of this instance MUST be hidden to GC.
*/

PersistentWrap::PersistentWrap(ValueWrap* lwValue) {
  // copy the given value information and set it as Strong
  copy(lwValue, Location::Strong);

  holder_ = lwValue;

  LWNODE_CALL_TRACE_ID(GCHEAP,
                       "%s trace: (%p)",
                       getPersistentInfoString().c_str(),
                       getTracingAddress());

  acquireStrong(this, nullptr);
  LWNODE_DCHECK(GCHeap()->isTraced(this));
}

void* PersistentWrap::getTracingAddress() {
  return this;
}

void* PersistentWrap::GlobalizeReference(v8::Isolate* isolate, void* address) {
  LWNODE_CALL_TRACE_ID(GCHEAP);
  ValueWrap* lwValue = reinterpret_cast<ValueWrap*>(address);
  LWNODE_CHECK(lwValue->isValid());

  PersistentWrap* persistent = new PersistentWrap(lwValue);

  persistent->v8Isolate_ = isolate;

  GCHeap()->printStatus(true);

  return persistent;
}

void PersistentWrap::DisposeGlobal(void* address) {
  PersistentWrap* persistent = reinterpret_cast<PersistentWrap*>(address);
  LWNODE_CALL_TRACE_ID(
      GCHEAP, "%s", persistent->getPersistentInfoString().c_str());

  LWNODE_DCHECK(persistent->location_ != Local);

  persistent->dispose();

  GCHeap()->printStatus(true);
}

void PersistentWrap::dispose() {
  LWNODE_CALL_TRACE_ID(GCHEAP,
                       "%s isFinalizerCalled: %s",
                       getPersistentInfoString().c_str(),
                       strBool(isFinalizerCalled));
  if (isFinalizerCalled) {
    return;
  }

  if (location_ == Location::Strong) {
    releaseStrong(getTracingAddress());
  } else if (location_ == Location::Weak) {
    releaseWeak(getTracingAddress());
  } else {
    LWNODE_CHECK_NOT_REACH_HERE();
  }
}

void PersistentWrap::MakeWeak(
    void* address,
    void* parameter,
    v8::WeakCallbackInfo<void>::Callback weak_callback) {
  LWNODE_CALL_TRACE_ID(GCHEAP);
  PersistentWrap* persitent = reinterpret_cast<PersistentWrap*>(address);
  LWNODE_CHECK(persitent->location() != Location::Local);
  persitent->makeWeak(parameter, weak_callback);

  GCHeap()->printStatus(true);
}

void PersistentWrap::makeWeak(
    void* parameter, v8::WeakCallbackInfo<void>::Callback weak_callback) {
  if (location_ == Location::Strong) {
    // Strong -> Weak
    makeStrongToWeak(getTracingAddress());
    location_ = Location::Weak;
    parameter_ = parameter;
    weak_callback_ = weak_callback;

  } else if (location_ == Location::Weak) {
    LWNODE_CHECK(weak_callback_ == weak_callback);
    LWNODE_CHECK(parameter_ == parameter);

  } else {
    LWNODE_CHECK_NOT_REACH_HERE();
  }

  LWNODE_CHECK(GCHeap()->isTraced(this));
}

void PersistentWrap::invokeFinalizer() {
  if (Engine::getState() == Engine::Running) {
    GCHeap()->disposePhantomWeak(this);
  }

  v8::HandleScope handleScope(v8Isolate_);
  void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {};
  v8::WeakCallbackInfo<void> data(
      v8Isolate_, parameter_, embedderFields, nullptr);

  weak_callback_(data);

  isFinalizerCalled = true;
}

void* PersistentWrap::ClearWeak(void* address) {
  LWNODE_CALL_TRACE_ID(GCHEAP);
  PersistentWrap* persitent = reinterpret_cast<PersistentWrap*>(address);
  LWNODE_CHECK(persitent->location() != Location::Local);
  void* p = persitent->clearWeak();
  GCHeap()->printStatus(true);
  return p;
}

void* PersistentWrap::clearWeak() {
  if (location_ == Location::Strong) {
    // Nothing to do
    LWNODE_CHECK_NULL(weak_callback_);
    LWNODE_CHECK_NULL(parameter_);

  } else if (location_ == Location::Weak) {
    // Weak -> Strong
    acquireStrong(getTracingAddress(), nullptr);
    releaseWeak(getTracingAddress());
    location_ = Location::Strong;

  } else {
    LWNODE_CHECK_NOT_REACH_HERE();
  }

  void* p = parameter_;
  parameter_ = nullptr;
  weak_callback_ = nullptr;
  return p;
}

PersistentWrap* PersistentWrap::as(void* address) {
  auto p = reinterpret_cast<PersistentWrap*>(address);
  LWNODE_CHECK(p->isStrongOrWeak());
  LWNODE_CHECK(p->isValid());
  return p;
}

std::string PersistentWrap::getPersistentInfoString() {
  std::stringstream ss;
  ss << getHandleInfoString() << " holder: " << holder_;

  return ss.str();
}

}  // namespace EscargotShim
