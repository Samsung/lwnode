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

namespace v8 {

using namespace EscargotShim;

Local<ObjectTemplate> ObjectTemplate::New(Isolate* isolate,
                                          Local<FunctionTemplate> constructor) {
  NESCARGOT_ASSERT(isolate);
  JsContextRef contextRef =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsObjectRef objectTemplateWrapper = nullptr;
  if (CreateJsObject(contextRef, objectTemplateWrapper) != JsNoError) {
    return Local<ObjectTemplate>();
  }

  JsObjectTemplateRef self = CreateJsObjectTemplate();
  TemplateData* tpData = new TemplateData(isolate, constructor);

  self->setInstanceExtraData(tpData);
  objectTemplateWrapper->setExtraData(self);

  return Local<ObjectTemplate>::New(IsolateShim::GetCurrent()->asIsolate(),
                                    objectTemplateWrapper);
}

MaybeLocal<Object> ObjectTemplate::NewInstance(Local<Context> context) {
  JsObjectTemplateRef self = TemplateData::castToJsObjectTemplateRef(this);
  ContextShim* contextShim = context->asContextShim();
  JsContextRef contextRef = contextShim->contextRef();
  JsObjectRef newInstance = self->instantiate(contextRef);
  return Local<Object>::New(contextShim->isolateShim()->asIsolate(),
                            newInstance);
}

void ObjectTemplate::SetAccessor(Handle<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter,
                                 Handle<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 Handle<AccessorSignature> signature) {
  return SetAccessor(name,
                     reinterpret_cast<AccessorNameGetterCallback>(getter),
                     reinterpret_cast<AccessorNameSetterCallback>(setter),
                     data,
                     settings,
                     attribute,
                     signature);
}

void ObjectTemplate::SetAccessor(Handle<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 Handle<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 Handle<AccessorSignature> signature) {
  NESCARGOT_ASSERT(*name);
  NESCARGOT_ASSERT(getter);

  JsObjectTemplateRef self = TemplateData::castToJsObjectTemplateRef(this);
  JsValueRef nameRef = name->asJsValueRef();
  JsTemplatePropertyNameRef propertyName;
  if (nameRef->isString()) {
    propertyName = JsTemplatePropertyNameRef(nameRef->asString());
  } else {
    propertyName = JsTemplatePropertyNameRef(nameRef->asSymbol());
  }

  bool isWritable = !(attribute & ReadOnly);
  bool isEnumerable = !(attribute & DontEnum);
  bool isConfigurable = !(attribute & DontDelete);

  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  NESCARGOT_ASSERT(tpData);
  tpData->m_name = name;
  tpData->m_getter = getter;
  tpData->m_setter = setter;
  tpData->m_accessorData = data;

  auto getterCallback =
      [](JsExecutionStateRef state,
         JsObjectRef self,
         JsNativeDataAccessorPropertyData* data) -> JsValueRef {
    TemplateData* tpData = reinterpret_cast<TemplateData*>(self->extraData());

    Local<Object> thisObject =
        Local<Object>::New(IsolateShim::GetCurrent()->asIsolate(), self);
    v8::PropertyCallbackInfo<v8::Value> info(
        tpData->m_accessorData, thisObject, thisObject);

    NESCARGOT_ASSERT(tpData->m_getter);
    tpData->m_getter(tpData->m_name, info);

    v8::Value* ret = info.GetReturnValue().Get();
    if (ret) {
      return ret->asJsValueRef();
    }

    return CreateJsUndefined();
  };

  JsNativeDataAccessorPropertySetter setterCallback = nullptr;
  if (setter) {
    setterCallback = [](JsExecutionStateRef state,
                        JsObjectRef self,
                        JsNativeDataAccessorPropertyData* data,
                        JsValueRef setterInputData) -> bool {
      TemplateData* tpData = reinterpret_cast<TemplateData*>(self->extraData());

      Local<Object> thisObject =
          Local<Object>::New(IsolateShim::GetCurrent()->asIsolate(), self);
      Local<Object> setValue = Local<Object>::New(
          IsolateShim::GetCurrent()->asIsolate(), setterInputData);
      v8::PropertyCallbackInfo<void> info(
          tpData->m_accessorData, thisObject, thisObject);

      tpData->m_setter(tpData->m_name, setValue, info);

      return CreateJsUndefined();
    };
  }

  self->setNativeDataAccessorProperty(propertyName,
                                      getterCallback,
                                      setterCallback,
                                      isWritable,
                                      isEnumerable,
                                      isConfigurable);
}

void ObjectTemplate::SetNamedPropertyHandler(
    NamedPropertyGetterCallback getter,
    NamedPropertySetterCallback setter,
    NamedPropertyQueryCallback query,
    NamedPropertyDeleterCallback deleter,
    NamedPropertyEnumeratorCallback enumerator,
    NamedPropertyDefinerCallback definer,
    NamedPropertyDescriptorCallback descriptor,
    Handle<Value> data) {
  NamedPropertyHandlerConfiguration config(
      reinterpret_cast<GenericNamedPropertyGetterCallback>(getter),
      reinterpret_cast<GenericNamedPropertySetterCallback>(setter),
      reinterpret_cast<GenericNamedPropertyQueryCallback>(query),
      reinterpret_cast<GenericNamedPropertyDeleterCallback>(deleter),
      reinterpret_cast<GenericNamedPropertyEnumeratorCallback>(enumerator),
      reinterpret_cast<GenericNamedPropertyDefinerCallback>(definer),
      reinterpret_cast<GenericNamedPropertyDescriptorCallback>(descriptor),
      data);
  SetHandler(config);
}

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  JsObjectTemplateRef self = TemplateData::castToJsObjectTemplateRef(this);
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  NESCARGOT_ASSERT(tpData);

  tpData->m_namedPropertyHandler = config;

  JsObjectTemplateNamedPropertyHandlerData handler;
  if (config.getter) {
    handler.getter = [](JsExecutionStateRef state,
                        JsObjectRef self,
                        void* data,
                        const JsTemplatePropertyNameRef& propertyName)
        -> Escargot::OptionalRef<Escargot::ValueRef> {
      TemplateData* tpData = reinterpret_cast<TemplateData*>(self->extraData());
      NESCARGOT_ASSERT(tpData);

      if (!tpData->m_namedPropertyHandler.getter) {
        return Escargot::OptionalRef<Escargot::ValueRef>();
      }

      Local<Name> v8PropertyName = Local<Name>::New(
          IsolateShim::GetCurrent()->asIsolate(), propertyName.value());
      Local<Object> thisObject =
          Local<Object>::New(IsolateShim::GetCurrent()->asIsolate(), self);
      v8::PropertyCallbackInfo<v8::Value> info(
          tpData->m_namedPropertyHandler.data, thisObject, thisObject);

      tpData->m_namedPropertyHandler.getter(v8PropertyName, info);
      v8::Value* ret = info.GetReturnValue().Get();
      if (ret) {
        return ret->asJsValueRef();
      }

      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.setter) {
    handler.setter =
        [](JsExecutionStateRef state,
           JsObjectRef self,
           void* data,
           const JsTemplatePropertyNameRef& propertyName,
           JsValueRef value) -> Escargot::OptionalRef<Escargot::ValueRef> {
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.query) {
    handler.query = [](JsExecutionStateRef state,
                       JsObjectRef self,
                       void* data,
                       const JsTemplatePropertyNameRef& propertyName)
        -> JsTemplatePropertyAttribute {
      TemplateData* tpData = reinterpret_cast<TemplateData*>(self->extraData());
      NESCARGOT_ASSERT(tpData);

      if (!tpData->m_namedPropertyHandler.query) {
        return JsTemplatePropertyAttribute::TemplatePropertyAttributeNotExist;
      }

      Local<Name> v8PropertyName = Local<Name>::New(
          IsolateShim::GetCurrent()->asIsolate(), propertyName.value());
      Local<Object> thisObject =
          Local<Object>::New(IsolateShim::GetCurrent()->asIsolate(), self);
      v8::PropertyCallbackInfo<v8::Integer> info(
          tpData->m_namedPropertyHandler.data, thisObject, thisObject);

      tpData->m_namedPropertyHandler.query(v8PropertyName, info);
      v8::Value* ret = info.GetReturnValue().Get();
      if (ret) {
        return JsTemplatePropertyAttribute::TemplatePropertyAttributeExist;
      }

      return JsTemplatePropertyAttribute::TemplatePropertyAttributeNotExist;
    };
  }

  if (config.deleter) {
    handler.deleter = [](JsExecutionStateRef state,
                         JsObjectRef self,
                         void* data,
                         const JsTemplatePropertyNameRef& propertyName)
        -> Escargot::OptionalRef<Escargot::ValueRef> {
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.enumerator) {
    handler.enumerator =
        [](JsExecutionStateRef state, JsObjectRef self, void* data)
        -> JsTemplateNamedPropertyHandlerEnumerationCallbackResultVector {
      JsTemplateNamedPropertyHandlerEnumerationCallbackResultVector v(0);
      return v;
    };
  }

  if (config.definer) {
    handler.definer = [](JsExecutionStateRef state,
                         JsObjectRef self,
                         void* data,
                         const JsTemplatePropertyNameRef& propertyName,
                         const JsObjectPropertyDescriptorRef& desc)
        -> Escargot::OptionalRef<Escargot::ValueRef> {
      return Escargot::OptionalRef<Escargot::ValueRef>();
    };
  }

  if (config.descriptor) {
    handler.descriptor =
        [](JsExecutionStateRef state,
           JsObjectRef self,
           void* data) -> Escargot::OptionalRef<Escargot::ObjectRef> {
      return Escargot::OptionalRef<Escargot::ObjectRef>();
    };
  }

  if (*(tpData->m_namedPropertyHandler.data)) {
    handler.data = tpData->m_namedPropertyHandler.data->asJsValueRef();
  }
  self->setNamedPropertyHandler(handler);
}

void ObjectTemplate::SetIndexedPropertyHandler(
    IndexedPropertyGetterCallback getter,
    IndexedPropertySetterCallback setter,
    IndexedPropertyQueryCallback query,
    IndexedPropertyDeleterCallback deleter,
    IndexedPropertyEnumeratorCallback enumerator,
    IndexedPropertyDefinerCallback definer,
    IndexedPropertyDescriptorCallback descriptor,
    Handle<Value> data) {
  IndexedPropertyHandlerConfiguration config(
      getter, setter, query, deleter, enumerator, definer, descriptor, data);
  SetHandler(config);
}

void ObjectTemplate::SetHandler(
    const IndexedPropertyHandlerConfiguration& config) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ObjectTemplate::SetAccessCheckCallbacks(
    NamedSecurityCallback named_callback,
    IndexedSecurityCallback indexed_callback,
    Handle<Value> data,
    bool turned_on_by_default) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ObjectTemplate::SetInternalFieldCount(int value) {
  JsObjectTemplateRef objectTemplate =
      TemplateData::castToJsObjectTemplateRef(this);
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(objectTemplate->instanceExtraData());
  NESCARGOT_ASSERT(tpData);

  tpData->m_internalField.clear();
  tpData->m_internalField.reserve(value);
  for (int i = 0; i < value; i++) {
    tpData->m_internalField.push_back(new TemplateData::FieldValue(nullptr));
  }
}

void ObjectTemplate::SetCallAsFunctionHandler(FunctionCallback callback,
                                              Handle<Value> data) {
  NESCARGOT_UNIMPLEMENTED("");
}

}  // namespace v8
