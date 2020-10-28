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

#ifndef DEPS_ESCARGOTSHIM_SRC_V8ESCARGOT_H_
#define DEPS_ESCARGOTSHIM_SRC_V8ESCARGOT_H_

#include "escargotutil.h"
#include "v8.h"

// FIXME: It is better to have the following class definitions in
// escargotutil.h. Need to rearrange headers since the definitions require
// v8.h
namespace EscargotShim {

class TemplateData : public gc {
 public:
  class FieldValue : public gc {
   public:
    FieldValue(void* value) : m_value(value) {}

    void setValue(void* value) { m_value = value; }
    void* value() const { return m_value; }

   private:
    void* m_value{nullptr};
  };

  TemplateData(v8::Isolate* isolate,
               v8::Local<v8::FunctionTemplate> constructor)
      : m_isolate(isolate), m_constructor(constructor) {
    // the vector contains one item by default
    m_internalField.push_back(new FieldValue(nullptr));
  }

  TemplateData(v8::Isolate* isolate,
               v8::FunctionCallback callback,
               v8::Local<v8::Value> data,
               v8::Local<v8::Signature> signature,
               int length)
      : m_isolate(isolate),
        m_callback(callback),
        m_callbackData(data),
        m_signature(signature),
        m_length(length) {}

  TemplateData(const TemplateData& tpData)
      : m_isolate(tpData.m_isolate),
        m_constructor(tpData.m_constructor),
        m_name(tpData.m_name),
        m_getter(tpData.m_getter),
        m_setter(tpData.m_setter),
        m_accessorData(tpData.m_accessorData),
        m_namedPropertyHandler(tpData.m_namedPropertyHandler),
        m_callback(tpData.m_callback),
        m_callbackData(tpData.m_callbackData),
        m_signature(tpData.m_signature),
        m_length(tpData.m_length),
        m_className(tpData.m_className) {
    size_t size = tpData.m_internalField.size();

    for (size_t i = 0; i < size; i++) {
      TemplateData::FieldValue* fieldValue = new TemplateData::FieldValue(
          (TemplateData::FieldValue*)tpData.m_internalField[i]->value());
      m_internalField.push_back(fieldValue);
    }
  }

  static JsFunctionTemplateRef castToJsFunctionTemplateRef(
      v8::FunctionTemplate* v8FunctionTemplate);
  static JsObjectTemplateRef castToJsObjectTemplateRef(
      v8::ObjectTemplate* v8ObjectTemplate);

  // objectTemplate
  v8::Isolate* m_isolate{nullptr};
  v8::Local<v8::FunctionTemplate> m_constructor;
  v8::Local<v8::Name> m_name;
  GCVector<FieldValue*> m_internalField;
  v8::AccessorNameGetterCallback m_getter{nullptr};
  v8::AccessorNameSetterCallback m_setter{nullptr};
  v8::Local<v8::Value> m_accessorData;

  v8::NamedPropertyHandlerConfiguration m_namedPropertyHandler;

  // functionTemplate
  v8::FunctionCallback m_callback;
  v8::Local<v8::Value> m_callbackData;
  v8::Local<v8::Signature> m_signature;
  int m_length{0};
  v8::Local<v8::String> m_className;
};

}  // namespace EscargotShim

namespace v8 {
// NOTE: use type if needed
enum class ExternalDataTypes {
  Unknown,
  ObjectTemplateData,
  ObjectData,
  FunctionTemplateData,
  FunctionCallbackData,
  ArrayBufferData,
};

class ExternalData : public gc {
 private:
  const ExternalDataTypes type;

 public:
  explicit ExternalData(ExternalDataTypes type);
  virtual ~ExternalData() {}

  ExternalDataTypes GetType() const { return type; }

  template <class T>
  static bool Is(T* data) {
    return data->GetType() == T::ExternalDataType;
  }

  // static JsErrorCode GetExternalData(JsValueRef ref, ExternalData** data) {
  //   return JsGetExternalData(ref, reinterpret_cast<void**>(data));
  // }

  template <class T>
  static T* GetExternalData(JsValueRef ref) {
    auto p =
        static_cast<ExternalData*>(EscargotShim::GetExtraData(ref->asObject()));
    T* data = (p != nullptr && p->GetType() == T::ExternalDataType)
                  ? static_cast<T*>(p)
                  : nullptr;

    // NESCARGOT_ASSERT(*data == nullptr || (error == JsNoError && Is(*data)));
    return data;
  }

  // template <class T>
  // static bool TryGet(JsValueRef ref, T** data) {
  //   return GetExternalData(ref, data) == JsNoError && *data != nullptr;
  // }
};

class ArrayBufferData : public ExternalData {
 public:
  static const ExternalDataTypes ExternalDataType =
      ExternalDataTypes::ArrayBufferData;
  ArrayBufferData(ArrayBufferCreationMode mode)
      : ExternalData(ExternalDataType), m_mode(mode) {}
  ~ArrayBufferData() = default;
  ArrayBufferCreationMode m_mode;
};

class Utils {
 public:
  template <class Func>
  static Local<Value> NewError(Handle<String> message);
  static Local<Value> NewError(Handle<String> message,
                               JsErrorObjectCode errorCode);
  static JsValueRef AccessorNativeFunctionCallback(JsExecutionStateRef state,
                                                   JsValueRef thisValue,
                                                   size_t argc,
                                                   JsValueRef* argv,
                                                   bool isNewExpression);

  static std::string GetStdString(Handle<String> string);

  static JsObjectRef CreateExternalObjectFromObjectTemplate(
      JsContextRef context, ObjectTemplate* objectTemplate);

  static JsValueRef getProxyFunction(JsExecutionStateRef state,
                                     JsValueRef thisValue,
                                     size_t argc,
                                     JsValueRef* argv,
                                     bool isNewExpression);

  static JsValueRef setProxyFunction(JsExecutionStateRef state,
                                     JsValueRef thisValue,
                                     size_t argc,
                                     JsValueRef* argv,
                                     bool isNewExpression);

  static JsValueRef deletePropertyProxyFunction(JsExecutionStateRef state,
                                                JsValueRef thisValue,
                                                size_t argc,
                                                JsValueRef* argv,
                                                bool isNewExpression);
  static JsValueRef ownKeysProxyFunction(JsExecutionStateRef state,
                                         JsValueRef thisValue,
                                         size_t argc,
                                         JsValueRef* argv,
                                         bool isNewExpression);
  static JsValueRef hasProxyFunction(JsExecutionStateRef state,
                                     JsValueRef thisValue,
                                     size_t argc,
                                     JsValueRef* argv,
                                     bool isNewExpression);

  static JsValueRef GetPropertiesHandler(size_t argc,
                                         JsValueRef* argv,
                                         bool getFromPrototype);

  static JsValueRef getOwnPropertyDescriptorProxyFunction(
      JsExecutionStateRef state,
      JsValueRef thisValue,
      size_t argc,
      JsValueRef* argv,
      bool isNewExpression);

  static bool CheckSignature(Local<FunctionTemplate> receiver,
                             Local<Object> thisPointer,
                             Local<Object>* holder);

  static Maybe<bool> Set(Handle<Context> context,
                         Handle<Object> object,
                         Handle<Value> key,
                         Handle<Value> value,
                         PropertyAttribute attribs,
                         bool force);

  static Maybe<bool> SetAccessor(Handle<Context> context,
                                 Handle<Object> object,
                                 Handle<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 v8::Handle<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attributes,
                                 Handle<AccessorSignature> signature);

  // Create a Local<T> internally (use private constructor)
  template <class T>
  static Local<T> ToLocal(T* that) {
    return Local<T>(that);
  }

  template <class T1, class T2>
  static Local<T2> NewTypedArray(Handle<ArrayBuffer> array_buffer,
                                 size_t byte_offset,
                                 size_t arrayLength,
                                 ExternalArrayType type);

  static void WeakReferenceCallbackWrapperCallback(void* self, void* data);

  static bool IsInstanceOf(Object* obj, ObjectTemplate* objectTemplate) {
    return obj->GetObjectTemplate() == objectTemplate;
  }
};

}  // namespace v8

#endif
