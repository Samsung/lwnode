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
// --- D a t a ---

bool Value::FullIsUndefined() const {
  LWNODE_RETURN_FALSE;
}

bool Value::FullIsNull() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsTrue() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsFalse() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsName() const {
  LWNODE_RETURN_FALSE;
}

bool Value::FullIsString() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSymbol() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArray() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArrayBuffer() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArrayBufferView() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsTypedArray() const {
  LWNODE_RETURN_FALSE;
}

#define VALUE_IS_TYPED_ARRAY(Type, typeName, TYPE, ctype)                      \
  bool Value::Is##Type##Array() const { LWNODE_RETURN_FALSE; }

TYPED_ARRAYS(VALUE_IS_TYPED_ARRAY)

#undef VALUE_IS_TYPED_ARRAY

bool Value::IsDataView() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSharedArrayBuffer() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsObject() const {
  return CVAL(this)->value()->isObject();
}

bool Value::IsNumber() const {
  return CVAL(this)->value()->isNumber();
}

bool Value::IsBigInt() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsProxy() const {
  LWNODE_RETURN_FALSE;
}

#define VALUE_IS_SPECIFIC_TYPE(Type, Check)                                    \
  bool Value::Is##Type() const { LWNODE_RETURN_FALSE; }

VALUE_IS_SPECIFIC_TYPE(ArgumentsObject, JSArgumentsObject)
VALUE_IS_SPECIFIC_TYPE(BigIntObject, BigIntWrapper)
VALUE_IS_SPECIFIC_TYPE(BooleanObject, BooleanWrapper)
VALUE_IS_SPECIFIC_TYPE(NumberObject, NumberWrapper)
VALUE_IS_SPECIFIC_TYPE(StringObject, StringWrapper)
VALUE_IS_SPECIFIC_TYPE(SymbolObject, SymbolWrapper)
VALUE_IS_SPECIFIC_TYPE(Date, JSDate)
VALUE_IS_SPECIFIC_TYPE(Map, JSMap)
VALUE_IS_SPECIFIC_TYPE(Set, JSSet)
VALUE_IS_SPECIFIC_TYPE(WasmModuleObject, WasmModuleObject)
VALUE_IS_SPECIFIC_TYPE(WeakMap, JSWeakMap)
VALUE_IS_SPECIFIC_TYPE(WeakSet, JSWeakSet)

#undef VALUE_IS_SPECIFIC_TYPE

bool Value::IsBoolean() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsInt32() const {
  return CVAL(this)->value()->isInt32();
}

bool Value::IsUint32() const {
  return CVAL(this)->value()->isUInt32();
}

bool Value::IsNativeError() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsRegExp() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsAsyncFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsGeneratorFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsGeneratorObject() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsMapIterator() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSetIterator() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsPromise() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsModuleNamespaceObject() const {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<String>());

  auto esValue = CVAL(this)->value();
  if (esValue->isString()) {
    return Local<String>::New(lwIsolate->toV8(),
                              ValueWrap::createValue(esValue));
  }

  LWNODE_UNIMPLEMENT;

  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<BigInt> Value::ToBigInt(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(BigInt)
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  LWNODE_RETURN_FALSE;
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  LWNODE_RETURN_LOCAL(Boolean);
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Number);
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Integer);
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Int32);
}

MaybeLocal<Uint32> Value::ToUint32(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Uint32);
}

i::Isolate* i::IsolateFromNeverReadOnlySpaceObject(i::Address obj) {
  LWNODE_RETURN_NULLPTR;
  ;
}

bool i::ShouldThrowOnError(i::Isolate* isolate) {
  LWNODE_RETURN_FALSE;
}

void i::Internals::CheckInitializedImpl(v8::Isolate* external_isolate) {
  i::Isolate* isolate = reinterpret_cast<i::Isolate*>(external_isolate);
  LWNODE_CHECK_NOT_NULL(isolate);
}

void External::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Object::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Function::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Boolean::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Name::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::String::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Symbol::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Private::CheckCast(v8::Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::Number::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Integer::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Int32::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Uint32::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BigInt::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Array::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Map::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Set::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Promise::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Promise::Resolver::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Proxy::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::WasmModuleObject::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

// void v8::debug::AccessorPair::CheckCast(Value* that) {}

// void v8::debug::WasmValue::CheckCast(Value* that) {}

v8::BackingStore::~BackingStore() {}

void* v8::BackingStore::Data() const {
  LWNODE_RETURN_NULLPTR;
}

size_t v8::BackingStore::ByteLength() const {
  LWNODE_RETURN_0;
}

bool v8::BackingStore::IsShared() const {
  LWNODE_RETURN_FALSE;
}

// static
std::unique_ptr<v8::BackingStore> v8::BackingStore::Reallocate(
    v8::Isolate* isolate,
    std::unique_ptr<v8::BackingStore> backing_store,
    size_t byte_length) {
  LWNODE_UNIMPLEMENT;
  return backing_store;
}

// static
void v8::BackingStore::EmptyDeleter(void* data,
                                    size_t length,
                                    void* deleter_data) {}

std::shared_ptr<v8::BackingStore> v8::ArrayBuffer::GetBackingStore() {
  LWNODE_RETURN_NULLPTR;
}

std::shared_ptr<v8::BackingStore> v8::SharedArrayBuffer::GetBackingStore() {
  LWNODE_RETURN_NULLPTR;
}

void v8::ArrayBuffer::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::ArrayBufferView::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

constexpr size_t v8::TypedArray::kMaxLength;

void v8::TypedArray::CheckCast(Value* that) {
  LWNODE_UNIMPLEMENT;
}

#define CHECK_TYPED_ARRAY_CAST(Type, typeName, TYPE, ctype)                    \
  void v8::Type##Array::CheckCast(Value* that) { LWNODE_UNIMPLEMENT; }

TYPED_ARRAYS(CHECK_TYPED_ARRAY_CAST)

#undef CHECK_TYPED_ARRAY_CAST

void v8::DataView::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::SharedArrayBuffer::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Date::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::StringObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::SymbolObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::NumberObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BigIntObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BooleanObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::RegExp::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(double);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int64_t);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int32_t);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(uint32_t);
}

MaybeLocal<Uint32> Value::ToArrayIndex(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Uint32);
}

Maybe<bool> Value::Equals(Local<Context> context, Local<Value> that) const {
  LWNODE_RETURN_MAYBE(bool)
}

bool Value::StrictEquals(Local<Value> that) const {
  LWNODE_RETURN_FALSE;
}

bool Value::SameValue(Local<Value> that) const {
  LWNODE_RETURN_FALSE;
}

Local<String> Value::TypeOf(v8::Isolate* external_isolate) {
  LWNODE_RETURN_LOCAL(String);
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> object){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            v8::Local<Value> key,
                            v8::Local<Value> value){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            uint32_t index,
                            v8::Local<Value> value){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           v8::Local<Name> key,
                                           v8::Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           uint32_t index,
                                           v8::Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool)
}

struct v8::PropertyDescriptor::PrivateData {
  PrivateData() {}
};

v8::PropertyDescriptor::PropertyDescriptor() : private_(nullptr) {}

// DataDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

// DataDescriptor with writable field
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
                                           bool writable)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

// AccessorDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
                                           v8::Local<v8::Value> set)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

v8::PropertyDescriptor::~PropertyDescriptor() {
  delete private_;
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  LWNODE_RETURN_LOCAL(Value);
}

bool v8::PropertyDescriptor::has_value() const {
  LWNODE_RETURN_FALSE;
}
bool v8::PropertyDescriptor::has_get() const {
  LWNODE_RETURN_FALSE;
}
bool v8::PropertyDescriptor::has_set() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::writable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_writable() const {
  LWNODE_RETURN_FALSE;
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {}

bool v8::PropertyDescriptor::enumerable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_enumerable() const {
  LWNODE_RETURN_FALSE;
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {}

bool v8::PropertyDescriptor::configurable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_configurable() const {
  LWNODE_RETURN_FALSE;
}

Maybe<bool> v8::Object::DefineOwnProperty(v8::Local<v8::Context> context,
                                          v8::Local<Name> key,
                                          v8::Local<Value> value,
                                          v8::PropertyAttribute attributes) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context,
                                   Local<Private> key,
                                   Local<Value> value) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto esContext = lwIsolate->CurrentContext()->get();

  EvalResult r = ObjectRefHelper::setPrivate(esContext,
                                             VAL(this)->value()->asObject(),
                                             VAL(*key)->value(),
                                             VAL(*value)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(true);
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
                                  Local<Value> key) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());

  auto esContext = lwIsolate->CurrentContext()->get();

  EvalResult r = ObjectRefHelper::getPrivate(
      esContext, VAL(this)->value()->asObject(), VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  API_RETURN_LOCAL(Value, lwIsolate->toV8(), r.result);
}

Maybe<PropertyAttribute> v8::Object::GetPropertyAttributes(
    Local<Context> context, Local<Value> key) {
  LWNODE_RETURN_MAYBE(PropertyAttribute);
}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                       Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> v8::Object::GetPrototype() {
  auto lwIsolate = IsolateWrap::currentIsolate();
  auto esContext = lwIsolate->CurrentContext()->get();
  auto esObject =
      ObjectRefHelper::getPrototype(esContext, VAL(this)->value()->asObject());

  API_RETURN_LOCAL(Value, lwIsolate->toV8(), esObject);
}

Maybe<bool> v8::Object::SetPrototype(Local<Context> context,
                                     Local<Value> value) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto esContext = VAL(*context)->context()->get();
  auto esSelf = VAL(this)->value()->asObject();
  auto esValue = VAL(*value)->value();

  EvalResult r = ObjectRefHelper::setPrototype(esContext, esSelf, esValue);

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(true);
}

Local<Object> v8::Object::FindInstanceInPrototypeChain(
    v8::Local<FunctionTemplate> tmpl) {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context,
    KeyCollectionMode mode,
    PropertyFilter property_filter,
    IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(
    Local<Context> context,
    PropertyFilter filter,
    KeyConversionMode key_conversion) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<String> v8::Object::ObjectProtoToString(Local<Context> context) {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> v8::Object::GetConstructorName() {
  LWNODE_RETURN_LOCAL(String);
}

Maybe<bool> v8::Object::SetIntegrityLevel(Local<Context> context,
                                          IntegrityLevel level){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Delete(Local<Context> context,
                               Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::Has(Local<Context> context,
                            Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context, Local<Private> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::Delete(Local<Context> context,
                               uint32_t index){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Has(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

template <typename Getter, typename Setter, typename Data>
static Maybe<bool> ObjectSetAccessor(Local<Context> context,
                                     Object* self,
                                     Local<Name> name,
                                     Getter getter,
                                     Setter setter,
                                     Data data,
                                     AccessControl settings,
                                     PropertyAttribute attributes,
                                     bool is_special_data_property,
                                     bool replace_on_access,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data,
                                AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type,
                                SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_MAYBE(bool)
}

void Object::SetAccessorProperty(Local<Name> name,
                                 Local<Function> getter,
                                 Local<Function> setter,
                                 PropertyAttribute attribute,
                                 AccessControl settings) {}

Maybe<bool> Object::SetNativeDataProperty(
    v8::Local<v8::Context> context,
    v8::Local<Name> name,
    AccessorNameGetterCallback getter,
    AccessorNameSetterCallback setter,
    v8::Local<Value> data,
    PropertyAttribute attributes,
    SideEffectType getter_side_effect_type,
    SideEffectType setter_side_effect_type){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Object::SetLazyDataProperty(v8::Local<v8::Context> context,
                                        v8::Local<Name> name,
                                        AccessorNameGetterCallback getter,
                                        v8::Local<Value> data,
                                        PropertyAttribute attributes,
                                        SideEffectType getter_side_effect_type,
                                        SideEffectType setter_side_effect_type){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
                                       Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealNamedProperty(Local<Context> context,
                                             Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealIndexedProperty(Local<Context> context,
                                               uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealNamedCallbackProperty(Local<Context> context,
                                                     Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

bool v8::Object::HasNamedLookupInterceptor() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::HasIndexedLookupInterceptor() {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<Value> v8::Object::GetRealNamedPropertyInPrototypeChain(
    Local<Context> context, Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<PropertyAttribute>
v8::Object::GetRealNamedPropertyAttributesInPrototypeChain(
    Local<Context> context,
    Local<Name> key){LWNODE_RETURN_MAYBE(PropertyAttribute)}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
                                                   Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
    Local<Context> context,
    Local<Name> key){LWNODE_RETURN_MAYBE(PropertyAttribute)}

Local<v8::Object> v8::Object::Clone() {
  LWNODE_RETURN_LOCAL(Object);
}

Local<v8::Context> v8::Object::CreationContext() {
  LWNODE_RETURN_LOCAL(Context);
}

int v8::Object::GetIdentityHash() {
  LWNODE_RETURN_0;
}

bool v8::Object::IsCallable() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsConstructor() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsApiWrapper() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsUndetectable() {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<Value> Object::CallAsFunction(Local<Context> context,
                                         Local<Value> recv,
                                         int argc,
                                         Local<Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> Object::CallAsConstructor(Local<Context> context,
                                            int argc,
                                            Local<Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback,
                                   Local<Value> data,
                                   int length,
                                   ConstructorBehavior behavior,
                                   SideEffectType side_effect_type) {
  LWNODE_RETURN_LOCAL(Function);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context,
                                         int argc,
                                         v8::Local<v8::Value> argv[]) const {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<Object> Function::NewInstanceWithSideEffectType(
    Local<Context> context,
    int argc,
    v8::Local<v8::Value> argv[],
    SideEffectType side_effect_type) const {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
                                     v8::Local<v8::Value> recv,
                                     int argc,
                                     v8::Local<v8::Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

void Function::SetName(v8::Local<v8::String> name) {}

Local<Value> Function::GetName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetInferredName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetDebugName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetDisplayName() const {
  LWNODE_RETURN_LOCAL(Value);
}

ScriptOrigin Function::GetScriptOrigin() const {
  LWNODE_UNIMPLEMENT;
  return ScriptOrigin(Local<Value>());
}

const int Function::kLineOffsetNotFound = -1;

int Function::GetScriptLineNumber() const {
  LWNODE_RETURN_0;
}

int Function::GetScriptColumnNumber() const {
  LWNODE_RETURN_0;
}

int Function::ScriptId() const {
  LWNODE_RETURN_0;
}

Local<v8::Value> Function::GetBoundFunction() const {
  LWNODE_RETURN_LOCAL(Value);
}

int Name::GetIdentityHash() {
  LWNODE_RETURN_0;
}

int String::Length() const {
  auto esString = CVAL(this)->value()->asString();
  return esString->length();
}

bool String::IsOneByte() const {
  return ContainsOnlyOneByte();
}

bool String::ContainsOnlyOneByte() const {
  auto esString = CVAL(this)->value()->asString();
  return esString->has8BitContent();
}

int String::Utf8Length(Isolate* isolate) const {
  auto esString = CVAL(this)->value()->asString();
  if (esString->has8BitContent()) {
    return esString->length();
  } else {
    return esString->toStdUTF8String().length();
  }
}

int String::WriteUtf8(Isolate* v8_isolate,
                      char* buffer,
                      int capacity,
                      int* nchars_ref,
                      int options) const {
  LWNODE_RETURN_0;
}

int String::WriteOneByte(Isolate* isolate,
                         uint8_t* buffer,
                         int start,
                         int length,
                         int options) const {
  auto esString = CVAL(this)->value()->asString();
  auto bufferData = esString->stringBufferAccessData();

  if (!buffer || length == 0 ||
      static_cast<size_t>(start) > bufferData.length) {
    return 0;
  }

  int capacity = length;
  if (length < 0) {
    capacity = bufferData.length + 1;
  }

  int count = std::min(capacity, static_cast<int>(bufferData.length) - start);
  memcpy(buffer,
         reinterpret_cast<const uint8_t*>(bufferData.buffer) + start,
         count);

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  if (writeNull && (count < capacity)) {
    buffer[count] = '\0';
  }

  return count;
}

int String::Write(Isolate* isolate,
                  uint16_t* buffer,
                  int start,
                  int length,
                  int options) const {
  auto esString = CVAL(this)->value()->asString();
  auto bufferData = esString->stringBufferAccessData();

  if (!buffer || length == 0 ||
      static_cast<size_t>(start) > bufferData.length) {
    return 0;
  }

  int capacity = length;
  if (length < 0) {
    capacity = bufferData.length + 1;
  }

  int count = std::min(capacity, static_cast<int>(bufferData.length) - start);
  if (bufferData.has8BitContent) {
    int i = 0;
    for (int j = start; j < start + count; j++) {
      buffer[i] = bufferData.uncheckedCharAtFor8Bit(j);
      i++;
    }
  } else {
    memcpy(buffer,
           reinterpret_cast<const uint16_t*>(bufferData.buffer) + start,
           count);
  }

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  if (writeNull && (count < capacity)) {
    buffer[count] = '\0';
  }

  return count;
}

bool v8::String::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

bool v8::String::IsExternalOneByte() const {
  LWNODE_RETURN_FALSE;
}

void v8::String::VerifyExternalStringResource(
    v8::String::ExternalStringResource* value) const {}

void v8::String::VerifyExternalStringResourceBase(
    v8::String::ExternalStringResourceBase* value, Encoding encoding) const {}

String::ExternalStringResource* String::GetExternalStringResourceSlow() const {
  LWNODE_RETURN_NULLPTR;
}

String::ExternalStringResourceBase* String::GetExternalStringResourceBaseSlow(
    String::Encoding* encoding_out) const {
  LWNODE_RETURN_NULLPTR;
}

const v8::String::ExternalOneByteStringResource*
v8::String::GetExternalOneByteStringResource() const {
  LWNODE_RETURN_NULLPTR;
}

Local<Value> Symbol::Description() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Private::Name() const {
  LWNODE_RETURN_LOCAL(Value);
}

template <typename T, typename F>
static T getValue(ValueRef* esValue, F toValue) {
  auto lwContext = IsolateWrap::currentIsolate()->CurrentContext();
  LWNODE_CHECK(lwContext != nullptr);
  T v = 0;
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* esValue, T* v, F toValue)
          -> ValueRef* {
        *v = toValue(esValue, esState);
        return esValue;
      },
      esValue,
      &v,
      toValue);

  if (!r.isSuccessful()) {
    LWNODE_RETURN_0;  // TODO: handle error
  }

  return v;
}

double Number::Value() const {
  return getValue<double>(CVAL(this)->value(),
                          [](ValueRef* esValue, ExecutionStateRef* esState) {
                            return esValue->toNumber(esState);
                          });
}

bool Boolean::Value() const {
  LWNODE_RETURN_FALSE;
}

int64_t Integer::Value() const {
  return getValue<int64_t>(CVAL(this)->value(),
                           [](ValueRef* esValue, ExecutionStateRef* esState) {
                             return esValue->toNumber(esState);
                           });
}

int32_t Int32::Value() const {
  return getValue<int32_t>(CVAL(this)->value(),
                           [](ValueRef* esValue, ExecutionStateRef* esState) {
                             return esValue->toInt32(esState);
                           });
}

uint32_t Uint32::Value() const {
  return getValue<uint32_t>(CVAL(this)->value(),
                            [](ValueRef* esValue, ExecutionStateRef* esState) {
                              return esValue->toUint32(esState);
                            });
}

int v8::Object::InternalFieldCount() {
  LWNODE_RETURN_0;
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
  LWNODE_RETURN_LOCAL(Value);
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  LWNODE_RETURN_NULLPTR;
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {}

void v8::Object::SetAlignedPointerInInternalFields(int argc,
                                                   int indices[],
                                                   void* values[]) {}

// static void* ExternalValue(i::Object obj) {
//   // Obscure semantics for undefined, but somehow checked in our unit
//   tests... if (obj.IsUndefined()) {
//     return nullptr;
//   }
//   i::Object foreign = i::JSObject::cast(obj).GetEmbedderField(0);
//   return
//   reinterpret_cast<void*>(i::Foreign::cast(foreign).foreign_address());
// }
}  // namespace v8
