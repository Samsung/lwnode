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

namespace v8 {
// --- T e m p l a t e ---

void Template::Set(v8::Local<Name> name,
                   v8::Local<Data> value,
                   v8::PropertyAttribute attribute) {
  bool isWritable = !(attribute & ReadOnly);
  bool isEnumerable = !(attribute & DontEnum);
  bool isConfigurable = !(attribute & DontDelete);

  TemplateRef* esTemplate = CVAL(this)->tpl();

  // Name can be either a string or symbol
  auto esName = CVAL(*name)->value();
  TemplatePropertyNameRef esPropertyName;
  if (esName->isString()) {
    esPropertyName = TemplatePropertyNameRef(esName->asString());
  } else if (esName->isSymbol()) {
    esPropertyName = TemplatePropertyNameRef(esName->asSymbol());
  }

  auto lwValue = CVAL(*value);
  if (lwValue->type() == HandleWrap::Type::ObjectTemplate ||
      lwValue->type() == HandleWrap::Type::FunctionTemplate) {
    esTemplate->set(esPropertyName,
                    lwValue->tpl(),
                    isWritable,
                    isEnumerable,
                    isConfigurable);
  } else {
    esTemplate->set(esPropertyName,
                    lwValue->value(),
                    isWritable,
                    isEnumerable,
                    isConfigurable);
  }
}

void Template::SetPrivate(v8::Local<Private> name,
                          v8::Local<Data> value,
                          v8::PropertyAttribute attribute) {
  LWNODE_RETURN_VOID;
}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
                                   v8::Local<FunctionTemplate> getter,
                                   v8::Local<FunctionTemplate> setter,
                                   v8::PropertyAttribute attribute,
                                   v8::AccessControl access_control) {
  auto esTemplate = CVAL(this)->tpl();
  auto esName = CVAL(*name)->value()->asString();
  FunctionTemplateRef* esGetter = nullptr;
  if (!getter.IsEmpty()) {
    esGetter = CVAL(*getter)->ftpl();
  }
  FunctionTemplateRef* esSetter = nullptr;
  if (!setter.IsEmpty()) {
    esSetter = CVAL(*setter)->ftpl();
  }

  esTemplate->setAccessorProperty(esName,
                                  OptionalRef<FunctionTemplateRef>(esGetter),
                                  OptionalRef<FunctionTemplateRef>(esSetter),
                                  !(attribute & DontEnum),
                                  !(attribute & DontDelete));
}

// --- F u n c t i o n   T e m p l a t e ---

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  FunctionTemplateRef* esFunctionTemplate = CVAL(this)->ftpl();

  return Utils::NewLocal(IsolateWrap::GetCurrent()->toV8(),
                         esFunctionTemplate->prototypeTemplate());
}

void FunctionTemplate::SetPrototypeProviderTemplate(
    Local<FunctionTemplate> prototype_provider) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  CVAL(this)->ftpl()->inherit(CVAL(*value)->ftpl());
}

static ValueRef* FunctionTemplateNativeFunction(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    OptionalRef<ObjectRef> newTarget) {
  Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
      state->resolveCallee();
  LWNODE_DCHECK_NOT_NULL(callee->extraData());
  auto fnData = FunctionData::toFunctionData(callee->extraData());

  LWNODE_CALL_TRACE_ID(TEMPLATE,
                       "es: %p newTarget: %s",
                       thisValue,
                       strBool(newTarget.hasValue()));

  auto lwIsolate = IsolateWrap::GetCurrent();
  if (!fnData->checkSignature(state, thisValue)) {
    lwIsolate->ScheduleThrow(TypeErrorObjectRef::create(
        state, StringRef::createFromASCII("Illegal invocation")));
    lwIsolate->ThrowErrorIfHasException(state);
    LWNODE_DLOG_ERROR("Signature mismatch!");
    return ValueRef::createUndefined();
  }

  auto thisObject = thisValue->asObject();
  if (newTarget.hasValue() && ObjectRefHelper::hasExtraData(thisObject)) {
    auto objectData = ObjectRefHelper::getExtraData(thisObject)->clone();
    objectData->setInstanceTemplate(fnData->instanceTemplate());
    ObjectRefHelper::setExtraData(thisObject, objectData, true);
  }

  Local<Value> result;
  if (fnData->callback()) {
    LWNODE_CALL_TRACE_ID(TEMPLATE, "> Call JS callback");
    FunctionCallbackInfoWrap info(fnData->isolate(),
                                  thisValue,
                                  thisValue,
                                  newTarget,
                                  VAL(fnData->callbackData()),
                                  argc,
                                  argv);
    fnData->callback()(info);
    lwIsolate->ThrowErrorIfHasException(state);
    result = info.GetReturnValue().Get();
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
  if (c_function != nullptr) {
    LWNODE_RETURN_LOCAL(FunctionTemplate);
  }

  if (side_effect_type == SideEffectType::kHasSideEffect) {
    LWNODE_ONCE(LWNODE_DLOG_WARN("@ignored/SideEffectType::kHasSideEffect"));
  }

  API_ENTER_NO_EXCEPTION(isolate, TEMPLATE);
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

  auto functionData = new FunctionData(isolate, *callback, *data, *signature);
  functionData->setInstanceTemplate(esFunctionTemplate);
  FunctionTemplateRefHelper::setInstanceExtraData(esFunctionTemplate,
                                                  functionData);

  return Utils::NewLocal(isolate, esFunctionTemplate);
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
  return Utils::NewLocalSignature(isolate, VAL(*receiver));
}

Local<AccessorSignature> AccessorSignature::New(
    Isolate* isolate, Local<FunctionTemplate> receiver) {
  LWNODE_RETURN_LOCAL(AccessorSignature);
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data,
                                      SideEffectType side_effect_type,
                                      const CFunction* c_function) {
  if (c_function != nullptr ||
      side_effect_type != SideEffectType::kHasSideEffect) {
    LWNODE_RETURN_VOID;
  }

  Escargot::FunctionTemplateRef* esFunctionTemplate = CVAL(this)->ftpl();
  auto fnData =
      FunctionTemplateRefHelper::getInstanceExtraData(esFunctionTemplate);
  fnData->setCallback(callback);
  fnData->setCallbackData(*data);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  FunctionTemplateRef* esFunctionTemplate = CVAL(this)->ftpl();

  return Utils::NewLocal(IsolateWrap::GetCurrent()->toV8(),
                         esFunctionTemplate->instanceTemplate());
}

void FunctionTemplate::SetLength(int length) {
  FunctionTemplateRef* self = CVAL(this)->ftpl();
  self->setLength(length);
}

void FunctionTemplate::SetClassName(Local<String> name) {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();
  FunctionTemplateRef* self = CVAL(this)->ftpl();
  auto esName = CVAL(*name)->value()->asString();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState,
         FunctionTemplateRef* esFunctionTemplate,
         StringRef* esName) -> ValueRef* {
        esFunctionTemplate->setName(
            AtomicStringRef::create(esState->context(), esName));
        return ValueRef::createNull();
      },
      self,
      esName);
  LWNODE_CHECK(r.isSuccessful());
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
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Function>(), TEMPLATE);
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esFunctionTemplate = CVAL(this)->ftpl();

  return Utils::NewLocal<Function>(lwIsolate->toV8(),
                                   esFunctionTemplate->instantiate(esContext));
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
  LWNODE_RETURN_LOCAL(Object);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  LWNODE_CALL_TRACE();
  auto esContext = IsolateWrap::GetCurrent()->GetCurrentContext()->get();
  auto esValue = CVAL(*value)->value();
  if (!esValue->isObject()) {
    return false;
  }
  auto esObject = esValue->asObject();

  if (!ObjectRefHelper::hasExtraData(esObject)) {
    return false;
  }

  auto esSelf = CVAL(this)->ftpl();
  auto tpl = ObjectRefHelper::getExtraData(esObject)->instanceTemplate();
  if (esSelf == tpl) {
    return true;
  }

  auto parent = tpl->parent();
  while (parent.hasValue()) {
    if (esSelf == parent.value()) {
      return true;
    }
    parent = parent->parent();
  }

  return false;
}

// --- O b j e c t T e m p l a t e ---

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  API_ENTER_NO_EXCEPTION(isolate, TEMPLATE);

  return Utils::NewLocal(isolate, ObjectTemplateRef::create());
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
  // @note AccessControl is not considered.
  ObjectTemplateUtils::SetAccessor(CVAL(this)->otpl(),
                                   IsolateWrap::GetCurrent(),
                                   name,
                                   getter,
                                   setter,
                                   data,
                                   attribute);
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
  // @note AccessControl is not considered.
  ObjectTemplateUtils::SetAccessor(CVAL(this)->otpl(),
                                   IsolateWrap::GetCurrent(),
                                   name,
                                   getter,
                                   setter,
                                   data,
                                   attribute);
}

struct HandlerConfiguration : public gc {
  HandlerConfiguration(
      v8::Isolate* isolate,
      const v8::NamedPropertyHandlerConfiguration& namedPropertyHandler)
      : m_isolate(isolate), m_namedPropertyHandler(namedPropertyHandler) {}
  v8::Isolate* m_isolate;
  v8::NamedPropertyHandlerConfiguration m_namedPropertyHandler;
};

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  ObjectTemplateRef* esObjectTemplate = CVAL(this)->otpl();
  HandlerConfiguration* handlerConfiguration =
      new HandlerConfiguration(IsolateWrap::GetCurrent()->toV8(), config);

  ObjectTemplateNamedPropertyHandlerData esHandlerData;
  if (config.getter) {
    esHandlerData.getter = [](ExecutionStateRef* state,
                              ObjectRef* esSelf,
                              ValueRef* esReceiver,
                              void* data,
                              const TemplatePropertyNameRef& esPropertyName)
        -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.getter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.getter(v8PropertyName, info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        return CVAL(*ret)->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.setter) {
    esHandlerData.setter = [](ExecutionStateRef* state,
                              ObjectRef* esSelf,
                              ValueRef* esReceiver,
                              void* data,
                              const TemplatePropertyNameRef& esPropertyName,
                              ValueRef* esValue) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.setter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName =
          v8::Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.setter(
          v8PropertyName, v8::Utils::ToLocal<Value>(esValue), info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        if (ret->IsFalse()) {
          return Escargot::OptionalRef<Escargot::ValueRef>(
              ValueRef::create(false));
        }
        return Escargot::OptionalRef<Escargot::ValueRef>(
            ValueRef::create(true));
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.query) {
    esHandlerData.query = [](ExecutionStateRef* state,
                             ObjectRef* esSelf,
                             ValueRef* esReceiver,
                             void* data,
                             const TemplatePropertyNameRef& esPropertyName)
        -> TemplatePropertyAttribute {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.query) {
        return TemplatePropertyAttribute::TemplatePropertyAttributeNotExist;
      }

      Local<Name> v8PropertyName =
          v8::Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<Integer> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.query(v8PropertyName, info);
      Local<Value> ret = info.GetReturnValue().Get();
      if (info.hasReturnValue()) {
        bool hasNone = (handlerConfiguration->m_namedPropertyHandler.flags ==
                        PropertyHandlerFlags::kNone);
        bool hasNoSideEffect =
            (static_cast<int>(
                 handlerConfiguration->m_namedPropertyHandler.flags) &
             static_cast<int>(PropertyHandlerFlags::kHasNoSideEffect));

        if (hasNone) {
          return TemplatePropertyAttribute::TemplatePropertyAttributeExist;
        } else if (hasNoSideEffect) {
          return TemplatePropertyAttribute::TemplatePropertyAttributeEnumerable;
        } else {
          LWNODE_UNIMPLEMENT;
        }
        return TemplatePropertyAttribute::TemplatePropertyAttributeExist;
      }

      return TemplatePropertyAttribute::TemplatePropertyAttributeNotExist;
    };
  }

  if (config.deleter) {
    esHandlerData.deleter = [](ExecutionStateRef* state,
                               ObjectRef* esSelf,
                               ValueRef* esReceiver,
                               void* data,
                               const TemplatePropertyNameRef& esPropertyName)
        -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.deleter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName =
          v8::Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<v8::Boolean> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.deleter(v8PropertyName,
                                                           info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        return CVAL(*ret)->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.enumerator) {
    esHandlerData.enumerator = [](ExecutionStateRef* state,
                                  ObjectRef* esSelf,
                                  ValueRef* esReceiver,
                                  void* data)
        -> TemplateNamedPropertyHandlerEnumerationCallbackResultVector {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.enumerator) {
        return TemplateNamedPropertyHandlerEnumerationCallbackResultVector(0);
      }

      PropertyCallbackInfoWrap<v8::Array> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.enumerator(info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        auto esArray = CVAL(*ret)->value()->asArrayObject();
        auto length = esArray->length(state);
        auto v =
            TemplateNamedPropertyHandlerEnumerationCallbackResultVector(length);

        for (size_t i = 0; i < length; i++) {
          v[i] = esArray->get(state, ValueRef::create(i))->toString(state);
        }
        return v;
      }

      return TemplateNamedPropertyHandlerEnumerationCallbackResultVector(0);
    };
  }

  if (config.definer) {
    esHandlerData.definer =
        [](ExecutionStateRef* state,
           ObjectRef* esSelf,
           ValueRef* esReceiver,
           void* data,
           const TemplatePropertyNameRef& esPropertyName,
           const ObjectPropertyDescriptorRef& esDesc) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.definer) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName =
          v8::Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      PropertyDescriptor desc;
      desc.get_private()->setDescriptor(
          const_cast<ObjectPropertyDescriptorRef*>(&esDesc));
      handlerConfiguration->m_namedPropertyHandler.definer(
          v8PropertyName, desc, info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        return Escargot::OptionalRef<Escargot::ValueRef>(CVAL(*ret)->value());
      }
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.descriptor) {
    esHandlerData.descriptor = [](ExecutionStateRef* state,
                                  ObjectRef* esSelf,
                                  ValueRef* esReceiver,
                                  void* data,
                                  const TemplatePropertyNameRef& esPropertyName)
        -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.descriptor) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName =
          v8::Utils::ToLocal<Name>(esPropertyName.value());

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.descriptor(v8PropertyName,
                                                              info);

      if (info.hasReturnValue()) {
        Local<Value> ret = info.GetReturnValue().Get();
        return Escargot::OptionalRef<Escargot::ValueRef>(CVAL(*ret)->value());
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  esHandlerData.data = handlerConfiguration;
  esObjectTemplate->setNamedPropertyHandler(esHandlerData);
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
  auto esObjectTemplate = CVAL(this)->otpl();
  return ObjectTemplateRefHelper::getInternalFieldCount(esObjectTemplate);
}

void ObjectTemplate::SetInternalFieldCount(int value) {
  auto esObjectTemplate = CVAL(this)->otpl();
  if (esObjectTemplate->didInstantiate()) {
    LWNODE_DLOG_WARN(
        "Don't modify internal field count after instantiating object");
    return;
  }
  ObjectTemplateRefHelper::setInternalFieldCount(esObjectTemplate, value);
}

bool ObjectTemplate::IsImmutableProto() {
  LWNODE_RETURN_FALSE;
}

void ObjectTemplate::SetImmutableProto() {
  LWNODE_RETURN_VOID;
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<v8::Object>(), TEMPLATE);
  auto esContext = VAL(*context)->context()->get();
  auto esObjectTemplate = CVAL(this)->otpl();

  auto newObject = esObjectTemplate->instantiate(esContext);
  if (ObjectRefHelper::getExtraData(newObject)) {
    auto objectData = ObjectRefHelper::getExtraData(newObject);
    ObjectRefHelper::setExtraData(newObject, objectData->clone(), true);
  }
  return Utils::NewLocal<Object>(lwIsolate->toV8(), newObject);
}
}  // namespace v8
