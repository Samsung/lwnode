/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "jsutils.h"
#include "v8.h"
#include "v8utils.h"

using namespace EscargotShim;

namespace v8 {

enum AccessorType { Setter = 0, Getter = 1 };

static inline JsObjectRef ToJsObjectRef(Object* object) {
  return object->asJsValueRef()->asObject();
}
typedef struct AcessorExternalDataType : public gc {
  Persistent<Value> data;
  Persistent<AccessorSignature> signature;
  AccessorType type;
  union {
    AccessorNameGetterCallback getter;
    AccessorNameSetterCallback setter;
  };
  Persistent<Name> propertyName;

  bool CheckSignature(Local<Object> thisPointer, Local<Object>* holder) {
    if (signature.IsEmpty()) {
      *holder = thisPointer;
      return true;
    }

    Local<FunctionTemplate> receiver = signature.As<FunctionTemplate>();
    return Utils::CheckSignature(receiver, thisPointer, holder);
  }
} AccessorExternalData;

struct InternalFieldDataStruct {
  void** internalFieldPointers;
  int size;
};

Maybe<bool> Object::Set(Local<Context> context,
                        Local<Value> key,
                        Local<Value> value) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef self = asJsValueRef()->asObject();

  bool result = false;
  auto error = SetProperty(
      contextRef, self, key->asJsValueRef(), value->asJsValueRef(), result);
  if (error != JsNoError) {
    return Nothing<bool>();
  }
  return Just(true);
}

// CHAKRA-TODO: Convert this function to javascript
bool Object::Set(Handle<Value> key, Handle<Value> value) {
  return FromMaybe(Set(Local<Context>(), key, value));
}

Maybe<bool> Utils::Set(Handle<Context> context,
                       Handle<Object> object,
                       Handle<Value> key,
                       Handle<Value> value,
                       PropertyAttribute attribs,
                       bool force) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsValueRef keyRef = key->asJsValueRef();
  JsValueRef valueRef = value->asJsValueRef();
  JsObjectRef objectRef = object->asJsValueRef()->asObject();
  // Do it faster if there are no property attributes
  if (!force && attribs == None) {
    bool result = false;
    if (SetProperty(contextRef, objectRef, keyRef, valueRef, result) !=
        JsNoError) {
      return Nothing<bool>();
    }
    if (result == false) {
      return Just(false);
    }
  } else {  // we have attributes just use it
    bool writable = false;
    if ((attribs & ReadOnly) == 0) {
      writable = true;
    }

    bool enumerable = false;
    if ((attribs & DontEnum) == 0) {
      enumerable = true;
    }

    bool configurable = false;
    if ((attribs & DontDelete) == 0) {
      configurable = true;
    }

    bool result = false;
    DefineDataProperty(contextRef,
                       objectRef,
                       keyRef,
                       writable,
                       enumerable,
                       configurable,
                       valueRef,
                       JS_INVALID_REFERENCE,
                       JS_INVALID_REFERENCE,
                       JS_INVALID_REFERENCE,
                       result);
    return Just(result);
  }
  return Just(true);
}

Maybe<bool> Object::Set(Handle<Value> key,
                        Handle<Value> value,
                        PropertyAttribute attribs,
                        bool force) {
  JsContextRef context =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  return Utils::Set(context, ToJsObjectRef(this), key, value, attribs, force);
}

Maybe<bool> Object::Set(Local<Context> context,
                        uint32_t index,
                        Local<Value> value) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef objectRef = ToJsObjectRef(this);

  bool result = false;
  if (EscargotShim::SetProperty(contextRef,
                                objectRef,
                                CreateJsValue(index),
                                value->asJsValueRef(),
                                result) != JsNoError) {
    return Nothing<bool>();
  };

  return Just(result);
}

Maybe<bool> Object::DefineOwnProperty(Local<Context> context,
                                      Local<Name> key,
                                      Local<Value> value,
                                      PropertyAttribute attributes) {
  return Utils::Set(context,
                    ToJsObjectRef(this),
                    key,
                    value,
                    attributes,
                    /*force*/ true);
}

Maybe<bool> Object::DefineProperty(Local<v8::Context> context,
                                   Local<Name> key,
                                   PropertyDescriptor& descriptor) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef self = asJsValueRef()->asObject();
  JsValueRef propertyName = key->asJsValueRef();
  JsErrorCode error = JsNoError;

  JsValueRef value = descriptor.has_value() ? descriptor.value()->asJsValueRef()
                                            : JS_INVALID_REFERENCE;
  JsValueRef getter = descriptor.has_get() ? descriptor.get()->asJsValueRef()
                                           : JS_INVALID_REFERENCE;
  JsValueRef setter = descriptor.has_set() ? descriptor.set()->asJsValueRef()
                                           : JS_INVALID_REFERENCE;

  if (value == JS_INVALID_REFERENCE && getter == JS_INVALID_REFERENCE &&
      setter == JS_INVALID_REFERENCE) {
    bool hasProperty = false;
    error = EscargotShim::HasOwnProperty(
        contextRef, self, propertyName, hasProperty);
    if (error != JsNoError) {
      return Nothing<bool>();
    }

    if (!descriptor.has_enumerable() && !descriptor.has_configurable() &&
        !descriptor.has_writable() && hasProperty) {
      return Just(true);
    }
    error = GetProperty(contextRef, self, propertyName, value);
    if (error != JsNoError) {
      return Nothing<bool>();
    }
  }

  bool isEnumerable =
      descriptor.has_enumerable() ? descriptor.enumerable() : false;
  bool isConfigurable =
      descriptor.has_configurable() ? descriptor.configurable() : false;
  bool isWritable = descriptor.has_writable() ? descriptor.writable() : false;

  bool result = false;
  error = DefineDataProperty(contextRef,
                             self,
                             propertyName,
                             isWritable,
                             isEnumerable,
                             isConfigurable,
                             value,
                             nullptr,
                             getter,
                             setter,
                             result);

  if (error != JsNoError) {
    return Nothing<bool>();
  }
  return Just(result);
}

bool Object::Set(uint32_t index, Handle<Value> value) {
  return FromMaybe(Set(Local<Context>(), index, value));
}

Maybe<bool> Object::ForceSet(Local<Context> context,
                             Local<Value> key,
                             Local<Value> value,
                             PropertyAttribute attribs) {
  return Utils::Set(context,
                    ToJsObjectRef(this),
                    key,
                    value,
                    attribs,
                    /*force*/ true);
}

bool Object::ForceSet(Handle<Value> key,
                      Handle<Value> value,
                      PropertyAttribute attribs) {
  EscargotShim::ContextShim* context =
      IsolateShim::GetCurrent()->currentContext();
  return FromMaybe(ForceSet(context->asContext(), key, value, attribs));
}

MaybeLocal<Value> Object::Get(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsValueRef keyRef = key->asJsValueRef();
  JsObjectRef objectRef = asJsValueRef()->asObject();
  JsValueRef valueRef = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, objectRef, keyRef, valueRef) != JsNoError) {
    return Local<Value>();
  }

  return Local<Value>::New(valueRef);
}

Local<Value> Object::Get(Handle<Value> key) {
  return FromMaybe(Get(Local<Context>(), key));
}

MaybeLocal<Value> Object::Get(Local<Context> context, uint32_t index) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsObjectRef objectRef = ToJsObjectRef(this);
  JsValueRef valueRef = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, objectRef, CreateJsValue(index), valueRef) !=
      JsNoError) {
    return Local<Value>();
  }

  return Local<Value>::New(valueRef);
}

Local<Value> Object::Get(uint32_t index) {
  return FromMaybe(Get(Local<Context>(), index));
}

Maybe<PropertyAttribute> Object::GetPropertyAttributes(Local<Context> context,
                                                       Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  PropertyAttribute attribute = PropertyAttribute::None;
  JsObjectRef self = ToJsObjectRef(this);
  JsValueRef descriptor = JS_INVALID_REFERENCE;
  JsErrorCode error = EscargotShim::GetOwnPropertyDescriptor(
      contextRef, self, key->asJsValueRef(), descriptor);
  if (error != JsNoError) {
    return Nothing<PropertyAttribute>();
  }

  if (descriptor->isUndefined()) {
    return Just(static_cast<PropertyAttribute>(attribute));
  }

  auto descriptorObject = descriptor->asObject();
  JsValueRef writable = JS_INVALID_REFERENCE;
  error =
      EscargotShim::GetOwnProperty(contextRef,
                                   descriptorObject,
                                   GetCachedJsValue(CachedStringId::writable),
                                   writable);
  if (error != JsNoError) {
    return Nothing<PropertyAttribute>();
  }
  if (!writable->asBoolean()) {
    attribute = (PropertyAttribute)(attribute | PropertyAttribute::ReadOnly);
  }

  JsValueRef enumerable = JS_INVALID_REFERENCE;
  error =
      EscargotShim::GetOwnProperty(contextRef,
                                   descriptorObject,
                                   GetCachedJsValue(CachedStringId::enumerable),
                                   enumerable);
  if (error != JsNoError) {
    return Nothing<PropertyAttribute>();
  }
  if (!enumerable->asBoolean()) {
    attribute = (PropertyAttribute)(attribute | PropertyAttribute::DontEnum);
  }

  JsValueRef configurable = JS_INVALID_REFERENCE;
  error = EscargotShim::GetOwnProperty(
      contextRef,
      descriptorObject,
      GetCachedJsValue(CachedStringId::configurable),
      configurable);
  if (error != JsNoError) {
    return Nothing<PropertyAttribute>();
  }
  if (!configurable->asBoolean()) {
    attribute = (PropertyAttribute)(attribute | PropertyAttribute::DontDelete);
  }

  return Just(static_cast<PropertyAttribute>(attribute));
}

PropertyAttribute Object::GetPropertyAttributes(Handle<Value> key) {
  return FromMaybe(GetPropertyAttributes(Local<Context>(), key));
}

MaybeLocal<Value> Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                   Local<Name> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsValueRef result;
  if (EscargotShim::GetOwnPropertyDescriptor(contextRef,
                                             asJsValueRef()->asObject(),
                                             key->asJsValueRef(),
                                             result) != JsNoError) {
    return Local<Value>();
  }
  return Local<Value>::New(result);
}

Local<Value> Object::GetOwnPropertyDescriptor(Local<Name> key) {
  return FromMaybe(GetOwnPropertyDescriptor(Local<Context>(), key));
}

Maybe<bool> Object::Has(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef objectRef = ToJsObjectRef(this);
  JsValueRef keyRef = key->asJsValueRef();
  auto evalResult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state,
                    JsObjectRef objectRef,
                    JsValueRef keyRef) -> JsValueRef {
                   bool result = objectRef->hasOwnProperty(state, keyRef);
                   if (!result) {
                     auto ref = objectRef->getPrototypeObject(state);
                     while (ref.hasValue()) {
                       if (ref.get()->hasOwnProperty(state, keyRef)) {
                         result = true;
                         return CreateJsValue(result);
                       }
                       ref = ref.get()->getPrototypeObject(state);
                     }
                     result = false;
                   }
                   return CreateJsValue(result);
                 },
                 objectRef,
                 keyRef);
  VERIFY_EVAL_RESULT(evalResult, Nothing<bool>());
  return Just(evalResult.result->asBoolean());
}

bool Object::Has(Handle<Value> key) {
  return FromMaybe(Has(Local<Context>(), key));
}

Maybe<bool> Object::Delete(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef objectRef = ToJsObjectRef(this);
  JsValueRef keyRef = key->asJsValueRef();
  auto evalResult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state,
                    JsObjectRef objectRef,
                    JsValueRef keyRef) -> JsValueRef {
                   bool result = objectRef->deleteOwnProperty(state, keyRef);
                   if (!result) {
                     auto ref = objectRef->getPrototypeObject(state);
                     while (ref.hasValue()) {
                       if (ref.get()->deleteOwnProperty(state, keyRef)) {
                         return CreateJsValue(true);
                       }
                       ref = ref.get()->getPrototypeObject(state);
                     }
                     result = false;
                   }
                   return CreateJsValue(result);
                 },
                 objectRef,
                 keyRef);
  VERIFY_EVAL_RESULT(evalResult, Nothing<bool>());
  return Just(evalResult.result->asBoolean());
}

bool Object::Delete(Handle<Value> key) {
  return FromMaybe(Delete(Local<Context>(), key));
}

Maybe<bool> Object::Has(Local<Context> context, uint32_t index) {
  return Has(context, Local<Value>::New(CreateJsValue(index)));
}

bool Object::Has(uint32_t index) {
  return FromMaybe(Has(Local<Context>(), index));
}

Maybe<bool> Object::Delete(Local<Context> context, uint32_t index) {
  return Delete(context, CreateJsValue(index));
}

bool Object::Delete(uint32_t index) {
  return FromMaybe(Delete(Local<Context>(), index));
}

// void CHAKRA_CALLBACK AcessorExternalObjectFinalizeCallback(void *data) {
//   if (data != nullptr) {
//     AccessorExternalData *accessorData =
//       static_cast<AccessorExternalData*>(data);
//     delete accessorData;
//   }
// }

JsValueRef Utils::AccessorNativeFunctionCallback(JsExecutionStateRef state,
                                                 JsValueRef thisValue,
                                                 size_t argc,
                                                 JsValueRef* argv,
                                                 bool isNewExpression) {
  auto callee = state->resolveCallee();
  NESCARGOT_ASSERT(callee);
  auto self = callee.get()->asObject();
  NESCARGOT_ASSERT(self);
  auto* accessorData = static_cast<AccessorExternalData*>(self->extraData());
  NESCARGOT_ASSERT(accessorData);

  JsValueRef result = JS_INVALID_REFERENCE;

  Local<Object> thisLocal = CastTo<Object*>(thisValue);
  Local<Value> dataLocal = accessorData->data;
  Local<Object> holder;
  if (!accessorData->CheckSignature(thisLocal, &holder)) {
    return JS_INVALID_REFERENCE;
  }

  Local<Name> propertyNameLocal = accessorData->propertyName;

  if (accessorData->type == Getter) {
    PropertyCallbackInfo<Value> info(dataLocal, thisLocal, holder);
    accessorData->getter(propertyNameLocal, info);
    result = info.GetReturnValue().Get()->asJsValueRef();
  } else if (accessorData->type == Setter) {
    PropertyCallbackInfo<void> info(dataLocal, thisLocal, holder);
    JsValueRef value = argc > 0 ? argv[0] : JsUndefined();
    accessorData->setter(propertyNameLocal, CastTo<Value*>(value), info);
    result = JsUndefined();
  }

  return result;
}

Maybe<bool> Utils::SetAccessor(Handle<Context> context,
                               Handle<Object> object,
                               Handle<Name> name,
                               AccessorNameGetterCallback getter,
                               AccessorNameSetterCallback setter,
                               v8::Handle<Value> data,
                               AccessControl settings,
                               PropertyAttribute attributes,
                               Handle<AccessorSignature> signature) {
  JsValueRef getterRef = JS_INVALID_REFERENCE;
  JsValueRef setterRef = JS_INVALID_REFERENCE;
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  Handle<Name> nameHandle =
      Handle<Name>(GetCachedJsValueStringIfExist(name->asJsValueRef()));

  JsObjectRef self = object->asJsValueRef()->asObject();

  if (getter) {
    NativeFunctionInfo nativeFunctionInfo(CreateJsEmptyAtomicString(),
                                          Utils::AccessorNativeFunctionCallback,
                                          0,
                                          true,
                                          false);

    AccessorExternalData* externalData = new AccessorExternalData();
    externalData->type = Getter;
    externalData->propertyName = nameHandle;
    externalData->getter = getter;
    externalData->data = data;
    externalData->signature = signature;

    JsFunctionRef functionObjectRef = JS_INVALID_REFERENCE;
    if (CreateJsFunction(contextRef, nativeFunctionInfo, functionObjectRef) !=
        JsNoError) {
      return Just(false);
    }
    SetExtraData(functionObjectRef, externalData);

    getterRef = functionObjectRef;
  } else {
    getterRef = CreateJsNull();
  }

  if (setter) {
    NativeFunctionInfo nativeFunctionInfo(CreateJsEmptyAtomicString(),
                                          Utils::AccessorNativeFunctionCallback,
                                          0,
                                          true,
                                          false);

    AccessorExternalData* externalData = new AccessorExternalData();
    externalData->type = Setter;
    externalData->propertyName = nameHandle;
    externalData->setter = setter;
    externalData->data = data;
    externalData->signature = signature;

    JsFunctionRef functionObjectRef = JS_INVALID_REFERENCE;
    if (CreateJsFunction(contextRef, nativeFunctionInfo, functionObjectRef) !=
        JsNoError) {
      return Just(false);
    }
    SetExtraData(functionObjectRef, externalData);

    setterRef = functionObjectRef;
  } else {
    setterRef = CreateJsUndefined();
  }

  bool result = false;
  if (DefineDataProperty(contextRef,
                         self,
                         nameHandle->asJsValueRef(),
                         false,
                         !(attributes & DontEnum),
                         !(attributes & DontDelete),
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         getterRef,
                         setterRef,
                         result) != JsNoError) {
    return Nothing<bool>();
  };

  return Just(result);
}

Maybe<bool> Object::SetAccessor(Handle<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                v8::Handle<Value> data,
                                AccessControl settings,
                                PropertyAttribute attributes,
                                Handle<AccessorSignature> signature) {
  return Utils::SetAccessor(Local<Context>(),
                            this,
                            name,
                            getter,
                            setter,
                            data,
                            settings,
                            attributes,
                            signature);
}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data,
                                AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type) {
  return Utils::SetAccessor(context,
                            this,
                            name,
                            getter,
                            setter,
                            FromMaybe(data),
                            settings,
                            attribute,
                            Local<AccessorSignature>());
}

bool Object::SetAccessor(Handle<String> name,
                         AccessorGetterCallback getter,
                         AccessorSetterCallback setter,
                         v8::Handle<Value> data,
                         AccessControl settings,
                         PropertyAttribute attributes) {
  return FromMaybe(
      Utils::SetAccessor(Local<Context>(),
                         this,
                         name,
                         reinterpret_cast<AccessorNameGetterCallback>(getter),
                         reinterpret_cast<AccessorNameSetterCallback>(setter),
                         data,
                         settings,
                         attributes,
                         Handle<AccessorSignature>()));
}

bool Object::SetAccessor(Handle<Name> name,
                         AccessorNameGetterCallback getter,
                         AccessorNameSetterCallback setter,
                         Handle<Value> data,
                         AccessControl settings,
                         PropertyAttribute attribute) {
  return FromMaybe(SetAccessor(
      Local<Context>(), name, getter, setter, data, settings, attribute));
}

MaybeLocal<Array> Object::GetPropertyNames(Local<Context> context,
                                           KeyCollectionMode mode,
                                           PropertyFilter propertyFilter,
                                           IndexFilter indexFilter,
                                           KeyConversionMode keyConversion) {
  JsObjectRef self = asJsValueRef()->asObject();
  JsContextRef contextRef = ContextShim::ToContextShim(*context)->contextRef();
  JsValueRef result = JS_INVALID_REFERENCE;

  if (EscargotShim::GetPropertyNames(contextRef,
                                     self,
                                     mode,
                                     propertyFilter,
                                     indexFilter,
                                     keyConversion,
                                     result) != JsNoError) {
    return Local<Array>();
  }

  return Local<Array>::New(result);
}

MaybeLocal<Array> Object::GetPropertyNames(Local<Context> context) {
  return GetPropertyNames(
      context,
      v8::KeyCollectionMode::kIncludePrototypes,
      static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS),
      v8::IndexFilter::kIncludeIndices);
}

MaybeLocal<Array> Object::GetOwnPropertyNames(Local<Context> context) {
  return GetPropertyNames(
      context,
      v8::KeyCollectionMode::kOwnOnly,
      static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS),
      v8::IndexFilter::kIncludeIndices);
}

Local<Value> Object::GetPrototype() {
  JsValueRef result;
  if (EscargotShim::GetPrototype(GetCurrentJsContextRef(),
                                 asJsValueRef()->asObject(),
                                 result) != JsNoError) {
    return Local<Value>();
  }

  if (!result->asObject()->extraData()) {
    result->asObject()->setExtraData(asJsValueRef()->asObject()->extraData());
  }

  return Local<Value>::New(result);
}

Maybe<bool> Object::SetPrototype(Local<Context> context,
                                 Local<Value> prototype) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef objectRef = ToJsObjectRef(this);

  bool result = false;
  if (EscargotShim::SetPrototype(
          contextRef, objectRef, prototype->asJsValueRef(), result) !=
      JsNoError) {
    Nothing<bool>();
  }

  return Just(result);
}

bool Object::SetPrototype(Handle<Value> prototype) {
  return FromMaybe(SetPrototype(Local<Context>(), prototype));
}

MaybeLocal<String> Object::ObjectProtoToString(Local<Context> context) {
  // ContextShim* contextShim = ContextShim::GetCurrent();
  // JsValueRef toString = contextShim->GetToStringFunction();

  // JsValueRef result;
  // JsValueRef args[] = { this };
  // if (JsCallFunction(toString, args, _countof(args), &result) != JsNoError) {
  //   return Local<String>();
  // }
  // return Local<String>::New(result);
  NESCARGOT_UNIMPLEMENTED("");
  return Local<String>();
}

Local<String> Object::ObjectProtoToString() {
  return FromMaybe(ObjectProtoToString(Local<Context>()));
}

Local<String> v8::Object::GetConstructorName() {
  JsObjectRef self = asJsValueRef()->asObject();
  JsContextRef context = GetCurrentJsContextRef();

  JsValueRef constructor = nullptr;
  if (GetObjectConstructor(context, self, constructor) != JsNoError) {
    return Local<String>();
  }

  if (constructor->isUndefined()) {
    return Local<String>::New(IsolateShim::GetCurrent()->asIsolate(),
                              GetCachedJsValue(CachedStringId::object));
  }

  NESCARGOT_ASSERT(constructor->isObject());
  JsValueRef name = nullptr;
  if (GetOwnProperty(context,
                     constructor->asObject(),
                     GetCachedJsValue(CachedStringId::name),
                     name) != JsNoError) {
    return Local<String>();
  }

  return Local<String>::New(IsolateShim::GetCurrent()->asIsolate(), name);
}

Maybe<bool> Object::HasOwnProperty(Local<Context> context, Local<Name> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  JsObjectRef objectRef = ToJsObjectRef(this);

  bool result = false;
  if (EscargotShim::HasOwnProperty(
          contextRef, objectRef, key->asJsValueRef(), result) != JsNoError) {
    return Nothing<bool>();
  }

  return Just(result);
}

bool Object::HasOwnProperty(Handle<String> key) {
  return FromMaybe(HasOwnProperty(Local<Context>(), key));
}

Maybe<bool> Object::HasRealNamedProperty(Local<Context> context,
                                         Local<Name> key) {
  return Has(context, key);
}

bool Object::HasRealNamedProperty(Handle<String> key) {
  return FromMaybe(HasRealNamedProperty(Local<Context>(), key));
}

Maybe<bool> Object::HasRealIndexedProperty(Local<Context> context,
                                           uint32_t index) {
  return Has(context, index);
}

bool Object::HasRealIndexedProperty(uint32_t index) {
  return FromMaybe(HasRealIndexedProperty(Local<Context>(), index));
}

MaybeLocal<Value> Object::GetRealNamedProperty(Local<Context> context,
                                               Local<Name> key) {
  // CHAKRA-TODO: how to skip interceptors?
  // return Get(context, key);
  NESCARGOT_UNIMPLEMENTED("GetRealNamedProperty");
  return Get(context, key);
}

Local<Value> Object::GetRealNamedProperty(Handle<String> key) {
  return FromMaybe(GetRealNamedProperty(Local<Context>(), key));
}

Maybe<PropertyAttribute> Object::GetRealNamedPropertyAttributes(
    Local<Context> context, Local<Name> key) {
  // CHAKRA-TODO: This walks prototype chain skipping interceptors
  // return GetPropertyAttributes(context, key);
  NESCARGOT_UNIMPLEMENTED("GetRealNamedPropertyAttributes");
  return GetPropertyAttributes(context, key);
}

// JsValueRef CHAKRA_CALLBACK Utils::AccessorHandler(
//     JsValueRef callee,
//     bool isConstructCall,
//     JsValueRef *arguments,
//     unsigned short argumentCount,  // NOLINT(runtime/int)
//     void *callbackState) {
//   void *externalData;
//   JsValueRef result = JS_INVALID_REFERENCE;

//   if (JsGetUndefinedValue(&result) != JsNoError) {
//     return JS_INVALID_REFERENCE;
//   }

//   if (jsrt::GetExternalData(callee, &externalData) != JsNoError) {
//     return result;
//   }

//   if (externalData == nullptr) {
//     return result;
//   }

//   AccessorExternalData *accessorData =
//     static_cast<AccessorExternalData*>(externalData);
//   Local<Value> dataLocal = accessorData->data;

//   JsValueRef thisRef = JS_INVALID_REFERENCE;
//   if (argumentCount > 0) {
//     thisRef = arguments[0];
//   }
//   // this is ok since the first argument will stay on the stack as long as we
//   // are in this function
//   Local<Object> thisLocal(static_cast<Object*>(thisRef));

//   Local<Object> holder;
//   if (!accessorData->CheckSignature(thisLocal, &holder)) {
//     return JS_INVALID_REFERENCE;
//   }

//   Local<Name> propertyNameLocal = accessorData->propertyName;
//   switch (accessorData->type) {
//     case Setter:
//     {
//       CHAKRA_VERIFY(argumentCount == 2);
//       PropertyCallbackInfo<void> info(dataLocal, thisLocal, holder);
//       accessorData->setter(
//           propertyNameLocal, static_cast<Value*>(arguments[1]), info);
//       break;
//     }
//     case Getter:
//     {
//       PropertyCallbackInfo<Value> info(dataLocal, thisLocal, holder);
//       accessorData->getter(propertyNameLocal, info);
//       result = info.GetReturnValue().Get();
//       break;
//     }
//      default:
//       break;
//   }

//   return result;
// }

Maybe<bool> Object::HasPrivate(Local<Context> context, Local<Private> key) {
  return Just(EscargotShim::HasPrivate(
      ContextShim::ToContextShim(*context)->contextRef(),
      asJsValueRef()->asObject(),
      CastTo<JsValueRef>(*key)));
}

// Maybe<bool> Object::DeletePrivate(Local<Context> context, Local<Private> key)
// {
//   return Just(jsrt::DeletePrivate((JsValueRef)this, (JsValueRef)*key));
// }

// Local<Value> Object::GetHiddenValue(Handle<String> key) {
//   JsValueRef result;
//   if (jsrt::GetPrivate((JsValueRef)this, (JsValueRef)*key, &result)
//       != JsNoError) {
//     result = jsrt::GetUndefined();
//   }

//   return Local<Value>::New(static_cast<Value*>(result));
// }

MaybeLocal<Value> Object::GetPrivate(Local<Context> context,
                                     Local<Private> key) {
  JsValueRef result;

  if (EscargotShim::GetPrivate(
          ContextShim::ToContextShim(*context)->contextRef(),
          asJsValueRef()->asObject(),
          CastTo<JsValueRef>(*key),
          result) != JsNoError) {
    result = JsUndefined();
  }

  return Local<Value>::New(result);
}

// bool Object::SetHiddenValue(Handle<String> key, Handle<Value> value) {
//   if (jsrt::SetPrivate((JsValueRef)this, (JsValueRef)*key,
//                            (JsValueRef)*value) != JsNoError) {
//     return false;
//   }

//   return true;
// }

Maybe<bool> Object::SetPrivate(Local<Context> context,
                               Local<Private> key,
                               Local<Value> value) {
  auto contextRef = ContextShim::ToContextShim(*context)->contextRef();

  if (EscargotShim::SetPrivate(contextRef,
                               asJsValueRef()->asObject(),
                               CastTo<JsValueRef>(*key),
                               value->asJsValueRef()) != JsNoError) {
    return Just(false);
  }

  return Just(true);
}

int Object::InternalFieldCount() {
  JsObjectRef self = asJsValueRef()->asObject();
  TemplateData* tpData = (TemplateData*)self->extraData();
  NESCARGOT_ASSERT(tpData);
  return tpData->m_internalField.size();
}

Local<Value> Object::GetInternalField(int index) {
  JsObjectRef self = asJsValueRef()->asObject();
  TemplateData* tpData = (TemplateData*)self->extraData();
  NESCARGOT_ASSERT(tpData);

  size_t i = static_cast<size_t>(index);
  if (i >= tpData->m_internalField.size()) {
    return Local<Value>();
  }
  TemplateData::FieldValue* fieldValue =
      (TemplateData::FieldValue*)tpData->m_internalField[i];
  if (!fieldValue->value()) {
    return CreateJsUndefined();
  }

  return Local<Value>::New(IsolateShim::GetCurrent()->asIsolate(),
                           fieldValue->value());
}

void Object::SetInternalField(int index, Handle<Value> value) {
  JsObjectRef self = asJsValueRef()->asObject();
  TemplateData* tpData = (TemplateData*)self->extraData();
  NESCARGOT_ASSERT(tpData);

  size_t i = static_cast<size_t>(index);
  if (i >= tpData->m_internalField.size()) {
    return;
  }

  tpData->m_internalField[i]->setValue(*value);
}

void* Object::GetAlignedPointerFromInternalField(int index) {
  JsObjectRef self = asJsValueRef()->asObject();
  TemplateData* tpData = (TemplateData*)self->extraData();
  NESCARGOT_ASSERT(tpData);

  size_t i = static_cast<size_t>(index);
  if (i >= tpData->m_internalField.size()) {
    return nullptr;
  }

  TemplateData::FieldValue* fieldValue =
      (TemplateData::FieldValue*)tpData->m_internalField[i];
  if (!fieldValue->value()) {
    return nullptr;
  }

  return fieldValue->value();
}

void Object::SetAlignedPointerInInternalField(int index, void* value) {
  JsObjectRef self = asJsValueRef()->asObject();
  TemplateData* tpData = (TemplateData*)self->extraData();
  NESCARGOT_ASSERT(tpData);

  size_t i = static_cast<size_t>(index);
  if (i >= tpData->m_internalField.size()) {
    return;
  }

  tpData->m_internalField[i]->setValue(value);
}

Local<Object> Object::Clone() {
  auto context = GetCurrentJsContextRef();
  JsObjectRef self = asJsValueRef()->asObject();

  JsValueRef constructor = JS_INVALID_REFERENCE;
  if (GetObjectConstructor(context, self, constructor) != JsNoError) {
    return Local<Object>();
  }

  JsObjectRef obj = JS_INVALID_REFERENCE;
  if (ConstructObject(context, constructor, obj) != JsNoError) {
    return Local<Object>();
  }

  JsValueRef result = JS_INVALID_REFERENCE;
  if (js::Call(CachedStringId::cloneObject, result, self, obj) != JsNoError) {
    return Local<Object>();
  }
  return Local<Object>(obj);
}

Local<Context> Object::CreationContext() {
  JsObjectRef self = asJsValueRef()->asObject();
  auto context = self->creationContext();
  if (!context.hasValue()) {
    return Local<Context>();
  }

  IsolateShim* isolateShim = IsolateShim::GetCurrent();
  ContextShim* contextShim =
      isolateShim->GetContextShimFromJsContext(context.get());
  if (contextShim == nullptr) {
    return Local<Context>();
  }
  return Local<Context>::New(isolateShim->asIsolate(), contextShim);
}

Isolate* Object::GetIsolate() {
  // TODO: Consider get Isolate from object
  return IsolateShim::GetCurrent()->asIsolate();
}

Local<Object> Object::New(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef contextRef =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsObjectRef object = JS_INVALID_REFERENCE;
  if (CreateJsObject(contextRef, object) != JsNoError) {
    return Local<Object>();
  }

  return Local<Object>::New(isolate, object);
}

Object* Object::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsObject());
  return static_cast<Object*>(obj);
}

// Maybe<bool> Object::CreateDataProperty(Local<Context> context, Local<Name>
// key,
//                                        Local<Value> value) {
//   return Set(context, key, value);
// }

// Maybe<bool> Object::CreateDataProperty(Local<Context> context, uint32_t
// index,
//                                        Local<Value> value) {
//   return Set(context, index, value);
// }

void Object::SetAccessorProperty(Local<Name> name,
                                 Local<Function> getter,
                                 Local<Function> setter,
                                 PropertyAttribute attribute,
                                 AccessControl settings) {
  NESCARGOT_UNIMPLEMENTED("");
  //   JsPropertyIdRef idRef;
  //   if (jsrt::GetPropertyIdFromName((JsValueRef)*name, &idRef) != JsNoError)
  //   {
  //     return;
  //   }

  //   jsrt::PropertyDescriptorOptionValues enumerable =
  //       jsrt::GetPropertyDescriptorOptionValue(!(attribute & DontEnum));
  //   jsrt::PropertyDescriptorOptionValues configurable =
  //       jsrt::GetPropertyDescriptorOptionValue(!(attribute & DontDelete));

  //   // CHAKRA-TODO: we ignore  AccessControl for now..

  //   if (jsrt::DefineProperty((JsValueRef)this,
  //                            idRef,
  //                            jsrt::PropertyDescriptorOptionValues::None,
  //                            enumerable,
  //                            configurable,
  //                            JS_INVALID_REFERENCE,
  //                            (JsValueRef)*getter,
  //                            (JsValueRef)*setter) != JsNoError) {
  //     return;
  //   }
}

Maybe<bool> Object::SetIntegrityLevel(Local<Context> context,
                                      IntegrityLevel level) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

MaybeLocal<v8::Array> Object::PreviewEntries(bool* is_key_value) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Array>();
}

int v8::Object::GetIdentityHash() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}
}  // namespace v8
