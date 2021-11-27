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
  auto lwValue = CVAL(*value);

  if (lwValue->type() == HandleWrap::Type::ObjectTemplate ||
      lwValue->type() == HandleWrap::Type::FunctionTemplate) {
    esTemplate->set(
        esName, lwValue->tpl(), isWritable, isEnumerable, isConfigurable);
  } else {
    esTemplate->set(
        esName, lwValue->value(), isWritable, isEnumerable, isConfigurable);
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
  auto esThisFunctionTemplate = CVAL(this)->ftpl();
  auto esThatFunctionTemplate = CVAL(*value)->ftpl();

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::Inherit(): %p",
                           esThisFunctionTemplate,
                           esThatFunctionTemplate);

  esThisFunctionTemplate->inherit(esThatFunctionTemplate);
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA,
      "FunctionTemplate(%p)::Inherit(): ExtraData1: %p, ExtraData2: %p",
      esThisFunctionTemplate,
      ExtraDataHelper::getExtraData(esThisFunctionTemplate),
      ExtraDataHelper::getExtraData(esThatFunctionTemplate));
}

// e.g.,
// sig_obj()
// var s = new sig_obj();
//  target: sig_obj == constructor obj, thisValue: s
// s.x();
//  target: null, thisValue: s
static ValueRef* FunctionTemplateNativeFunction(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    OptionalRef<ObjectRef> newTarget) {
  Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
      state->resolveCallee();

  auto calleeExtraData = ExtraDataHelper::getExtraData(callee.value());
  LWNODE_DCHECK_NOT_NULL(calleeExtraData);

  FunctionData* functionData = nullptr;
  // callee->extraData() is one of two types below
  if (calleeExtraData->isFunctionData()) {
    functionData = calleeExtraData->asFunctionData();
  } else if (calleeExtraData->isFunctionTemplateData()) {
    functionData = new FunctionData(
        calleeExtraData->asFunctionTemplateData()->functionTemplate());
  } else {
    LWNODE_CHECK(false);
  }

  LWNODE_CALL_TRACE_ID(TEMPLATE,
                       "es: %p newTarget: %s",
                       thisValue,
                       strBool(newTarget.hasValue()));

  if (newTarget.hasValue()) {
    auto extraData = ExtraDataHelper::getExtraData(newTarget.value());
    if (extraData) {
      // targetData was created in FunctionTemplate::GetFunction()
      auto targetData = extraData->asFunctionData();
      auto functionTemplate = targetData->functionTemplate();
      auto objectTemplate = functionTemplate->instanceTemplate();

      LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                               "FunctionTemplateNativeFunction::targetData: %p",
                               targetData);

      auto newInstance = thisValue->asObject();
      auto newInstanceData = ExtraDataHelper::getExtraData(newInstance);
      if (newInstanceData) {
        // newInstanceData was created in FunctionTemplate::InstanceTemplate()
        LWNODE_CHECK(newInstanceData->isObjectTemplateData());
        auto objectTemplateData = newInstanceData->asObjectTemplateData();
        LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                                 "FunctionTemplateNative: Existing Data 1: %p",
                                 newInstanceData);
        auto newObjectData =
            objectTemplateData->createObjectData(objectTemplate);
        // Replace ObjectTemplateData with its objectData, i.e., creating an
        // instance
        ObjectRefHelper::setExtraData(newInstance, newObjectData, true);
      } else {
        auto objectTemplateData = ExtraDataHelper::getExtraData(objectTemplate);
        if (objectTemplateData) {
          // newInstance was created from ObjectTemplate
          auto newObjectData =
              objectTemplateData->asObjectTemplateData()->createObjectData(
                  objectTemplate);
          LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                                   "FunctionTemplateNative: New ExtaData: %p",
                                   newObjectData);
          ObjectRefHelper::setExtraData(newInstance, newObjectData);
        } else {
          // newInstance was created from FunctionTemplate::GetFunction(), and
          // with "var s = new S();" in Javascript
          LWNODE_CHECK(targetData);
          auto newObjectData = new ObjectData(targetData->functionTemplate());
          LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                                   "FunctionTemplateNative: New ExtraData: %p",
                                   newObjectData);
          ObjectRefHelper::setExtraData(newInstance, newObjectData);
        }
      }
    } else {
      // newTarget was created from JavaScript without using Template
    }
  }

  auto lwIsolate = IsolateWrap::GetCurrent();
  if (!newTarget.hasValue() &&
      !functionData->checkSignature(state, thisValue->asObject())) {
    lwIsolate->ScheduleThrow(TypeErrorObjectRef::create(
        state, StringRef::createFromASCII("Illegal invocation")));
    lwIsolate->ThrowErrorIfHasException(state);
    LWNODE_DLOG_ERROR("Signature mismatch!");
    return ValueRef::createUndefined();
  }

  Local<Value> result;
  if (functionData->callback()) {
    LWNODE_CALL_TRACE_ID(TEMPLATE, "> Call JS callback");
    FunctionCallbackInfoWrap info(functionData->isolate(),
                                  thisValue,
                                  thisValue,
                                  newTarget,
                                  VAL(functionData->callbackData()),
                                  argc,
                                  argv);
    functionData->callback()(info);
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

  auto esFunctionTemplate = FunctionTemplateRef::create(
      AtomicStringRef::emptyAtomicString(),  // name
      length,                                // argumentCount
      false,                                 // isStrict
      isConstructor,                         // isConstruction
      FunctionTemplateNativeFunction);       // cb: called whenever the function
                                        // created from this FunctionTemplate is
                                        // called

  // FunctionTemplateNative callback will receive "functionTemplateData"
  // in s as in "var s = new A();"
  auto functionTemplateData = new FunctionTemplateData(
      esFunctionTemplate, isolate, *callback, *data, *signature);

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::New(): New ExtraData: %p",
                           esFunctionTemplate,
                           functionTemplateData);

  ExtraDataHelper::setExtraData(esFunctionTemplate, functionTemplateData);

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
  auto functionTemplateData = ExtraDataHelper::getExtraData(esFunctionTemplate)
                                  ->asFunctionTemplateData();
  functionTemplateData->setCallback(callback);
  functionTemplateData->setCallbackData(*data);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  FunctionTemplateRef* esFunctionTemplate = CVAL(this)->ftpl();
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA, "InstanceTemplate(%p)", esFunctionTemplate);

  auto functionTemplateData = ExtraDataHelper::getExtraData(esFunctionTemplate)
                                  ->asFunctionTemplateData();
  LWNODE_CHECK(functionTemplateData);

  // Only one instanceTemplate should exist.
  auto esObjectTemplate = esFunctionTemplate->instanceTemplate();
  auto objectTemplateData = ExtraDataHelper::getExtraData(esObjectTemplate);
  if (objectTemplateData) {
    // objectTemplateData was set by itself in the below 'else'
    LWNODE_CHECK(objectTemplateData->isObjectTemplateData());
    LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                             "InstanceTemplate(%p): Existing ExtraData: %p",
                             esObjectTemplate,
                             objectTemplateData);
  } else {
    auto objectTemplateData = new ObjectTemplateData(esFunctionTemplate);
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "FunctionTemplate(%p)::InstanceTemplate(%p): New ExtarData: %p",
        esFunctionTemplate,
        esObjectTemplate,
        objectTemplateData);
    ExtraDataHelper::setExtraData(esObjectTemplate, objectTemplateData);
  }

  return Utils::NewLocal(IsolateWrap::GetCurrent()->toV8(), esObjectTemplate);
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

// Returns a unique function instance in the "current execution context"
MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Function>(), TEMPLATE);
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esFunctionTemplate = CVAL(this)->ftpl();

  auto esFunction = esFunctionTemplate->instantiate(esContext);
  auto functionTemplateData = ExtraDataHelper::getExtraData(esFunctionTemplate)
                                  ->asFunctionTemplateData();

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::GetFunction(%p)",
                           esFunctionTemplate,
                           esFunction);

  auto functionData = ExtraDataHelper::getExtraData(esFunction);
  if (functionData) {
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "FunctionTemplate(%p)::GetFunction(%p): Existing functionData: %p",
        esFunctionTemplate,
        esFunction,
        functionData);
    if (functionData->isFunctionTemplateData()) {
      // This functionData was created by FunctionTemplate::New(), and
      // FunctionTemplateData was set in esFunction. We need to replace it with
      // a new FunctionData
      auto newFunctionData = new FunctionData(esFunctionTemplate);
      LWNODE_CALL_TRACE_ID_LOG(
          EXTRADATA,
          "FunctionTemplate(%p)::GetFunction(%p): New functionData: %p",
          esFunctionTemplate,
          esFunction,
          newFunctionData);
      ExtraDataHelper::setExtraData(
          esFunction->asFunctionObject(), newFunctionData, true);
    } else if (functionData->isFunctionData()) {
      // this functionData was created previously by the above line.
      // Use it as it is.
    } else {
      LWNODE_CHECK(false);
    }
  } else {
    LWNODE_CHECK(false);
  }

  return Utils::NewLocal<Function>(lwIsolate->toV8(), esFunction);
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
  LWNODE_RETURN_LOCAL(Object);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  LWNODE_CALL_TRACE();
  auto esSelfFunctionTemplate = CVAL(this)->ftpl();
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::HasInstance(): %p",
                           CVAL(this)->ftpl(),
                           CVAL(*value)->value()->asObject());
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA,
      "FunctionTemplate(%p)::HasInstance: ExtraData: %p, %p",
      esSelfFunctionTemplate,
      ExtraDataHelper::getExtraData(CVAL(this)->ftpl()),
      ExtraDataHelper::getExtraData(CVAL(*value)->value()->asObject()));

  auto esContext = IsolateWrap::GetCurrent()->GetCurrentContext()->get();
  auto esValue = CVAL(*value)->value();
  if (!esValue->isObject()) {
    return false;
  }

  auto esObject = esValue->asObject();
  auto extraData = ExtraDataHelper::getExtraData(esObject);
  if (!extraData) {
    return false;
  }

  FunctionTemplateRef* esOtherFunctionTemplate = nullptr;
  if (extraData->isObjectData()) {
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "FunctionTemplate(%p)::HasInstance: Existing ExtraData: %p",
        esSelfFunctionTemplate,
        extraData->asObjectData());

    if (extraData->asObjectData()->objectTemplate()) {
      auto objectTemplateData = ExtraDataHelper::getExtraData(
                                    extraData->asObjectData()->objectTemplate())
                                    ->asObjectTemplateData();
      esOtherFunctionTemplate = objectTemplateData->functionTemplate();
    }
  } else if (extraData->isObjectTemplateData()) {
    esOtherFunctionTemplate =
        extraData->asObjectTemplateData()->functionTemplate();
  } else {
    LWNODE_CHECK(false);
  }

  for (auto functionTemplate = esOtherFunctionTemplate;
       functionTemplate != nullptr;
       functionTemplate = functionTemplate->parent().value()) {
    if (esSelfFunctionTemplate == functionTemplate) {
      return true;
    }
  }

  return false;
}

// --- O b j e c t T e m p l a t e ---

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  API_ENTER_NO_EXCEPTION(isolate, TEMPLATE);
  auto esObjectTemplate = ObjectTemplateRef::create();

  FunctionTemplateRef* esFunctionTemplate = nullptr;
  if (!constructor.IsEmpty()) {
    esFunctionTemplate = CVAL(*constructor)->ftpl();
  }
  auto objectTemplateData = new ObjectTemplateData(esFunctionTemplate);
  ExtraDataHelper::setExtraData(esObjectTemplate, objectTemplateData);

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "ObjectTemplate()::New(%p), Data: %p\n",
                           esObjectTemplate,
                           objectTemplateData);

  return Utils::NewLocal(isolate, esObjectTemplate);
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
                              ValueRef* propertyName) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.getter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.getter(v8PropertyName, info);

      if (info.hasReturnValue()) {
        return CVAL(*info.GetReturnValue().Get())->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.setter) {
    esHandlerData.setter = [](ExecutionStateRef* state,
                              ObjectRef* esSelf,
                              ValueRef* esReceiver,
                              void* data,
                              ValueRef* propertyName,
                              ValueRef* esValue) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.setter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = v8::Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.setter(
          v8PropertyName, v8::Utils::ToLocal<Value>(esValue), info);

      if (info.hasReturnValue()) {
        if (info.GetReturnValue().Get()->IsFalse()) {
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
    esHandlerData.query =
        [](ExecutionStateRef* state,
           ObjectRef* esSelf,
           ValueRef* esReceiver,
           void* data,
           ValueRef* propertyName) -> TemplatePropertyAttribute {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.query) {
        return TemplatePropertyAttribute::TemplatePropertyAttributeNotExist;
      }

      Local<Name> v8PropertyName = v8::Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<Integer> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.query(v8PropertyName, info);

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
    esHandlerData.deleter =
        [](ExecutionStateRef* state,
           ObjectRef* esSelf,
           ValueRef* esReceiver,
           void* data,
           ValueRef* propertyName) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.deleter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = v8::Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<v8::Boolean> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.deleter(v8PropertyName,
                                                           info);

      if (info.hasReturnValue()) {
        return CVAL(*info.GetReturnValue().Get())->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.enumerator) {
    esHandlerData.enumerator = [](ExecutionStateRef* state,
                                  ObjectRef* esSelf,
                                  ValueRef* esReceiver,
                                  void* data) -> ValueVectorRef* {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.enumerator) {
        return ValueVectorRef::create(0);
      }

      PropertyCallbackInfoWrap<v8::Array> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.enumerator(info);

      if (info.hasReturnValue()) {
        auto esArray =
            CVAL(*info.GetReturnValue().Get())->value()->asArrayObject();
        auto length = esArray->length(state);
        auto vector = ValueVectorRef::create(length);

        for (size_t i = 0; i < length; i++) {
          vector->set(
              i, esArray->get(state, ValueRef::create(i))->toString(state));
        }
        return vector;
      }

      return ValueVectorRef::create(0);
    };
  }

  if (config.definer) {
    esHandlerData.definer = [](ExecutionStateRef* state,
                               ObjectRef* esSelf,
                               ValueRef* esReceiver,
                               void* data,
                               ValueRef* propertyName,
                               const ObjectPropertyDescriptorRef& esDescriptor)
        -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.definer) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = v8::Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      PropertyDescriptor descriptor;
      descriptor.get_private()->setDescriptor(
          const_cast<ObjectPropertyDescriptorRef*>(&esDescriptor));
      handlerConfiguration->m_namedPropertyHandler.definer(
          v8PropertyName, descriptor, info);

      if (info.hasReturnValue()) {
        return Escargot::OptionalRef<Escargot::ValueRef>(
            CVAL(*info.GetReturnValue().Get())->value());
      }
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.descriptor) {
    esHandlerData.descriptor =
        [](ExecutionStateRef* state,
           ObjectRef* esSelf,
           ValueRef* esReceiver,
           void* data,
           ValueRef* propertyName) -> OptionalRef<ValueRef> {
      auto handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);

      if (!handlerConfiguration->m_namedPropertyHandler.descriptor) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = v8::Utils::ToLocal<Name>(propertyName);

      PropertyCallbackInfoWrap<v8::Value> info(
          handlerConfiguration->m_isolate,
          esSelf,
          esReceiver,
          VAL(*handlerConfiguration->m_namedPropertyHandler.data));

      handlerConfiguration->m_namedPropertyHandler.descriptor(v8PropertyName,
                                                              info);

      if (info.hasReturnValue()) {
        return Escargot::OptionalRef<Escargot::ValueRef>(
            CVAL(*info.GetReturnValue().Get())->value());
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

  auto esNewObject = esObjectTemplate->instantiate(esContext);
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "ObjectTemplate(%p)::NewInstance(): %p",
                           esObjectTemplate,
                           esNewObject);

  auto objectTemplateData =
      ExtraDataHelper::getExtraData(esObjectTemplate)->asObjectTemplateData();

  auto objectData = ObjectRefHelper::getExtraData(esNewObject);
  if (objectData) {
    // objectData was created in ObjectTemplate::New(). We need to create an
    // ObjectData instance from its ObjectTemplateData.
    LWNODE_CHECK(objectData->isObjectTemplateData());
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "ObjectTemplate(%p)::NewInstance(): Existing Data: %p",
        esObjectTemplate,
        objectData);
    auto objectData = objectTemplateData->createObjectData(esObjectTemplate);
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "ObjectTemplate(%p)::NewInstance() New objectData: %p",
        esObjectTemplate,
        objectData);
    ObjectRefHelper::setExtraData(esNewObject, objectData, true);
  } else {
    LWNODE_CHECK(false);
  }

  return Utils::NewLocal<Object>(lwIsolate->toV8(), esNewObject);
}
}  // namespace v8
