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

#include "api.h"
#include "base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace {
class TemplateData : public gc {
 public:
  TemplateData(v8::Isolate* isolate) : m_isolate(isolate) {}

  v8::Isolate* m_isolate{nullptr};
};

class FunctionTemplateData : public TemplateData {
 public:
  FunctionTemplateData(v8::Isolate* isolate,
                       v8::FunctionCallback callback,
                       v8::Local<v8::Value> data,
                       v8::Local<v8::Signature> signature,
                       int length)
      : TemplateData(isolate),
        m_callback(callback),
        m_callbackData(data),
        m_signature(signature),
        m_length(length) {}

  static FunctionTemplateData* toFunctionTemplateData(void* ptr) {
    return reinterpret_cast<FunctionTemplateData*>(ptr);
  }

  v8::FunctionCallback m_callback;
  v8::Local<v8::Value> m_callbackData;
  v8::Local<v8::Signature> m_signature;
  int m_length{0};
};
}  // namespace

namespace v8 {
// --- T e m p l a t e ---

void Template::Set(v8::Local<Name> name,
                   v8::Local<Data> value,
                   v8::PropertyAttribute attribute) {
  LWNODE_UNIMPLEMENT;
}

void Template::SetPrivate(v8::Local<Private> name,
                          v8::Local<Data> value,
                          v8::PropertyAttribute attribute) {}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
                                   v8::Local<FunctionTemplate> getter,
                                   v8::Local<FunctionTemplate> setter,
                                   v8::PropertyAttribute attribute,
                                   v8::AccessControl access_control) {
  LWNODE_RETURN_VOID;
}

// --- F u n c t i o n   T e m p l a t e ---

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void FunctionTemplate::SetPrototypeProviderTemplate(
    Local<FunctionTemplate> prototype_provider) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  LWNODE_RETURN_VOID;
}

static ValueRef* FunctionTemplateNativeFunction(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    OptionalRef<ObjectRef> newTarget) {
  Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
      state->resolveCallee();
  auto ptr = callee->extraData();
  LWNODE_DCHECK_NOT_NULL(callee->extraData());
  auto tpData =
      FunctionTemplateData::toFunctionTemplateData(callee->extraData());

  Local<Value> result;
  if (tpData->m_callback) {
    FunctionCallbackInfoWrap info(
        tpData->m_isolate, thisValue, thisValue, argc, argv);
    tpData->m_callback(info);
    result = info.GetReturnValue().Get();
    // TODO: error check from 'state'
  }

  if (newTarget.hasValue()) {
    return thisValue;
  }
  if (!result.IsEmpty()) {
    return VAL(*result)->value();
  }
  return ValueRef::createUndefined();
}

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
                                              FunctionCallback callback,
                                              v8::Local<Value> data,
                                              v8::Local<Signature> signature,
                                              int length,
                                              ConstructorBehavior behavior,
                                              SideEffectType side_effect_type,
                                              const CFunction* c_function) {
  API_ENTER_NO_EXCEPTION(isolate);
  bool isConstructor = false;
  if (behavior == ConstructorBehavior::kAllow) {
    isConstructor = true;
  }

  auto esFunctionTemplate =
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),  // name
                                  length,         // argumentCount
                                  false,          // isStrict
                                  isConstructor,  // isConstruction
                                  FunctionTemplateNativeFunction);  // fn

  esFunctionTemplate->setInstanceExtraData(
      new FunctionTemplateData(isolate, callback, data, signature, length));

  return Local<FunctionTemplate>::New(
      isolate, ValueWrap::createFunctionTemplate(esFunctionTemplate));
}

Local<FunctionTemplate> FunctionTemplate::NewWithCache(
    Isolate* isolate,
    FunctionCallback callback,
    Local<Private> cache_property,
    Local<Value> data,
    Local<Signature> signature,
    int length,
    SideEffectType side_effect_type) {
  LWNODE_RETURN_LOCAL(FunctionTemplate);
}

Local<Signature> Signature::New(Isolate* isolate,
                                Local<FunctionTemplate> receiver) {
  LWNODE_RETURN_LOCAL(Signature);
}

Local<AccessorSignature> AccessorSignature::New(
    Isolate* isolate, Local<FunctionTemplate> receiver) {
  LWNODE_RETURN_LOCAL(AccessorSignature);
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data,
                                      SideEffectType side_effect_type,
                                      const CFunction* c_function) {
  LWNODE_RETURN_VOID;
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void FunctionTemplate::SetLength(int length) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::SetClassName(Local<String> name) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::SetAcceptAnyReceiver(bool value) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::ReadOnlyPrototype() {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::RemovePrototype() {
  LWNODE_RETURN_VOID;
}

MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Function>());
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esFunctionTemplate = CVAL(this)->functionTemplate();

  API_RETURN_LOCAL(
      Function, lwIsolate->toV8(), esFunctionTemplate->instantiate(esContext));
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
  LWNODE_RETURN_LOCAL(Object);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  LWNODE_RETURN_FALSE;
}

// --- O b j e c t T e m p l a t e ---

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void Template::SetNativeDataProperty(v8::Local<String> name,
                                     AccessorGetterCallback getter,
                                     AccessorSetterCallback setter,
                                     v8::Local<Value> data,
                                     PropertyAttribute attribute,
                                     v8::Local<AccessorSignature> signature,
                                     AccessControl settings,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetNativeDataProperty(v8::Local<Name> name,
                                     AccessorNameGetterCallback getter,
                                     AccessorNameSetterCallback setter,
                                     v8::Local<Value> data,
                                     PropertyAttribute attribute,
                                     v8::Local<AccessorSignature> signature,
                                     AccessControl settings,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetLazyDataProperty(v8::Local<Name> name,
                                   AccessorNameGetterCallback getter,
                                   v8::Local<Value> data,
                                   PropertyAttribute attribute,
                                   SideEffectType getter_side_effect_type,
                                   SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetIntrinsicDataProperty(Local<Name> name,
                                        Intrinsic intrinsic,
                                        PropertyAttribute attribute) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessor(v8::Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter,
                                 v8::Local<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessor(v8::Local<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 v8::Local<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::MarkAsUndetectable() {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessCheckCallback(AccessCheckCallback callback,
                                            Local<Value> data) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessCheckCallbackAndHandler(
    AccessCheckCallback callback,
    const NamedPropertyHandlerConfiguration& named_handler,
    const IndexedPropertyHandlerConfiguration& indexed_handler,
    Local<Value> data) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetHandler(
    const IndexedPropertyHandlerConfiguration& config) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetCallAsFunctionHandler(FunctionCallback callback,
                                              Local<Value> data) {
  LWNODE_RETURN_VOID;
}

int ObjectTemplate::InternalFieldCount() {
  LWNODE_RETURN_0;
}

void ObjectTemplate::SetInternalFieldCount(int value) {}

bool ObjectTemplate::IsImmutableProto() {
  LWNODE_RETURN_FALSE;
}

void ObjectTemplate::SetImmutableProto() {}
}  // namespace v8
