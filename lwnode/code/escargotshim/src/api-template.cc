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

  EsScopeTemplate scope(this);
  // Name can be either a string or symbol
  auto lwValue = CVAL(*value);
  if (lwValue->type() == HandleWrap::Type::ObjectTemplate ||
      lwValue->type() == HandleWrap::Type::FunctionTemplate) {
    scope.self()->set(scope.asValue(name),
                      lwValue->tpl(),
                      isWritable,
                      isEnumerable,
                      isConfigurable);
  } else {
    scope.self()->set(scope.asValue(name),
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
  EsScopeTemplate scope(this);

  FunctionTemplateRef* esGetter = nullptr;
  if (!getter.IsEmpty()) {
    esGetter = CVAL(*getter)->ftpl();
  }
  FunctionTemplateRef* esSetter = nullptr;
  if (!setter.IsEmpty()) {
    esSetter = CVAL(*setter)->ftpl();
  }

  scope.self()->setAccessorProperty(scope.asValue(name)->asString(),
                                    OptionalRef<FunctionTemplateRef>(esGetter),
                                    OptionalRef<FunctionTemplateRef>(esSetter),
                                    !(attribute & DontEnum),
                                    !(attribute & DontDelete));
}

// --- F u n c t i o n   T e m p l a t e ---

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  EsScopeFunctionTemplate scope(this);

  return Utils::NewLocal(scope.v8Isolate(), scope.self()->prototypeTemplate());
}

void FunctionTemplate::SetPrototypeProviderTemplate(
    Local<FunctionTemplate> prototype_provider) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  EsScopeFunctionTemplate scope(this);

  auto esThatFunctionTemplate = scope.asFunctionTemplate(value);
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::Inherit(): %p",
                           scope.self(),
                           esThatFunctionTemplate);

  scope.self()->inherit(esThatFunctionTemplate);
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA,
      "FunctionTemplate(%p)::Inherit(): ExtraData1: %p, ExtraData2: %p",
      scope.self(),
      ExtraDataHelper::getFunctionTemplateExtraData(scope.self()),
      ExtraDataHelper::getFunctionTemplateExtraData(esThatFunctionTemplate));
}

static FunctionData* getFunctionDataFromCallee(FunctionObjectRef* callee) {
  auto calleeExtraData = ExtraDataHelper::getExtraData(callee);
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

  return functionData;
}

static void setExtraDataToNewObjectInstance(ValueRef* thisValue,
                                            ObjectRef* newTarget) {
  auto targetData = ExtraDataHelper::getFunctionExtraData(newTarget);
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA, "targetData: %p", targetData);

  if (!targetData) {
    return;
  }

  // newTarget was created from JavaScript using Template.
  // targetData was created from FunctionTemplate::GetFunction()
  auto objectTemplate = targetData->functionTemplate()->instanceTemplate();

  auto newInstance = thisValue->asObject();
  auto newInstanceObjectTemplateData =
      ExtraDataHelper::getObjectTemplateExtraData(newInstance);
  ObjectData* newObjectData = nullptr;
  if (newInstanceObjectTemplateData) {
    // 1. newInstanceObjectTemplateData was created from
    // FunctionTemplate::InstanceTemplate()
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA, "Existing Data: %p", newInstanceObjectTemplateData);
    newObjectData =
        newInstanceObjectTemplateData->createObjectData(objectTemplate);
    // Replace ObjectTemplateData with correct ObjectData
    ObjectRefHelper::setExtraData(newInstance, newObjectData, true);
  } else {
    auto objectTemplateData =
        ExtraDataHelper::getObjectTemplateExtraData(objectTemplate);
    if (objectTemplateData) {
      // 2. newInstance was created from ObjectTemplate
      newObjectData = objectTemplateData->createObjectData(objectTemplate);
      ObjectRefHelper::setExtraData(newInstance, newObjectData);
    } else {
      // 3. newInstance was created from FunctionTemplate::GetFunction(),
      // with "var s = new S();" in Javascript
      newObjectData = new ObjectData(targetData->functionTemplate());
      ObjectRefHelper::setExtraData(newInstance, newObjectData);
    }
  }

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA, "New ExtraData: %p", newObjectData);
}

static ValueRef* runNativeFunctionCallback(ExecutionStateRef* state,
                                           FunctionData* functionData,
                                           ValueRef* thisValue,
                                           ObjectRef* newTarget,
                                           size_t argc,
                                           ValueRef** argv) {
  auto lwIsolate = IsolateWrap::GetCurrent();
  if (!functionData->checkSignature(state, thisValue->asObject())) {
    lwIsolate->ScheduleThrow(TypeErrorObjectRef::create(
        state, StringRef::createFromASCII("Illegal invocation")));
    lwIsolate->ThrowErrorIfHasException(state);
    LWNODE_DLOG_ERROR("Signature mismatch!");
    return nullptr;
  }

  Local<Value> result;
  if (functionData->callback()) {
    LWNODE_CALL_TRACE_ID(TEMPLATE, "> Call JS callback");
    lwIsolate->increaseCallDepth();
    FunctionCallbackInfoWrap info(functionData->isolate(),
                                  thisValue,
                                  thisValue,
                                  newTarget,
                                  VAL(functionData->callbackData()),
                                  argc,
                                  argv);
    functionData->callback()(info);
    lwIsolate->decreaseCallDepth();

    lwIsolate->ThrowErrorIfHasException(state);
    result = info.GetReturnValue().Get();
  }

  if (!result.IsEmpty()) {
    return VAL(*result)->value();
  }

  return ValueRef::createUndefined();
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
  LWNODE_CALL_TRACE_ID(TEMPLATE,
                       "es: %p newTarget: %s",
                       thisValue,
                       strBool(newTarget.hasValue()));

  FunctionData* functionData =
      getFunctionDataFromCallee(state->resolveCallee().value());

  if (newTarget.hasValue()) {
    setExtraDataToNewObjectInstance(thisValue, newTarget.value());
  }

  ValueRef* result = runNativeFunctionCallback(
      state, functionData, thisValue, newTarget.value(), argc, argv);
  if (result == nullptr) {
    return ValueRef::createUndefined();
  }

  if (newTarget.hasValue()) {
    return thisValue;
  }

  return result;
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

  EsScopeFunctionTemplate scope(this);
  auto functionTemplateData =
      ExtraDataHelper::getFunctionTemplateExtraData(scope.self());
  functionTemplateData->setCallback(callback);
  functionTemplateData->setCallbackData(*data);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  EsScopeFunctionTemplate scope(this);
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA, "InstanceTemplate(%p)", scope.self());

  auto functionTemplateData =
      ExtraDataHelper::getFunctionTemplateExtraData(scope.self());
  LWNODE_CHECK(functionTemplateData);

  // Only one instanceTemplate should exist.
  auto esObjectTemplate = scope.self()->instanceTemplate();
  auto objectTemplateData =
      ExtraDataHelper::getObjectTemplateExtraData(esObjectTemplate);
  if (objectTemplateData) {
    // objectTemplateData was set by itself in the below 'else'
    LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                             "InstanceTemplate(%p): Existing ExtraData: %p",
                             esObjectTemplate,
                             objectTemplateData);
  } else {
    auto objectTemplateData = new ObjectTemplateData(scope.self());
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "FunctionTemplate(%p)::InstanceTemplate(%p): New ExtarData: %p",
        scope.self(),
        esObjectTemplate,
        objectTemplateData);
    ExtraDataHelper::setExtraData(esObjectTemplate, objectTemplateData);
  }

  return Utils::NewLocal(IsolateWrap::GetCurrent()->toV8(), esObjectTemplate);
}

void FunctionTemplate::SetLength(int length) {
  EsScopeFunctionTemplate scope(this);
  scope.self()->setLength(length);
}

void FunctionTemplate::SetClassName(Local<String> name) {
  EsScopeFunctionTemplate scope(this);
  auto esName = CVAL(*name)->value()->asString();

  auto r = Evaluator::execute(
      scope.context(),
      [](ExecutionStateRef* esState,
         FunctionTemplateRef* esFunctionTemplate,
         StringRef* esName) -> ValueRef* {
        esFunctionTemplate->setName(
            AtomicStringRef::create(esState->context(), esName));
        return ValueRef::createNull();
      },
      scope.self(),
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
  API_ENTER_AND_EXIT_IF_TERMINATING(
      EsScopeFunctionTemplate, context, MaybeLocal<Function>());
  auto esFunction = scope.self()->instantiate(scope.context());
  auto functionTemplateData =
      ExtraDataHelper::getFunctionTemplateExtraData(scope.self());

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::GetFunction(%p)",
                           scope.self(),
                           esFunction);

  auto functionData = ExtraDataHelper::getExtraData(esFunction);
  if (functionData) {
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "FunctionTemplate(%p)::GetFunction(%p): Existing functionData: %p",
        scope.self(),
        esFunction,
        functionData);
    if (functionData->isFunctionTemplateData()) {
      // This functionData was created by FunctionTemplate::New(), and
      // FunctionTemplateData was set in esFunction. We need to replace it with
      // a new FunctionData
      auto newFunctionData = new FunctionData(scope.self());
      LWNODE_CALL_TRACE_ID_LOG(
          EXTRADATA,
          "FunctionTemplate(%p)::GetFunction(%p): New functionData: %p",
          scope.self(),
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

  return Utils::NewLocal<Function>(scope.v8Isolate(), esFunction);
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
  LWNODE_RETURN_LOCAL(Object);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  EsScopeFunctionTemplate scope(this);
  LWNODE_CALL_TRACE();
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "FunctionTemplate(%p)::HasInstance(): %p",
                           scope.self(),
                           scope.asValue(value)->asObject());
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA,
      "FunctionTemplate(%p)::HasInstance: ExtraData: %p, %p",
      scope.self(),
      ExtraDataHelper::getFunctionTemplateExtraData(scope.self()),
      ExtraDataHelper::getExtraData(scope.asValue(value)->asObject()));

  auto esValue = scope.asValue(value);
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
        scope.self(),
        extraData->asObjectData());

    if (extraData->asObjectData()->objectTemplate()) {
      auto objectTemplateData = ExtraDataHelper::getObjectTemplateExtraData(
          extraData->asObjectData()->objectTemplate());
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
    if (scope.self() == functionTemplate) {
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
      v8::Isolate* _isolate,
      const v8::NamedPropertyHandlerConfiguration& namedPropertyHandlers)
      : isolate(_isolate), namedPropertyHandlers(namedPropertyHandlers) {}

  HandlerConfiguration(
      v8::Isolate* _isolate,
      const v8::IndexedPropertyHandlerConfiguration& _indexedPropertyHandlers)
      : isolate(_isolate), indexedPropertyHandlers(_indexedPropertyHandlers) {}
  v8::Isolate* isolate{nullptr};
  union {
    v8::NamedPropertyHandlerConfiguration namedPropertyHandlers;
    v8::IndexedPropertyHandlerConfiguration indexedPropertyHandlers;
  };
};

static_assert(sizeof(v8::NamedPropertyHandlerConfiguration) ==
                  sizeof(v8::IndexedPropertyHandlerConfiguration),
              "PropertyHandlerConfiguration size is different");

template <typename T>
struct ObjectTemplateLocalDataHelper {
  ObjectTemplateLocalDataHelper(ExecutionStateRef* state,
                                ObjectRef* esSelf,
                                ValueRef* esReceiver,
                                ValueRef* esPropertyName,
                                void* data) {
    if (esPropertyName) {
      v8PropertyName = Utils::ToLocal<Name>(esPropertyName);
      indexProperty = esPropertyName->tryToUseAsIndexProperty(state);
    }

    handlerConfiguration = reinterpret_cast<HandlerConfiguration*>(data);
    info = std::unique_ptr<PropertyCallbackInfoWrap<T>>(
        new PropertyCallbackInfoWrap<T>(
            handlerConfiguration->isolate,
            esSelf,
            esReceiver,
            VAL(*handlerConfiguration->namedPropertyHandlers.data)));
  }

  bool isIndexedProperty() {
    return indexProperty != ValueRef::InvalidIndexPropertyValue;
  }

  Local<Name> v8PropertyName;
  uint32_t indexProperty{ValueRef::InvalidIndexPropertyValue};
  HandlerConfiguration* handlerConfiguration = nullptr;
  std::unique_ptr<PropertyCallbackInfoWrap<T>> info;
};

class ObjectTemplatePropertyHandlerCallbackHelper {
 public:
  PropertyHandlerGetterCallback getGetterCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName) -> OptionalRef<ValueRef> {
      ObjectTemplateLocalDataHelper<v8::Value> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.getter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.getter(
            helper.indexProperty, *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.getter(
            helper.v8PropertyName, *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        return CVAL(*helper.info->GetReturnValue().Get())->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  PropertyHandlerSetterCallback getSetterCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName,
              ValueRef* esValue) -> OptionalRef<ValueRef> {
      ObjectTemplateLocalDataHelper<v8::Value> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.setter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.setter(
            helper.indexProperty,
            v8::Utils::ToLocal<Value>(esValue),
            *helper.info);

      } else {
        helper.handlerConfiguration->namedPropertyHandlers.setter(
            helper.v8PropertyName,
            v8::Utils::ToLocal<Value>(esValue),
            *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        if (helper.info->GetReturnValue().Get()->IsFalse()) {
          return Escargot::OptionalRef<Escargot::ValueRef>(
              ValueRef::create(false));
        }
        return Escargot::OptionalRef<Escargot::ValueRef>(
            ValueRef::create(true));
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  PropertyHandlerQueryCallback getQueryCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName) -> ObjectTemplatePropertyAttribute {
      ObjectTemplateLocalDataHelper<v8::Integer> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.query) {
        return ObjectTemplatePropertyAttribute::PropertyAttributeNotExist;
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.query(
            helper.indexProperty, *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.query(
            helper.v8PropertyName, *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        bool hasNone =
            (helper.handlerConfiguration->namedPropertyHandlers.flags ==
             PropertyHandlerFlags::kNone);
        bool hasNoSideEffect =
            (static_cast<int>(
                 helper.handlerConfiguration->namedPropertyHandlers.flags) &
             static_cast<int>(PropertyHandlerFlags::kHasNoSideEffect));

        if (hasNone) {
          return ObjectTemplatePropertyAttribute::PropertyAttributeExist;
        } else if (hasNoSideEffect) {
          return ObjectTemplatePropertyAttribute::PropertyAttributeEnumerable;
        } else {
          LWNODE_UNIMPLEMENT;
        }
        return ObjectTemplatePropertyAttribute::PropertyAttributeExist;
      }

      return ObjectTemplatePropertyAttribute::PropertyAttributeNotExist;
    };
  }

  PropertyHandlerDeleteCallback getDeleteCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName) -> OptionalRef<ValueRef> {
      ObjectTemplateLocalDataHelper<v8::Boolean> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.deleter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.deleter(
            helper.indexProperty, *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.deleter(
            helper.v8PropertyName, *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        return CVAL(*helper.info->GetReturnValue().Get())->value();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  PropertyHandlerEnumerationCallback getEnumerationCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data) -> ValueVectorRef* {
      ObjectTemplateLocalDataHelper<v8::Array> helper(
          state, esSelf, esReceiver, nullptr, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.enumerator) {
        return ValueVectorRef::create(0);
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.enumerator(
            *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.enumerator(
            *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        auto esArray = CVAL(*helper.info->GetReturnValue().Get())
                           ->value()
                           ->asArrayObject();
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

  PropertyHandlerDefineOwnPropertyCallback getDefineOwnPropertyCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName,
              const ObjectPropertyDescriptorRef& esDescriptor)
               -> OptionalRef<ValueRef> {
      ObjectTemplateLocalDataHelper<v8::Value> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.definer) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      PropertyDescriptor descriptor;
      descriptor.get_private()->setDescriptor(
          const_cast<ObjectPropertyDescriptorRef*>(&esDescriptor));

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.definer(
            helper.indexProperty, descriptor, *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.definer(
            helper.v8PropertyName, descriptor, *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        return Escargot::OptionalRef<Escargot::ValueRef>(
            CVAL(*helper.info->GetReturnValue().Get())->value());
      }
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  PropertyHandlerGetPropertyDescriptorCallback
  getGetPropertyDescriptorCallback() {
    return [](ExecutionStateRef* state,
              ObjectRef* esSelf,
              ValueRef* esReceiver,
              void* data,
              ValueRef* propertyName) -> OptionalRef<ValueRef> {
      ObjectTemplateLocalDataHelper<v8::Value> helper(
          state, esSelf, esReceiver, propertyName, data);

      if (!helper.handlerConfiguration->namedPropertyHandlers.descriptor) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      if (helper.isIndexedProperty()) {
        helper.handlerConfiguration->indexedPropertyHandlers.descriptor(
            helper.indexProperty, *helper.info);
      } else {
        helper.handlerConfiguration->namedPropertyHandlers.descriptor(
            helper.v8PropertyName, *helper.info);
      }

      if (helper.info->hasReturnValue()) {
        return Escargot::OptionalRef<Escargot::ValueRef>(
            CVAL(*helper.info->GetReturnValue().Get())->value());
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }
};

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  EsScopeObjectTemplate scope(this);

  ObjectTemplatePropertyHandlerConfiguration esHandlerData;

  ObjectTemplatePropertyHandlerCallbackHelper helper;
  esHandlerData.getter = config.getter ? helper.getGetterCallback() : nullptr;
  esHandlerData.setter = config.setter ? helper.getSetterCallback() : nullptr;
  esHandlerData.query = config.query ? helper.getQueryCallback() : nullptr;
  esHandlerData.deleter = config.deleter ? helper.getDeleteCallback() : nullptr;
  esHandlerData.enumerator =
      config.enumerator ? helper.getEnumerationCallback() : nullptr;
  esHandlerData.definer =
      config.definer ? helper.getDefineOwnPropertyCallback() : nullptr;
  esHandlerData.descriptor =
      config.descriptor ? helper.getGetPropertyDescriptorCallback() : nullptr;

  esHandlerData.data =
      new HandlerConfiguration(IsolateWrap::GetCurrent()->toV8(), config);

  scope.self()->setNamedPropertyHandler(esHandlerData);
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
  EsScopeObjectTemplate scope(this);

  ObjectTemplatePropertyHandlerConfiguration esConfig;

  ObjectTemplatePropertyHandlerCallbackHelper helper;
  esConfig.getter = config.getter ? helper.getGetterCallback() : nullptr;
  esConfig.setter = config.setter ? helper.getSetterCallback() : nullptr;
  esConfig.query = config.query ? helper.getQueryCallback() : nullptr;
  esConfig.deleter = config.deleter ? helper.getDeleteCallback() : nullptr;
  esConfig.enumerator =
      config.enumerator ? helper.getEnumerationCallback() : nullptr;
  esConfig.definer =
      config.definer ? helper.getDefineOwnPropertyCallback() : nullptr;
  esConfig.descriptor =
      config.descriptor ? helper.getGetPropertyDescriptorCallback() : nullptr;

  esConfig.data =
      new HandlerConfiguration(IsolateWrap::GetCurrent()->toV8(), config);

  scope.self()->setIndexedPropertyHandler(esConfig);
}

void ObjectTemplate::SetCallAsFunctionHandler(FunctionCallback callback,
                                              Local<Value> data) {
  LWNODE_RETURN_VOID;
}

int ObjectTemplate::InternalFieldCount() {
  EsScopeObjectTemplate scope(this);
  return ObjectTemplateRefHelper::getInternalFieldCount(scope.self());
}

void ObjectTemplate::SetInternalFieldCount(int value) {
  EsScopeObjectTemplate scope(this);

  if (scope.self()->didInstantiate()) {
    LWNODE_DLOG_WARN(
        "Don't modify internal field count after instantiating object");
    return;
  }
  ObjectTemplateRefHelper::setInternalFieldCount(scope.self(), value);
}

bool ObjectTemplate::IsImmutableProto() {
  LWNODE_RETURN_FALSE;
}

void ObjectTemplate::SetImmutableProto() {
  LWNODE_RETURN_VOID;
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  API_ENTER_AND_EXIT_IF_TERMINATING(
      EsScopeObjectTemplate, context, MaybeLocal<v8::Object>());

  auto esNewObject = scope.self()->instantiate(scope.context());
  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "ObjectTemplate(%p)::NewInstance(): %p",
                           scope.self(),
                           esNewObject);

  auto objectTemplateData =
      ExtraDataHelper::getObjectTemplateExtraData(scope.self());

  auto objectData = ObjectRefHelper::getExtraData(esNewObject);
  if (objectData) {
    // objectData was created in ObjectTemplate::New(). We need to create an
    // ObjectData instance from its ObjectTemplateData.
    LWNODE_CHECK(objectData->isObjectTemplateData());
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "ObjectTemplate(%p)::NewInstance(): Existing Data: %p",
        scope.self(),
        objectData);
    auto objectData = objectTemplateData->createObjectData(scope.self());
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "ObjectTemplate(%p)::NewInstance() New objectData: %p",
        scope.self(),
        objectData);
    ObjectRefHelper::setExtraData(esNewObject, objectData, true);
  } else {
    LWNODE_CHECK(false);
  }

  return Utils::NewLocal<Object>(scope.v8Isolate(), esNewObject);
}
}  // namespace v8
