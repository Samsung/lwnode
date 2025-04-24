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

#include "api/global.h"
#include "api/utils.h"
#include "base.h"

#include <sstream>

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {
// --- D a t a ---

bool Value::FullIsUndefined() const {
  return CVAL(this)->value()->isUndefined();
}

// @lwnode: from v8.h
bool Value::QuickIsUndefined() const {
  return FullIsUndefined();
}

bool Value::FullIsNull() const {
  return CVAL(this)->value()->isNull();
}

// @lwnode: from v8.h
bool Value::QuickIsNull() const {
  return FullIsNull();
}

// @lwnode: from v8.h
bool Value::QuickIsNullOrUndefined() const {
  return FullIsNull() || FullIsUndefined();
}

bool Value::IsTrue() const {
  return CVAL(this)->value()->isTrue();
}

bool Value::IsFalse() const {
  return CVAL(this)->value()->isFalse();
}

bool Value::IsFunction() const {
  return CVAL(this)->value()->isCallable();
}

bool Value::IsName() const {
  auto esSelf = CVAL(this)->value();
  return esSelf->isString() || esSelf->isSymbol();
}

bool Value::FullIsString() const {
  return CVAL(this)->value()->isString();
}

bool Value::QuickIsString() const {
  return FullIsString();
}

bool Value::IsSymbol() const {
  return CVAL(this)->value()->isSymbol();
}

bool Value::IsArray() const {
  return CVAL(this)->value()->isArrayObject();
}

bool Value::IsArrayBuffer() const {
  return CVAL(this)->value()->isArrayBufferObject();
}

bool Value::IsArrayBufferView() const {
  return CVAL(this)->value()->isArrayBufferView();
}

bool Value::IsTypedArray() const {
  return CVAL(this)->value()->isTypedArrayObject();
}

#define VALUE_IS_TYPED_ARRAY(Type, typeName, TYPE, ctype)                      \
  bool Value::Is##Type##Array() const {                                        \
    return CVAL(this)->value()->is##Type##ArrayObject();                       \
  }

TYPED_ARRAYS(VALUE_IS_TYPED_ARRAY)

#undef VALUE_IS_TYPED_ARRAY

bool Value::IsDataView() const {
  return CVAL(this)->value()->isDataViewObject();
}

bool Value::IsSharedArrayBuffer() const {
  return CVAL(this)->value()->isSharedArrayBufferObject();
}

bool Value::IsObject() const {
  return CVAL(this)->value()->isObject();
}

bool Value::IsNumber() const {
  return CVAL(this)->value()->isNumber();
}

bool Value::IsBigInt() const {
  return CVAL(this)->value()->isBigInt();
}

bool Value::IsProxy() const {
  return CVAL(this)->value()->isProxyObject();
}

#define VALUE_IS_SPECIFIC_TYPE(Type, Check)                                    \
  bool Value::Is##Type() const { return CVAL(this)->value()->is##Type(); }

VALUE_IS_SPECIFIC_TYPE(ArgumentsObject, JSArgumentsObject)
VALUE_IS_SPECIFIC_TYPE(BigIntObject, BigIntWrapper)
VALUE_IS_SPECIFIC_TYPE(BooleanObject, BooleanWrapper)
VALUE_IS_SPECIFIC_TYPE(NumberObject, NumberWrapper)
VALUE_IS_SPECIFIC_TYPE(StringObject, StringWrapper)
VALUE_IS_SPECIFIC_TYPE(SymbolObject, SymbolWrapper)
#undef VALUE_IS_SPECIFIC_TYPE

#define VALUE_IS_SPECIFIC_TYPE(Type, Check)                                    \
  bool Value::Is##Type() const {                                               \
    return CVAL(this)->value()->is##Type##Object();                            \
  }

VALUE_IS_SPECIFIC_TYPE(Date, JSDate)
VALUE_IS_SPECIFIC_TYPE(Map, JSMap)
VALUE_IS_SPECIFIC_TYPE(Set, JSSet)
VALUE_IS_SPECIFIC_TYPE(WeakMap, JSWeakMap)
VALUE_IS_SPECIFIC_TYPE(WeakSet, JSWeakSet)
#undef VALUE_IS_SPECIFIC_TYPE

#define VALUE_IS_SPECIFIC_TYPE(Type, Check)                                    \
  bool Value::Is##Type() const { LWNODE_RETURN_FALSE; }

VALUE_IS_SPECIFIC_TYPE(WasmModuleObject, WasmModuleObject)
#undef VALUE_IS_SPECIFIC_TYPE

bool Value::IsBoolean() const {
  return CVAL(this)->value()->isBoolean();
}

bool Value::IsExternal() const {
  auto esSelf = CVAL(this)->value();
  if (!esSelf->isObject()) {
    return false;
  }
  auto data = ObjectRefHelper::getExtraData(esSelf->asObject());
  return data && data->isExternalObjectData();
}

bool Value::IsInt32() const {
  auto esSelf = CVAL(this)->value();

  if (esSelf->isInt32()) {
    return true;
  } else if (esSelf->isNumber()) {
    double value = esSelf->asNumber();
    bool isInRange = false;
    if (std::numeric_limits<int32_t>::min() <= value &&
        value <= std::numeric_limits<int32_t>::max()) {
      isInRange = true;
    }
    return isInteger(esSelf) && isInRange;
  }

  return false;
}

bool Value::IsUint32() const {
  auto esSelf = CVAL(this)->value();

  if (esSelf->isUInt32()) {
    return true;
  } else if (esSelf->isNumber()) {
    double value = esSelf->asNumber();
    bool isInRange = false;
    if (0 <= value && value <= std::numeric_limits<uint32_t>::max()) {
      isInRange = true;
    }
    return isInteger(esSelf) && isInRange;
  }

  return false;
}

bool Value::IsNativeError() const {
  return CVAL(this)->value()->isErrorObject();
}

bool Value::IsRegExp() const {
  return CVAL(this)->value()->isRegExpObject();
}

bool Value::IsAsyncFunction() const {
  auto esSelf = CVAL(this)->value();

  if (esSelf->isFunctionObject() == false) {
    return false;
  }

  auto esPureContext =
      ContextRef::create(IsolateWrap::GetCurrent()->vmInstance());
  auto esString =
      ObjectRefHelper::getConstructorName(esPureContext, esSelf->asObject());

  return StringRefHelper::equalsWithASCIIString(esString, "AsyncFunction");
}

bool Value::IsGeneratorFunction() const {
  return CVAL(this)->value()->isGeneratorFunctionObject();
}

bool Value::IsGeneratorObject() const {
  return CVAL(this)->value()->isGeneratorObject();
}

bool Value::IsMapIterator() const {
  return CVAL(this)->value()->isMapIteratorObject();
}

bool Value::IsSetIterator() const {
  return CVAL(this)->value()->isSetIteratorObject();
}

bool Value::IsPromise() const {
  return CVAL(this)->value()->isPromiseObject();
}

bool Value::IsModuleNamespaceObject() const {
  return CVAL(this)->value()->isModuleNamespaceObject();
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<String>());

  auto esContext = VAL(*context)->context()->get();

  auto esSelf = CVAL(this)->value();
  if (esSelf->isString()) {
    return Utils::NewLocal<String>(lwIsolate->toV8(), esSelf);
  }

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state, ValueRef* esSelf) -> ValueRef* {
        return esSelf->toString(state);
      },
      esSelf);

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<String>());

  return Utils::NewLocal<String>(lwIsolate->toV8(), r.result);
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  EsScope scope(context, this);

  if (scope.self()->isSymbol()) {
    auto str = scope.self()->asSymbol()->symbolDescriptiveString();
    return Utils::NewLocal<String>(scope.v8Isolate(), str);
  }

  auto esString = scope.self()->toStringWithoutException(scope.context());
  return Utils::NewLocal<String>(scope.v8Isolate(), esString);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Object>());
  auto esContext = VAL(*context)->context()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state, ValueRef* esSelf) -> ValueRef* {
        return esSelf->toObject(state);
      },
      CVAL(this)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Object>());

  return Utils::NewLocal<Object>(lwIsolate->toV8(), r.result);
}

MaybeLocal<BigInt> Value::ToBigInt(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(BigInt)
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  return ToBoolean(v8_isolate)->Value();
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  API_ENTER(v8_isolate, Local<Boolean>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* esSelf) -> ValueRef* {
        return ValueRef::create(esSelf->toBoolean(esState));
      },
      CVAL(this)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Local<Boolean>());

  return Utils::NewLocal<Boolean>(v8_isolate, r.result);
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Number>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* esSelf) -> ValueRef* {
        return ValueRef::create(esSelf->toNumber(esState));
      },
      CVAL(this)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Local<Number>());

  return Utils::NewLocal<Number>(lwIsolate->toV8(), r.result);
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Integer>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* esSelf) -> ValueRef* {
        return ValueRef::create(esSelf->toInteger(esState));
      },
      CVAL(this)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Local<Integer>());

  return Utils::NewLocal<Integer>(lwIsolate->toV8(), r.result);
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Int32>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* esSelf) -> ValueRef* {
        return ValueRef::create(esSelf->toInt32(esState));
      },
      CVAL(this)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Local<Int32>());

  return Utils::NewLocal<Int32>(lwIsolate->toV8(), r.result);
}

MaybeLocal<Uint32> Value::ToUint32(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Uint32>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* esSelf) -> ValueRef* {
        return ValueRef::create(esSelf->toUint32(esState));
      },
      CVAL(this)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Local<Uint32>());

  return Utils::NewLocal<Uint32>(lwIsolate->toV8(), r.result);
}

i::Isolate* i::IsolateFromNeverReadOnlySpaceObject(i::Address obj) {
  LWNODE_RETURN_NULLPTR;
}

bool i::ShouldThrowOnError(i::Isolate* isolate) {
  LWNODE_RETURN_FALSE;
}

void i::Internals::CheckInitializedImpl(v8::Isolate* external_isolate) {
  i::Isolate* isolate = reinterpret_cast<i::Isolate*>(external_isolate);
  LWNODE_CHECK_NOT_NULL(isolate);
}

void External::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsExternal());
}

void v8::Object::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsObject());
}

void v8::Function::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsFunction());
}

void v8::Boolean::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsBoolean());
}

void v8::Name::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsName());
}

void v8::String::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsString());
}

void v8::Symbol::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsSymbol());
}

void v8::Private::CheckCast(v8::Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::Number::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsNumber());
}

void v8::Integer::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsInt32() || that->IsUint32());
}

void v8::Int32::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsInt32());
}

void v8::Uint32::CheckCast(v8::Value* that) {
  LWNODE_CHECK(that->IsUint32());
}

void v8::BigInt::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Array::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsArray());
}

void v8::Map::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsMap());
}

void v8::Set::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsSet());
}

void v8::Promise::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsPromise());
}

void v8::Promise::Resolver::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsPromise());
}

void v8::Proxy::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsProxy());
}

void v8::WasmModuleObject::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

// void v8::debug::AccessorPair::CheckCast(Value* that) {}

// void v8::debug::WasmValue::CheckCast(Value* that) {}

v8::BackingStore::~BackingStore() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  LWNODE_CHECK(lwIsolate != nullptr);
  lwIsolate->removeBackingStore(reinterpret_cast<BackingStoreRef*>(this));
}

void v8::BackingStore::operator delete(void* p) {
  // Ignore all implicit delete by unique_ptr/shared_ptr
}

void* v8::BackingStore::Data() const {
  auto self = const_cast<BackingStoreRef*>(
      reinterpret_cast<const BackingStoreRef*>(this));
  return self->data();
}

size_t v8::BackingStore::ByteLength() const {
  auto self = const_cast<BackingStoreRef*>(
      reinterpret_cast<const BackingStoreRef*>(this));
  return self->byteLength();
}

bool v8::BackingStore::IsShared() const {
  auto self = const_cast<BackingStoreRef*>(
      reinterpret_cast<const BackingStoreRef*>(this));
  return self->isShared();
}

// static
std::unique_ptr<v8::BackingStore> v8::BackingStore::Reallocate(
    v8::Isolate* isolate,
    std::unique_ptr<v8::BackingStore> backing_store,
    size_t byte_length) {
  auto self = reinterpret_cast<BackingStoreRef*>(backing_store.get());
  self->reallocate(byte_length);
  return backing_store;
}

// static
void v8::BackingStore::EmptyDeleter(void* data,
                                    size_t length,
                                    void* deleter_data) {
  LWNODE_RETURN_VOID;
}

std::shared_ptr<v8::BackingStore> v8::ArrayBuffer::GetBackingStore() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esSelf = CVAL(this)->value()->asArrayBufferObject();

  BackingStoreRef* esBackingStore = nullptr;
  if (esSelf->backingStore().hasValue()) {
    esBackingStore = esSelf->backingStore().value();
  } else {
    esBackingStore = BackingStoreRef::createDefaultNonSharedBackingStore(0);
  }

  lwIsolate->addBackingStore(esBackingStore);
  return std::shared_ptr<v8::BackingStore>(
      reinterpret_cast<v8::BackingStore*>(esBackingStore));
}

std::shared_ptr<v8::BackingStore> v8::SharedArrayBuffer::GetBackingStore() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esSelf = CVAL(this)->value()->asSharedArrayBufferObject();

  BackingStoreRef* esBackingStore = nullptr;
  if (esSelf->backingStore().hasValue()) {
    esBackingStore = esSelf->backingStore().value();
  } else {
    esBackingStore = BackingStoreRef::createDefaultNonSharedBackingStore(0);
  }

  lwIsolate->addBackingStore(esBackingStore);
  return std::shared_ptr<v8::BackingStore>(
      reinterpret_cast<v8::BackingStore*>(esBackingStore));
}

void v8::ArrayBuffer::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsArrayBuffer());
}

void v8::ArrayBufferView::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsArrayBufferView());
}

constexpr size_t v8::TypedArray::kMaxLength;

void v8::TypedArray::CheckCast(Value* that) {
  LWNODE_CHECK(that->IsTypedArray());
}

#define CHECK_TYPED_ARRAY_CAST(Type, typeName, TYPE, ctype)                    \
  void v8::Type##Array::CheckCast(Value* that) {                               \
    LWNODE_CHECK(that->Is##Type##Array());                                     \
  }

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
  auto esSelf = CVAL(this)->value();
  if (esSelf->isNumber()) {
    return Just(esSelf->asNumber());
  }

  API_ENTER_WITH_CONTEXT(context, Nothing<double>());
  auto lwContext = lwIsolate->GetCurrentContext();
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self) {
        return ValueRef::create(self->toNumber(esState));
      },
      esSelf);
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<double>());

  return Just(r.result->asNumber());
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  auto esSelf = CVAL(this)->value();
  if (esSelf->isNumber()) {
    return Just(NumberToInt64(esSelf));
  }

  API_ENTER_WITH_CONTEXT(context, Nothing<int64_t>());
  auto lwContext = lwIsolate->GetCurrentContext();
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self) {
        return ValueRef::create(self->toInteger(esState));
      },
      CVAL(this)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<int64_t>());

  return Just(NumberToInt64(r.result));
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  auto self = CVAL(this)->value();
  if (self->isInt32()) {
    return Just(self->asInt32());
  }

  API_ENTER_WITH_CONTEXT(context, Nothing<int32_t>());
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self) {
        return ValueRef::create(self->toInt32(esState));
      },
      CVAL(this)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<int32_t>());

  return Just(r.result->asInt32());
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  auto self = CVAL(this)->value();
  if (self->isUInt32()) {
    return Just(self->asUInt32());
  }

  API_ENTER_WITH_CONTEXT(context, Nothing<uint32_t>());
  auto lwContext = lwIsolate->GetCurrentContext();

  uint32_t value = 0;
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self, uint32_t* value) {
        *value = self->toUint32(esState);
        return ValueRef::create(*value);
      },
      CVAL(this)->value(),
      &value);
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<uint32_t>());

  return Just(value);
}

MaybeLocal<Uint32> Value::ToArrayIndex(Local<Context> context) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Uint32>());
  auto lwContext = lwIsolate->GetCurrentContext();

  uint32_t index = ValueRef::InvalidIndex32Value;
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self, uint32_t* index) {
        *index = self->toIndex32(esState);
        return ValueRef::create(*index);
      },
      CVAL(this)->value(),
      &index);
  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Uint32>());

  if (index == ValueRef::InvalidIndex32Value) {
    return MaybeLocal<Uint32>();
  }

  return Utils::NewLocal<Uint32>(lwIsolate->toV8(), r.result);
}

Maybe<bool> Value::Equals(Local<Context> context, Local<Value> that) const {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState,
         ValueRef* self,
         ValueRef* that) -> ValueRef* {
        bool r = self->abstractEqualsTo(esState, that);
        return ValueRef::create(r);
      },
      CVAL(this)->value(),
      CVAL(*that)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

bool Value::StrictEquals(Local<Value> that) const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState,
         ValueRef* self,
         ValueRef* that) -> ValueRef* {
        return ValueRef::create(self->equalsTo(esState, that));
      },
      CVAL(this)->value(),
      CVAL(*that)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, false);

  return r.result->asBoolean();
}

bool Value::SameValue(Local<Value> that) const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();
  auto esThis = CVAL(this)->value();
  auto esThat = CVAL(*that)->value();

  if (esThis->isNumber()) {
    if (!esThat->isNumber()) {
      return false;
    }
    double a = esThis->asNumber();
    double b = esThat->asNumber();
    if (std::isnan(a) && std::isnan(b)) {
      return true;
    }
    if (std::isnan(a) || std::isnan(b)) {
      return false;
    }
    return a == b && std::signbit(a) == std::signbit(b);
  }

  return StrictEquals(that);
}

Local<String> Value::TypeOf(v8::Isolate* external_isolate) {
  API_ENTER_NO_EXCEPTION(external_isolate);
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* self) -> ValueRef* {
        std::string type;
        if (self->isUndefined()) {
          type = "undefined";
        } else if (self->isFunctionObject()) {
          type = "function";
        } else if (self->isObject() || self->isNull()) {
          type = "object";
        } else if (self->isString()) {
          type = "string";
        } else if (self->isNumber()) {
          type = "number";
        } else if (self->isBoolean()) {
          type = "boolean";
        } else {
          LWNODE_UNIMPLEMENT;
          type = "unknown";
        }

        return ValueRef::create(
            StringRef::createFromASCII(type.data(), type.length()));
      },
      CVAL(this)->value());

  LWNODE_CHECK(r.isSuccessful());

  return Utils::NewLocal<String>(lwIsolate->toV8(), r.result);
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> object) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState,
         ValueRef* self,
         ObjectRef* object) -> ValueRef* {
        return ValueRef::create(self->instanceOf(esState, object));
      },
      CVAL(this)->value(),
      CVAL(*object)->value()->asObject());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            v8::Local<Value> key,
                            v8::Local<Value> value) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::setProperty(VAL(*context)->context()->get(),
                                        VAL(this)->value()->asObject(),
                                        VAL(*key)->value(),
                                        VAL(*value)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(true);
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            uint32_t index,
                            v8::Local<Value> value) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  return Set(context,
             Utils::NewLocal<Value>(lwIsolate->toV8(), ValueRef::create(index)),
             value);
}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           v8::Local<Name> key,
                                           v8::Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           uint32_t index,
                                           v8::Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool);
}

PropertyDescriptor::PrivateData::PrivateData(Escargot::ValueRef* value) {
  descriptor_ = new Escargot::ObjectPropertyDescriptorRef(value);
}

PropertyDescriptor::PrivateData::PrivateData(Escargot::ValueRef* value,
                                             bool writable) {
  descriptor_ = new Escargot::ObjectPropertyDescriptorRef(value, writable);
}

PropertyDescriptor::PrivateData::PrivateData(Escargot::ValueRef* get,
                                             Escargot::ValueRef* set) {
  descriptor_ = new Escargot::ObjectPropertyDescriptorRef(get, set);
}

PropertyDescriptor::PrivateData::~PrivateData() {
  if (!isExternalDescriptor_) {
    delete descriptor_;
    descriptor_ = nullptr;
  }
}

void PropertyDescriptor::PrivateData::setDescriptor(
    Escargot::ObjectPropertyDescriptorRef* descriptor) {
  LWNODE_CHECK_NULL(descriptor_);
  descriptor_ = descriptor;
  isExternalDescriptor_ = true;
}

v8::PropertyDescriptor::PropertyDescriptor() : private_(new PrivateData()) {}

// DataDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value) {
  LWNODE_CHECK_MSG(!value.IsEmpty(), "not supported!");
  private_ = new PrivateData(VAL(*value)->value());
}

// DataDescriptor with writable field
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
                                           bool writable) {
  LWNODE_CHECK_MSG(!value.IsEmpty(), "not supported!");
  private_ = new PrivateData(VAL(*value)->value(), writable);
}

// AccessorDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
                                           v8::Local<v8::Value> set) {
  LWNODE_CHECK_MSG(!get.IsEmpty(), "not supported!");
  LWNODE_CHECK_MSG(!set.IsEmpty(), "not supported!");
  private_ = new PrivateData(VAL(*get)->value(), VAL(*set)->value());
}

v8::PropertyDescriptor::~PropertyDescriptor() {
  delete private_;
  private_ = nullptr;
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return Local<Value>();
  }
  return Utils::NewLocal<Value>(IsolateWrap::GetCurrent()->toV8(),
                                private_->descriptor()->value());
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return Local<Value>();
  }
  return Utils::NewLocal<Value>(IsolateWrap::GetCurrent()->toV8(),
                                private_->descriptor()->getter());
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return Local<Value>();
  }
  return Utils::NewLocal<Value>(IsolateWrap::GetCurrent()->toV8(),
                                private_->descriptor()->setter());
}

bool v8::PropertyDescriptor::has_value() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasValue();
}

bool v8::PropertyDescriptor::has_get() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasGetter();
}

bool v8::PropertyDescriptor::has_set() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasSetter();
}

bool v8::PropertyDescriptor::writable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->isWritable();
}

bool v8::PropertyDescriptor::has_writable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasWritable();
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return;
  }
  return private_->descriptor()->setEnumerable(enumerable);
}

bool v8::PropertyDescriptor::enumerable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->isEnumerable();
}

bool v8::PropertyDescriptor::has_enumerable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasEnumerable();
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return;
  }
  return private_->descriptor()->setConfigurable(configurable);
}

bool v8::PropertyDescriptor::configurable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->isConfigurable();
}

bool v8::PropertyDescriptor::has_configurable() const {
  LWNODE_DCHECK_NOT_NULL(private_);
  if (!private_->descriptor()) {
    return false;
  }
  return private_->descriptor()->hasConfigurable();
}

Maybe<bool> v8::Object::DefineOwnProperty(v8::Local<v8::Context> context,
                                          v8::Local<Name> key,
                                          v8::Local<Value> value,
                                          v8::PropertyAttribute attributes) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::defineDataProperty(
      CVAL(*context)->context()->get(),
      CVAL(this)->value()->asObject(),
      CVAL(*key)->value(),
      ObjectRef::DataPropertyDescriptor(
          CVAL(*value)->value(), V8Helper::toPresentAttribute(attributes)));

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esObject = CVAL(this)->value()->asObject();
  auto esKey = CVAL(*key)->value();

  ObjectRef::PresentAttribute attribute =
      ObjectRef::PresentAttribute::NotPresent;

  if (descriptor.has_writable()) {
    if (descriptor.writable()) {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::WritablePresent);
    } else {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::NonWritablePresent);
    }
  }

  if (descriptor.has_enumerable()) {
    if (descriptor.enumerable()) {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::EnumerablePresent);
    } else {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::NonEnumerablePresent);
    }
  }

  if (descriptor.has_configurable()) {
    if (descriptor.enumerable()) {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::ConfigurablePresent);
    } else {
      attribute = static_cast<ObjectRef::PresentAttribute>(
          attribute | ObjectRef::PresentAttribute::NonConfigurablePresent);
    }
  }

  if (descriptor.has_value()) {
    ObjectRef::DataPropertyDescriptor dataDescriptor(
        CVAL(*descriptor.value())->value(), attribute);
    auto r = ObjectRefHelper::defineDataProperty(
        esContext, esObject, esKey, dataDescriptor);
    API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());
    return Just(r.result->asBoolean());
  } else if (descriptor.has_get()) {
    ValueRef* setter = nullptr;
    if (descriptor.has_set()) {
      setter = CVAL(*descriptor.set())->value();
    }
    ObjectRef::AccessorPropertyDescriptor accessorDescriptor(
        CVAL(*descriptor.get())->value(), setter, attribute);
    auto r = ObjectRefHelper::defineAccessorProperty(
        esContext, esObject, esKey, accessorDescriptor);
    API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());
    return Just(r.result->asBoolean());
  } else {
    ObjectRef::DataPropertyDescriptor dataDescriptor(
        ValueRef::createUndefined(), attribute);
    auto r = ObjectRefHelper::defineDataProperty(
        esContext, esObject, esKey, dataDescriptor);
    API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());
    return Just(r.result->asBoolean());
  }
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context,
                                   Local<Private> key,
                                   Local<Value> value) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  EvalResult r = ObjectRefHelper::setPrivate(VAL(*context)->context()->get(),
                                             lwIsolate->privateValuesSymbol(),
                                             VAL(this)->value()->asObject(),
                                             VAL(*key)->value(),
                                             VAL(*value)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(true);
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
                                  Local<Value> key) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());

  auto r = ObjectRefHelper::getProperty(VAL(*context)->context()->get(),
                                        VAL(this)->value()->asObject(),
                                        VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  return Utils::NewLocal<Value>(lwIsolate->toV8(), r.result);
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());
  return Get(
      context,
      Utils::NewLocal<Value>(lwIsolate->toV8(), ValueRef::create(index)));
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());

  EvalResult r = ObjectRefHelper::getPrivate(VAL(*context)->context()->get(),
                                             lwIsolate->privateValuesSymbol(),
                                             VAL(this)->value()->asObject(),
                                             VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  return Utils::NewLocal<Value>(lwIsolate->toV8(), r.result);
}

Maybe<PropertyAttribute> v8::Object::GetPropertyAttributes(
    Local<Context> context, Local<Value> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<PropertyAttribute>());
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esSelf = CVAL(this)->value()->asObject();
  auto esKey = CVAL(*key)->value();

  auto r = ObjectRefHelper::getPropertyAttributes(esContext, esSelf, esKey);
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<PropertyAttribute>());

  if (r.result->isUndefined()) {
    return Just(PropertyAttribute::None);
  }

  PropertyAttribute attribute =
      static_cast<PropertyAttribute>(r.result->asUInt32());
  return Just(attribute);
}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                       Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> v8::Object::GetPrototype() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esObject =
      ObjectRefHelper::getPrototype(esContext, VAL(this)->value()->asObject());

  return Utils::NewLocal<Value>(lwIsolate->toV8(), esObject);
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
  return GetPropertyNames(
      context,
      v8::KeyCollectionMode::kIncludePrototypes,
      static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS),
      v8::IndexFilter::kIncludeIndices);
}

static void pushUniqueValue(SetObjectRef* set,
                            GCVector<ValueRef*>* vector,
                            ValueRef* value,
                            ExecutionStateRef* state) {
  if (!set->has(state, value)) {
    vector->push_back(value);
    set->add(state, value);
  }
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context,
    KeyCollectionMode mode,
    PropertyFilter property_filter,
    IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Array>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state,
         ObjectRef* esObject,
         KeyCollectionMode mode,
         PropertyFilter property_filter,
         IndexFilter index_filter,
         KeyConversionMode key_conversion) -> ValueRef* {
        auto propertyNameVector = ValueVectorRef::create();
        auto propertyNameSet = SetObjectRef::create(state);

        ObjectRef* esSelf = esObject;

        while (true) {
          GCVector<ValueRef*> indexes;
          GCVector<ValueRef*> strings;
          GCVector<ValueRef*> symbols;
          esSelf->enumerateObjectOwnProperties(
              state,
              [&propertyNameSet,
               &indexes,
               &strings,
               &symbols,
               property_filter,
               index_filter](ExecutionStateRef* state,
                             ValueRef* propertyName,
                             bool isWritable,
                             bool isEnumerable,
                             bool isConfigurable) -> bool {
                if (propertyName->isSymbol()) {
                  if (!(property_filter & PropertyFilter::SKIP_SYMBOLS)) {
                    pushUniqueValue(
                        propertyNameSet, &symbols, propertyName, state);
                  }
                  return true;
                }

                if (property_filter & (PropertyFilter::ONLY_WRITABLE |
                                       PropertyFilter::ONLY_ENUMERABLE |
                                       PropertyFilter::ONLY_CONFIGURABLE)) {
                  if (((property_filter & PropertyFilter::ONLY_WRITABLE) &&
                       !isWritable) ||
                      ((property_filter & PropertyFilter::ONLY_ENUMERABLE) &&
                       !isEnumerable) ||
                      ((property_filter & PropertyFilter::ONLY_CONFIGURABLE) &&
                       !isConfigurable)) {
                    return true;
                  }
                }

                uint32_t index = propertyName->tryToUseAsIndexProperty(state);
                if (index == ValueRef::InvalidIndex32Value) {
                  if (!(property_filter & PropertyFilter::SKIP_STRINGS)) {
                    pushUniqueValue(
                        propertyNameSet, &strings, propertyName, state);
                    return true;
                  }
                } else {
                  if (index_filter != IndexFilter::kSkipIndices) {
                    pushUniqueValue(
                        propertyNameSet, &indexes, propertyName, state);
                  }
                }
                return true;
              },
              false);

          if (index_filter != IndexFilter::kSkipIndices) {
            std::sort(indexes.begin(),
                      indexes.end(),
                      [&state](ValueRef* a, ValueRef* b) -> bool {
                        return a->toUint32(state) < b->toUint32(state);
                      });
            if (key_conversion == KeyConversionMode::kConvertToString) {
              for (auto& index : indexes) {
                propertyNameVector->pushBack(index->toString(state));
              }
            } else {
              for (auto& index : indexes) {
                propertyNameVector->pushBack(index);
              }
            }
          }

          if (!(property_filter & PropertyFilter::SKIP_STRINGS)) {
            for (auto& string : strings) {
              propertyNameVector->pushBack(string);
            }
          }

          if (!(property_filter & PropertyFilter::SKIP_SYMBOLS)) {
            for (auto& symbol : symbols) {
              propertyNameVector->pushBack(symbol);
            }
          }

          if (mode == KeyCollectionMode::kOwnOnly) {
            break;
          }
          auto prototype = esSelf->getPrototypeObject(state);
          if (!prototype.hasValue()) {
            break;
          }
          esSelf = prototype.get();
        }
        return ArrayObjectRef::create(state, propertyNameVector);
      },
      CVAL(this)->value()->asObject(),
      mode,
      property_filter,
      index_filter,
      key_conversion);

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Array>());

  return Utils::NewLocal<Array>(lwIsolate->toV8(), r.result);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  return GetOwnPropertyNames(
      context, static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS));
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(
    Local<Context> context,
    PropertyFilter filter,
    KeyConversionMode key_conversion) {
  return GetPropertyNames(context,
                          KeyCollectionMode::kOwnOnly,
                          filter,
                          v8::IndexFilter::kIncludeIndices,
                          key_conversion);
}

MaybeLocal<String> v8::Object::ObjectProtoToString(Local<Context> context) {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> v8::Object::GetConstructorName() {
  LWNODE_CALL_TRACE();

  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esObject = CVAL(this)->value()->asObject();
  auto esString = ObjectRefHelper::getConstructorName(esContext, esObject);
  return Utils::NewLocal<String>(lwIsolate->toV8(), esString);
}

Maybe<bool> v8::Object::SetIntegrityLevel(Local<Context> context,
                                          IntegrityLevel level) {
// Note: Debugger and setIntegrityLevel cannot be used together
#if defined(LWNODE_ENABLE_DEBUGGER)
  return Just(true);
#else
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState,
         ObjectRef* esSelf,
         bool isSealed) -> ValueRef* {
        return ValueRef::create(esSelf->setIntegrityLevel(esState, isSealed));
      },
      CVAL(this)->value()->asObject(),
      level == IntegrityLevel::kSealed);
// TODO: Fail to execute SetIntegrityLevel because the property assigned with
// SetAccessor has special attribute.
// The attribute is data property, configurable and writable.
#if 0
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());
#else
  if (!r.isSuccessful()) {
    LWNODE_DLOG_ERROR("Fail to execute 'setIntegrityLevel'");
    return Just(true);
  }
#endif
  return Just(r.result->asBoolean());
#endif
}

Maybe<bool> v8::Object::Delete(Local<Context> context, Local<Value> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::deleteProperty(VAL(*context)->context()->get(),
                                           VAL(this)->value()->asObject(),
                                           VAL(*key)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r =
      ObjectRefHelper::deletePrivateProperty(VAL(*context)->context()->get(),
                                             lwIsolate->privateValuesSymbol(),
                                             VAL(this)->value()->asObject(),
                                             VAL(*key)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::Has(Local<Context> context, Local<Value> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::hasProperty(VAL(*context)->context()->get(),
                                        VAL(this)->value()->asObject(),
                                        VAL(*key)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context, Local<Private> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  EvalResult r = ObjectRefHelper::getPrivate(VAL(*context)->context()->get(),
                                             lwIsolate->privateValuesSymbol(),
                                             VAL(this)->value()->asObject(),
                                             VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  if (r.result->isUndefined()) {
    return Just(false);
  }
  return Just(true);
}

Maybe<bool> v8::Object::Delete(Local<Context> context,
                               uint32_t index){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Has(Local<Context> context, uint32_t index) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  return Has(
      context,
      Utils::NewLocal<Value>(lwIsolate->toV8(), ValueRef::create(index)));
}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data,
                                AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type,
                                SideEffectType setter_side_effect_type) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());
  auto esSelf = CVAL(this)->value()->asObject();

  Local<Value> dataLocalValue;
  if (!data.ToLocal(&dataLocalValue)) {
    dataLocalValue =
        Utils::NewLocal<Value>(lwIsolate->toV8(), lwIsolate->undefined_value());
  }

  return ObjectUtils::SetAccessor(
      esSelf, lwIsolate, name, getter, setter, dataLocalValue, attribute);
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
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::hasOwnProperty(VAL(*context)->context()->get(),
                                           VAL(this)->value()->asObject(),
                                           VAL(*key)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context, uint32_t index) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::hasOwnProperty(VAL(*context)->context()->get(),
                                           VAL(this)->value()->asObject(),
                                           ValueRef::create(index));
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  return Just(r.result->asBoolean());
}

Maybe<bool> v8::Object::HasRealNamedProperty(Local<Context> context,
                                             Local<Name> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::getOwnProperty(VAL(*context)->context()->get(),
                                           VAL(this)->value()->asObject(),
                                           VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  if (r.result->isUndefined()) {
    return Just(false);
  }

  return Just(true);
}

Maybe<bool> v8::Object::HasRealIndexedProperty(Local<Context> context,
                                               uint32_t index) {
  API_ENTER_WITH_CONTEXT(context, Nothing<bool>());

  auto r = ObjectRefHelper::getOwnIndexedProperty(
      VAL(*context)->context()->get(), VAL(this)->value()->asObject(), index);

  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<bool>());

  if (r.result->isUndefined()) {
    return Just(false);
  }

  return Just(true);
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
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());

  auto r = ObjectRefHelper::getOwnProperty(VAL(*context)->context()->get(),
                                           VAL(this)->value()->asObject(),
                                           VAL(*key)->value());

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  if (r.result->isUndefined()) {
    return MaybeLocal<Value>();
  }

  return Utils::NewLocal<Value>(lwIsolate->toV8(), r.result);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
    Local<Context> context, Local<Name> key) {
  API_ENTER_WITH_CONTEXT(context, Nothing<PropertyAttribute>());

  auto r =
      ObjectRefHelper::getPropertyAttributes(VAL(*context)->context()->get(),
                                             CVAL(this)->value()->asObject(),
                                             CVAL(*key)->value(),
                                             true);
  API_HANDLE_EXCEPTION(r, lwIsolate, Nothing<PropertyAttribute>());

  if (r.result->isUndefined()) {
    return Just(PropertyAttribute::None);
  }

  auto attribute =
      static_cast<ObjectRef::PresentAttribute>(r.result->asUInt32());
  return Just(V8Helper::toPropertyAttribute(attribute));
}

Local<v8::Object> v8::Object::Clone() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto esSelf = CVAL(this)->value()->asObject();

  auto r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ObjectRef* esSelf) -> ValueRef* {
        auto constructor =
            esSelf->get(esState, StringRef::createFromASCII("constructor"));
        ValueRef* args[] = {};
        auto esCloneObject =
            constructor->construct(esState, 0, args)->asObject();

        esSelf->enumerateObjectOwnProperties(
            esState,
            [esSelf, esCloneObject](ExecutionStateRef* esState,
                                    ValueRef* propertyName,
                                    bool isWritable,
                                    bool isEnumerable,
                                    bool isConfigurable) -> bool {
              auto prop = esSelf->getOwnProperty(esState, propertyName);
              esCloneObject->defineDataProperty(esState,
                                                propertyName,
                                                prop,
                                                isWritable,
                                                isEnumerable,
                                                isConfigurable);
              return true;
            });
        return esCloneObject;
      },
      esSelf);
  LWNODE_CHECK(r.isSuccessful());

  return v8::Utils::NewLocal<Object>(lwIsolate->toV8(), r.result);
}

Local<v8::Context> v8::Object::CreationContext() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esSelf = CVAL(this)->value()->asObject();

  auto esVal = esSelf->creationContext();
  if (!esVal.hasValue()) {
    return Local<v8::Context>();
  }

  auto lwCreationContext = ContextWrap::fromEscargot(esVal.get());

  return v8::Local<v8::Context>::New(
      lwIsolate->toV8(),
      reinterpret_cast<v8::Context*>(
          EscargotShim::ValueWrap::createContext(lwCreationContext)));
}

int v8::Object::GetIdentityHash() {
  LWNODE_RETURN_0;
}

bool v8::Object::IsCallable() {
  auto esSelf = CVAL(this)->value()->asObject();
  return esSelf->isCallable();
}

bool v8::Object::IsConstructor() {
  auto esSelf = CVAL(this)->value()->asObject();
  if (esSelf->isFunctionObject()) {
    return esSelf->asFunctionObject()->isConstructor();
  }

  return false;
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
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Function>());

  Local<FunctionTemplate> functionTemplate =
      FunctionTemplate::New(lwIsolate->toV8(),
                            callback,
                            data,
                            Local<Signature>(),
                            length,
                            behavior,
                            side_effect_type);

  return functionTemplate->GetFunction(context);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context,
                                         int argc,
                                         v8::Local<v8::Value> argv[]) const {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Object>());
  auto lwContext = lwIsolate->GetCurrentContext();
  auto esFunction = CVAL(this)->value()->asFunctionObject();
  LWNODE_CALL_TRACE_ID_LOG(
      EXTRADATA, "Function(%p)::NewInstance()", esFunction);

  GCVector<ValueRef*> arguments;
  for (int i = 0; i < argc; i++) {
    arguments.push_back(VAL(*argv[i])->value());
  }

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* state,
         ObjectRef* self,
         size_t argc,
         ValueRef** argv) -> ValueRef* {
        return self->construct(state, argc, argv);
      },
      CVAL(this)->value()->asObject(),
      arguments.size(),
      arguments.data());

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Object>());

  auto esNewObject = r.result->asObject();
  ObjectData* existingNewObjectExtraData = nullptr;
  if (ExtraDataHelper::getExtraData(esNewObject)) {
    // This objectData was added in ObjectTemplate::createObjectData();
    existingNewObjectExtraData =
        ExtraDataHelper::getExtraData(esNewObject)->asObjectData();
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "Function(%p)::NewInstance(): Existing extraData: %p",
        esFunction,
        existingNewObjectExtraData);
  }

  auto functionData = ExtraDataHelper::getExtraData(esFunction);
  if (functionData) {
    // This functionData was added in FunctionTemplate::GetFunction()
    LWNODE_CHECK(functionData->isFunctionData());
    LWNODE_CALL_TRACE_ID_LOG(
        EXTRADATA,
        "Function(%p)::NewInstance(): Existing functionData: %p\n",
        esFunction,
        functionData);

    if (existingNewObjectExtraData) {
      // esNewObject was created from ObjectTemplate. Add this esFunction info
      // so that we can trace it back later
      existingNewObjectExtraData->setFunctionObject(esFunction);
    } else {
      auto objectData = new ObjectData(esFunction);
      LWNODE_CALL_TRACE_ID_LOG(
          EXTRADATA,
          "Function(%p)::NewInstance(): New objectData1: %p",
          esFunction,
          objectData);
      ObjectRefHelper::setExtraData(esNewObject, objectData);
    }
  } else {
    // Function::NewInstance() was called in JavaScript
    LWNODE_CHECK(existingNewObjectExtraData == nullptr);
    auto objectData = new ObjectData(esFunction);
    LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                             "Function(%p)::NewInstance(): New objectData2: %p",
                             esFunction,
                             objectData);
    ObjectRefHelper::setExtraData(esNewObject, objectData);
  }

  return Utils::NewLocal<Object>(lwIsolate->toV8(), esNewObject);
}

MaybeLocal<Object> Function::NewInstanceWithSideEffectType(
    Local<Context> context,
    int argc,
    v8::Local<v8::Value> argv[],
    SideEffectType side_effect_type) const {
  LWNODE_RETURN_LOCAL(Object);
}

static inline std::string toStdStringWithoutException(ContextRef* context,
                                                      ValueRef* esValue) {
  return esValue->toStringWithoutException(context)->toStdUTF8String();
}

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
                                     v8::Local<v8::Value> recv,
                                     int argc,
                                     v8::Local<v8::Value> argv[]) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());
  LWNODE_CHECK(CVAL(this)->value()->isCallable());

  auto esContext = VAL(*context)->context()->get();

  GCVector<ValueRef*> arguments;
  for (int i = 0; i < argc; i++) {
    arguments.push_back(VAL(*argv[i])->value());
  }

  lwIsolate->increaseCallDepth();
  auto r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state,
         IsolateWrap* lwIsolate,
         ValueRef* self,
         ValueRef* receiver,
         const size_t argc,
         ValueRef** argv) -> ValueRef* {
        auto r = self->call(state, receiver, argc, argv);
        lwIsolate->ThrowErrorIfHasException(state);
        return r;
      },
      lwIsolate,
      CVAL(this)->value(),
      CVAL(*recv)->value(),
      arguments.size(),
      arguments.data());
  lwIsolate->decreaseCallDepth();

  if (!r.isSuccessful()) {
    LWNODE_DLOG_ERROR("Function::Call()");
    LWNODE_DLOG_RAW("Internal:\n  this: %p (es: %p)\n  recv: %p (es: %p)",
                    this,
                    CVAL(this)->value(),
                    *recv,
                    CVAL(*recv)->value());

    LWNODE_DLOG_RAW("  arguments (%d):", argc);
    for (int i = 0; i < argc; i++) {
      auto esValue = VAL(*argv[i])->value();
      LWNODE_DLOG_RAW("  [%d] %p (es: %p) %s",
                      i,
                      *argv[i],
                      esValue,
                      toStdStringWithoutException(esContext, esValue).c_str());
    }

    LWNODE_DLOG_RAW("Execute:\n  %s\nResource:\n  %s\n%s",
                    __CODE_LOCATION__,
                    "N/A",
                    EvalResultHelper::getErrorString(
                        lwIsolate->GetCurrentContext()->get(), r)
                        .c_str());

    if (EscargotShim::Global::flags()->isOn(
            Flag::Type::AbortOnUncaughtException)) {
      if (!lwIsolate->abortOnUncaughtExceptionCallback() ||
          lwIsolate->abortOnUncaughtExceptionCallback()(lwIsolate->toV8())) {
        LWNODE_DLOG_INFO("Abort because of uncaught exception callback!");
        abort();
      }
    }

    if (lwIsolate->hasCallDepth()) {
      lwIsolate->ScheduleThrow(r.error.get());
    } else {
      lwIsolate->SetPendingExceptionAndMessage(r.error.get(), r.stackTrace);
      lwIsolate->ReportPendingMessages();
    }
    return MaybeLocal<Value>();
  }

  if (lwIsolate->sholdReportPendingMessage(false)) {
    lwIsolate->ReportPendingMessages();
    return MaybeLocal<Value>();
  }

  return Utils::NewLocal<Value>(lwIsolate->toV8(), r.result);
}

void Function::SetName(v8::Local<v8::String> name) {
  auto r = Evaluator::execute(
      IsolateWrap::GetCurrent()->GetCurrentContext()->get(),
      [](ExecutionStateRef* esState,
         FunctionObjectRef* esFunction,
         StringRef* esName) -> ValueRef* {
        esFunction->setName(
            AtomicStringRef::create(esState->context(), esName));
        return ValueRef::createNull();
      },
      CVAL(this)->value()->asFunctionObject(),
      CVAL(*name)->value()->asString());
  LWNODE_CHECK(r.isSuccessful());
}

Local<Value> Function::GetName() const {
  auto lwIsolate = IsolateWrap::GetCurrent();

  auto r = Evaluator::execute(
      lwIsolate->GetCurrentContext()->get(),
      [](ExecutionStateRef* esState,
         FunctionObjectRef* esFunction) -> ValueRef* {
        return esFunction->getOwnProperty(esState,
                                          StringRef::createFromASCII("name"));
      },
      CVAL(this)->value()->asFunctionObject());
  LWNODE_CHECK(r.isSuccessful());

  return Utils::NewLocal<String>(lwIsolate->toV8(), r.result);
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
  if (StringRefHelper::isAsciiString(esString)) {
    return esString->length();
  } else {
    return esString->toStdUTF8String().length();
  }
}

static int readAUtf8Sequence(char* start, const char* end) {
  char byte1 = *start;
  if ((byte1 & 0x80) == 0) {
    return 1;
  }

  char byte2 = 0;
  char byte3 = 0;
  char byte4 = 0;
  for (int i = 1; i < 4 && start + i < end; i++) {
    if (i == 1) {
      byte2 = *(start + 1);
      if (((byte1 & 0xE0) == 0xC0) && ((byte2 & 0xC0) == 0x80)) {
        return 2;
      }
    } else if (i == 2) {
      byte3 = *(start + 2);
      if ((byte1 & 0xF0) == 0xE0 && (byte2 & 0xC0) == 0x80 &&
          (byte3 & 0xC0) == 0x80) {
        return 3;
      }
    } else if (i == 3) {
      byte4 = *(start + 3);
      if ((byte1 & 0xF8) == 0xF0 && (byte2 & 0xC0) == 0x80 &&
          (byte3 & 0xC0) == 0x80 && (byte4 & 0xC0) == 0x80) {
        return 4;
      }
    }
  }

  return -1;
}

static void copyUtf8Sequences(
    char* dest, const char* source, int maxBytes, int* nchars, int* nbytes) {
  char* s = const_cast<char*>(source);
  char* d = dest;
  for (int i = 0; i < maxBytes;) {
    int utf8CharLength = readAUtf8Sequence(s, source + maxBytes);
    if (utf8CharLength > 0) {
      memcpy(d, s, utf8CharLength);
      s += utf8CharLength;
      d += utf8CharLength;
      *nchars += 1;
      *nbytes += utf8CharLength;
      i += utf8CharLength;
    } else {
      // invalid char
      break;
    }
  }
}

int String::WriteUtf8(Isolate* v8_isolate,
                      char* buffer,
                      int capacity,  // nbytes
                      int* nchars_ref,
                      int options) const {
  int nchars = 0;
  int nbytes = 0;
  int bufferCapacity = capacity >= 0 ? capacity : v8::String::kMaxLength;
  int byteLength = 0;

  auto esString = CVAL(this)->value()->asString();
  auto bufferData = esString->stringBufferAccessData();

  if (StringRefHelper::isAsciiString(esString)) {
    byteLength = bufferData.length;
    int maxBytes = std::min(bufferCapacity, byteLength);
    memcpy(buffer, bufferData.buffer, maxBytes);
    nchars = nbytes = maxBytes;
  } else {
    std::string utf8Str = esString->toStdUTF8String(options);
    byteLength = utf8Str.length();
    int maxBytes = std::min(bufferCapacity, byteLength);
    copyUtf8Sequences(buffer, utf8Str.data(), maxBytes, &nchars, &nbytes);
  }

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  if (writeNull && (nbytes == byteLength) && (nbytes < bufferCapacity)) {
    buffer[nbytes] = '\0';
    nbytes++;
  }

  if (nchars_ref) {
    *nchars_ref = nchars;
  }

  return nbytes;
}

int String::WriteOneByte(Isolate* isolate,
                         uint8_t* buffer,
                         int start,
                         int length,
                         int options) const {
  auto esString = CVAL(this)->value()->asString();
  auto bufferData = esString->stringBufferAccessData();

  if (!buffer || length == 0) {
    return 0;
  }

  int bufferCapacity = length > 0 ? length : v8::String::kMaxLength;
  int nchars = 0;
  if (!esString->has8BitContent()) {
    // NOTE
    // The esString in this v8::String can contain a string encoded in UTF16
    // even though all characters can be encoded in one-byte.
    // This happens when:
    // 1. The source JS file is encoded in UTF16
    //    (due to non-one byte characters), and
    // 2. The esString represents a string literal in the source JS file.
    //
    // In this case, all characters in the esString are one-byte-representable,
    // but stored in uint16. V8 seems to use a one-byte string
    // if the string can be represented in one byte characters.
    // Hence, node.js assumes that when WriteOneByte() is called,
    // the string is a one-byte string.
    // To workaround, when this inconsistency happens, we check
    // whether the string is one-byte representable, and we write
    // back in one-byte string.
    StringRef* toOneByteStr = StringRef::createFromUTF8(
        reinterpret_cast<const char*>(bufferData.buffer),
        bufferData.length * 2);
    auto oneByteBufferData = toOneByteStr->stringBufferAccessData();

    if (static_cast<size_t>(start * 2) > oneByteBufferData.length) {
      return 0;
    }

    if (!toOneByteStr->has8BitContent()) {
      LWNODE_DLOG_WARN(
          "Incorrectly converting a UTF16 string to a 1 byte string");
    }

    size_t j = 0;
    for (size_t i = start * 2; i < oneByteBufferData.length; i += 2) {
      if (j > static_cast<size_t>(bufferCapacity)) {
        break;
      }

      buffer[j] = ((uint8_t*)(oneByteBufferData.buffer))[i];
      j++;
    }
    nchars = j;
  } else {
    if (static_cast<size_t>(start) > bufferData.length) {
      return 0;
    }

    nchars = std::min(static_cast<size_t>(bufferCapacity),
                      bufferData.length - start);
    memcpy(buffer,
           reinterpret_cast<const uint8_t*>(bufferData.buffer) + start,
           nchars);
  }

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  if (writeNull && (nchars < bufferCapacity)) {
    buffer[nchars] = '\0';
  }

  return nchars;
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

  int bufferCapacity = length >= 0 ? length : v8::String::kMaxLength;

  int nchars =
      std::min(bufferCapacity, static_cast<int>(bufferData.length) - start);
  if (bufferData.has8BitContent) {
    int i = 0;
    for (int j = start; j < start + nchars; j++) {
      buffer[i] = bufferData.uncheckedCharAtFor8Bit(j);
      i++;
    }
  } else {
    memcpy(buffer,
           reinterpret_cast<const uint16_t*>(bufferData.buffer) + start,
           nchars * 2);
  }

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  if (writeNull && (nchars < bufferCapacity)) {
    buffer[nchars] = '\0';
  }

  return nchars;
}

bool v8::String::IsExternal() const {
  return CVAL(this)->isExternalString();
}

bool v8::String::IsExternalOneByte() const {
  auto lwSelf = CVAL(this);
  return lwSelf->isExternalString() &&
         lwSelf->value()->asString()->has8BitContent();
}

void v8::String::VerifyExternalStringResource(
    v8::String::ExternalStringResource* value) const {
  LWNODE_RETURN_VOID;
}

void v8::String::VerifyExternalStringResourceBase(
    v8::String::ExternalStringResourceBase* value, Encoding encoding) const {
  LWNODE_RETURN_VOID;
}

String::ExternalStringResource* String::GetExternalStringResourceSlow() const {
  return reinterpret_cast<ExternalStringResource*>(
      CVAL(this)->asExternalString()->resource());
}

String::ExternalStringResourceBase* String::GetExternalStringResourceBaseSlow(
    String::Encoding* encoding_out) const {
  auto lwSelf = CVAL(this);
  auto esSelf = lwSelf->value()->asString();
  if (esSelf->has8BitContent()) {
    *encoding_out = String::Encoding::ONE_BYTE_ENCODING;
  } else {
    *encoding_out = String::Encoding::TWO_BYTE_ENCODING;
  }
  return lwSelf->asExternalString()->resource();
}

const v8::String::ExternalOneByteStringResource*
v8::String::GetExternalOneByteStringResource() const {
  auto lwSelf = CVAL(this)->asExternalString();
  return reinterpret_cast<ExternalOneByteStringResource*>(lwSelf->resource());
}

void* String::ExternalStringResourceBase::operator new(size_t size) {
  ExternalStringResourceBase* buf = (ExternalStringResourceBase*)malloc(size);
  Engine::current()->registerExternalString(buf);
  return buf;
}

void String::ExternalStringResourceBase::operator delete(void* ptr) {
  Engine::current()->unregisterExternalString((ExternalStringResourceBase*)ptr);
  free(ptr);
}

Local<Value> Symbol::Description() const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esDescription = CVAL(this)->value()->asSymbol()->descriptionString();
  return Utils::NewLocal<String>(lwIsolate->toV8(), esDescription);
}

Local<Value> Private::Name() const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esDescription = CVAL(this)->value()->asSymbol()->descriptionString();
  return Utils::NewLocal<String>(lwIsolate->toV8(), esDescription);
}

template <typename T, typename F>
static T getValue(ValueRef* esValue, F toValue) {
  auto lwContext = IsolateWrap::GetCurrent()->GetCurrentContext();
  LWNODE_CHECK(lwContext != nullptr);
  T outValue = 0;
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* esState, ValueRef* esValue, T* outValue, F toValue)
          -> ValueRef* {
        *outValue = toValue(esValue, esState);
        return esValue;
      },
      esValue,
      &outValue,
      toValue);

  if (!r.isSuccessful()) {
    LWNODE_RETURN_0;  // TODO: handle error
  }

  return outValue;
}

double Number::Value() const {
  return getValue<double>(CVAL(this)->value(),
                          [](ValueRef* esValue, ExecutionStateRef* esState) {
                            return esValue->toNumber(esState);
                          });
}

bool Boolean::Value() const {
  LWNODE_CHECK(CVAL(this)->value()->isBoolean());
  return CVAL(this)->value()->asBoolean();
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
  return ObjectRefHelper::getInternalFieldCount(
      CVAL(this)->value()->asObject());
}

static std::string toExtraDataString(const void* target,
                                     int index,
                                     const void* field) {
  std::ostringstream oss;
  ObjectData* data =
      ObjectRefHelper::getExtraData(CVAL(target)->value()->asObject());

  oss << CVAL(target)->getHandleInfoString();
  oss << " fields (" << data << ")";
  oss << " [" << index << "]";

  if (field && CVAL(field)->isValid()) {
    oss << " " << CVAL(field)->getHandleInfoString();
  } else {
    oss << " " << field;
  }
  return oss.str();
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
  auto lwValue =
      ObjectRefHelper::getInternalField(CVAL(this)->value()->asObject(), index);

  LWNODE_CALL_TRACE_ID(
      OBJDATA, "%s", toExtraDataString(this, index, lwValue).c_str());

  return Utils::ToLocal<Value>(lwValue);
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {
  LWNODE_CALL_TRACE_ID(
      OBJDATA, "%s", toExtraDataString(this, index, *value).c_str());

  ObjectRefHelper::setInternalField(
      CVAL(this)->value()->asObject(), index, VAL(*value));
}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  void* field = ObjectRefHelper::getInternalPointer(
      CVAL(this)->value()->asObject(), index);

  LWNODE_CALL_TRACE_ID(
      OBJDATA, "%s", toExtraDataString(this, index, field).c_str());

  return field;
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {
  LWNODE_CALL_TRACE_ID(
      OBJDATA, "%s", toExtraDataString(this, index, value).c_str());

  ObjectRefHelper::setInternalPointer(
      CVAL(this)->value()->asObject(), index, value);
}

void v8::Object::SetAlignedPointerInInternalFields(int argc,
                                                   int indices[],
                                                   void* values[]) {
  for (int i = 0; i < argc; i++) {
    SetAlignedPointerInInternalField(indices[i], values[i]);
  }
}

}  // namespace v8
