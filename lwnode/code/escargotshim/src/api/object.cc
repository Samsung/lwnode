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

#include "object.h"
#include "api.h"
#include "base.h"
#include "isolate.h"

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

using AccessorNameCallbackDataWrap =
    NativeDataAccessorPropertyDataWrap<AccessorNameGetterCallback,
                                       AccessorNameSetterCallback>;

static ValueRef* accessorPropertyGetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ObjectRef::NativeDataAccessorPropertyData* data) {
  auto wrapper = AccessorNameCallbackDataWrap::toWrap(data);

  PropertyCallbackInfoWrap<v8::Value> info(
      wrapper->m_isolate, self, self, VAL(wrapper->m_data));

  auto v8Getter = wrapper->m_getter;
  LWNODE_CHECK_NOT_NULL(v8Getter);
  v8Getter(v8::Utils::ToLocal<Name>(VAL(wrapper->m_name)),
           info);

  return VAL(*info.GetReturnValue().Get())->value();
}

static bool accessorPropertySetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ObjectRef::NativeDataAccessorPropertyData* data,
    ValueRef* setterInputData) {
  auto wrapper = AccessorNameCallbackDataWrap::toWrap(data);

  HandleScope handle_scope(wrapper->m_isolate);

  Local<Value> v8SetValue =
      v8::Utils::NewLocal<Value>(wrapper->m_isolate, setterInputData);

  PropertyCallbackInfoWrap<void> info(
      wrapper->m_isolate, self, self, VAL(wrapper->m_data));

  auto v8Setter = wrapper->m_setter;
  LWNODE_CHECK_NOT_NULL(v8Setter);
  v8Setter(v8::Utils::ToLocal<Name>(VAL(wrapper->m_name)),
           v8SetValue,
           info);

  return true;
}

template <typename T, typename F>
NativeDataAccessorPropertyDataWrap<T, F>::NativeDataAccessorPropertyDataWrap(
    v8::Isolate* isolate,
    Local<Name> name,
    T getter,
    F setter,
    Local<Value> data,
    bool isWritable,
    bool isEnumerable,
    bool isConfigurable)
    : NativeDataAccessorPropertyData(
          isWritable,
          isEnumerable,
          isConfigurable,
          accessorPropertyGetter,
          setter == nullptr ? nullptr : accessorPropertySetter),
      m_isolate(isolate),
      m_name(*name),
      m_getter(getter),
      m_setter(setter),
      m_data(*data) {
  LWNODE_CHECK_NOT_NULL(getter);
}

Maybe<bool> ObjectUtils::SetAccessor(ObjectRef* esObject,
                                     IsolateWrap* lwIsolate,
                                     Local<Name> name,
                                     AccessorNameGetterCallback getter,
                                     AccessorNameSetterCallback setter,
                                     Local<Value> data,
                                     PropertyAttribute attribute) {
  auto accessorWrapData =
      new AccessorNameCallbackDataWrap(lwIsolate->toV8(),
                                       name,
                                       getter,
                                       setter,
                                       data,
                                       !(attribute & v8::ReadOnly),
                                       !(attribute & v8::DontEnum),
                                       !(attribute & v8::DontDelete));
  auto esName = CVAL(*name)->value();
  ValueRef* esPropertyName = nullptr;
  if (esName->isString()) {
    esPropertyName = ValueRef::create(esName->asString());
  } else if (esName->isSymbol()) {
    esPropertyName = ValueRef::create(esName->asSymbol());
  }

  LWNODE_CHECK_NOT_NULL(esName);

  auto result = Evaluator::execute(
      lwIsolate->GetCurrentContext()->get(),
      [](ExecutionStateRef* esState,
         ObjectRef* esSelf,
         ValueRef* esName,
         AccessorNameCallbackDataWrap* data) {
        return ValueRef::create(
            esSelf->defineNativeDataAccessorProperty(esState, esName, data));
      },
      esObject,
      esPropertyName,
      accessorWrapData);
  API_HANDLE_EXCEPTION(result, lwIsolate, Nothing<bool>());
  return Just(result.result->asBoolean());
}

template void ObjectTemplateUtils::SetAccessor<AccessorNameGetterCallback,
                                               AccessorNameSetterCallback>(
    ObjectTemplateRef* esObjectTemplate,
    IsolateWrap* lwIsolate,
    Local<Name> name,
    AccessorNameGetterCallback getter,
    AccessorNameSetterCallback setter,
    Local<Value> data,
    PropertyAttribute attribute);

template void ObjectTemplateUtils::SetAccessor<AccessorGetterCallback,
                                               AccessorSetterCallback>(
    ObjectTemplateRef* esObjectTemplate,
    IsolateWrap* lwIsolate,
    Local<Name> name,
    AccessorGetterCallback getter,
    AccessorSetterCallback setter,
    Local<Value> data,
    PropertyAttribute attribute);

template <typename T, typename F>
void ObjectTemplateUtils::SetAccessor(ObjectTemplateRef* esObjectTemplate,
                                      IsolateWrap* lwIsolate,
                                      Local<Name> name,
                                      T getter,
                                      F setter,
                                      Local<Value> data,
                                      PropertyAttribute attribute) {
  auto esName = CVAL(*name)->value();
  TemplatePropertyNameRef esPropertyName;
  if (esName->isString()) {
    esPropertyName = TemplatePropertyNameRef(esName->asString());
  } else if (esName->isSymbol()) {
    esPropertyName = TemplatePropertyNameRef(esName->asSymbol());
  }

  auto accessorWrapData = new NativeDataAccessorPropertyDataWrap<T, F>(
      lwIsolate->toV8(),
      name,
      getter,
      setter,
      data,
      !(attribute & v8::ReadOnly),
      !(attribute & v8::DontEnum),
      !(attribute & v8::DontDelete));

  esObjectTemplate->setNativeDataAccessorProperty(esPropertyName,
                                                  accessorWrapData);
}

}  // namespace EscargotShim
