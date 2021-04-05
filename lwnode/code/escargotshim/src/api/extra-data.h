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

namespace EscargotShim {

class ArrayBufferObjectData;
class ValueWrap;
class SetAccessorFunctionData;

class ObjectData : public gc {
 public:
  ArrayBufferObjectData* asArrayBufferObjectData() {
    LWNODE_CHECK(isArryBufferObjectData());
    return reinterpret_cast<ArrayBufferObjectData*>(this);
  }

  virtual bool isArryBufferObjectData() const { return false; }

  void setSetAccessorFunctionData(SetAccessorFunctionData* fnData) {
    m_setAccessorFunctionData = fnData;
  }

  SetAccessorFunctionData* getSetAccessorFunctionData() {
    return m_setAccessorFunctionData;
  }

 private:
  SetAccessorFunctionData* m_setAccessorFunctionData{nullptr};
};

class ArrayBufferObjectData : public ObjectData {
 public:
  bool isArryBufferObjectData() const override { return true; }

  void setBackingStoreWrapHolder(BackingStoreWrapHolder* holder) {
    m_backingStoreWrapHolder = holder;
  }

  BackingStoreWrapHolder* backingStoreWrapHolder() {
    return m_backingStoreWrapHolder.get();
  }

 private:
  PersistentRefHolder<BackingStoreWrapHolder> m_backingStoreWrapHolder;
};

class TemplateData : public gc {
 public:
  TemplateData(v8::Isolate* isolate) : m_isolate(isolate) {}
  v8::Isolate* isolate() { return m_isolate; }

  virtual bool isFunctionTemplateData() = 0;
  virtual bool isObjectTemplateData() = 0;

 private:
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

  static FunctionTemplateData* toTemplateData(void* ptr) {
    LWNODE_CHECK_NOT_NULL(ptr);
    auto data = reinterpret_cast<FunctionTemplateData*>(ptr);
    LWNODE_CHECK(data->isFunctionTemplateData());
    return data;
  }

  bool isFunctionTemplateData() override { return true; }
  bool isObjectTemplateData() override { return false; }

  v8::FunctionCallback m_callback;
  v8::Local<v8::Value> m_callbackData;
  v8::Local<v8::Signature> m_signature;
  int m_length{0};
};

class ObjectTemplateData : public TemplateData {
 public:
  ObjectTemplateData(v8::Isolate* isolate) : TemplateData(isolate) {}

  static ObjectTemplateData* toTemplateData(void* ptr) {
    LWNODE_CHECK_NOT_NULL(ptr);
    auto data = reinterpret_cast<ObjectTemplateData*>(ptr);
    LWNODE_CHECK(data->isObjectTemplateData());
    return data;
  }

  bool isFunctionTemplateData() override { return false; }
  bool isObjectTemplateData() override { return true; }

  v8::Local<v8::Name> m_name;
  v8::internal::Address m_getter{0};
  v8::internal::Address m_setter{0};
  v8::Local<v8::Value> m_accessorData;
  v8::NamedPropertyHandlerConfiguration m_namedPropertyHandler;
};

class SetAccessorFunctionData : public gc {
 public:
  SetAccessorFunctionData(v8::Isolate* isolate,
                          v8::Local<v8::Name> name,
                          ObjectRef* self,
                          v8::AccessorNameGetterCallback getter,
                          v8::AccessorNameSetterCallback setter,
                          ValueRef* data)
      : m_isolate(isolate),
        m_name(name),
        m_self(self),
        m_getter(getter),
        m_setter(setter),
        m_data(data) {}
  v8::Isolate* m_isolate{nullptr};
  v8::Local<v8::Name> m_name;
  ObjectRef* m_self{nullptr};
  v8::AccessorNameGetterCallback m_getter;
  v8::AccessorNameSetterCallback m_setter;
  ValueRef* m_data{nullptr};
};

}  // namespace EscargotShim
