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

#include <limits.h>
#include <math.h>
#include "escargotutil.h"
#include "jsutils.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

// static bool IsOfType(const Value* ref, JsValueType type) {
//   JsValueType valueType;
//   if (JsGetValueType(const_cast<Value*>(ref), &valueType) != JsNoError) {
//     return false;
//   }
//   return valueType == type;
// }

bool Value::IsUndefined() const {
  return asJsValueRef()->isUndefined();
}

bool Value::IsNull() const {
  return asJsValueRef()->isNull();
}

bool Value::IsNullOrUndefined() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsTrue() const {
  return asJsValueRef()->isTrue();
}

bool Value::IsFalse() const {
  return asJsValueRef()->isFalse();
}

bool Value::IsString() const {
  return asJsValueRef()->isString();
}

bool Value::IsSymbol() const {
  return asJsValueRef()->isSymbol();
}

bool Value::IsGeneratorFunction() const {
  return asJsValueRef()->isGeneratorFunctionObject();
}

// NOTE: isFunctionObject returns a wrong result with a boundFunction (e.g
// function fn() var boundFunction = fn.bind(this); )
//
// bool Value::IsFunction() const {
//   return CastTo<JsValueRef>(this)->isFunctionObject();
// }

bool Value::IsArray() const {
  JsValueRef value = asJsValueRef();
  if (value->isObject()) {
    return value->asObject()->isArrayObject();
  }
  return false;
}

bool Value::IsObject() const {
  // return type >= JsValueType::JsObject && type != JsSymbol;
  return asJsValueRef()->isObject();
}

bool Value::IsExternal() const {
  return External::IsExternal(this);
}

bool Value::IsArrayBuffer() const {
  JsValueRef value = asJsValueRef();
  if (value->isObject() == false) {
    return false;
  }

  return value->asObject()->isArrayBufferObject();
}

bool Value::IsTypedArray() const {
  JsValueRef value = asJsValueRef();
  if (value->isObject() == false) {
    return false;
  }

  // TODO: Escargot doesn't have a public API for this at the moment.
  auto result = EvalScript(
      GetCurrentJsContextRef(),
      [](JsExecutionStateRef state, JsValueRef value) -> JsValueRef {
        auto nullableobj = value->asObject()->getPrototypeObject(state);
        if (nullableobj.hasValue() == false) {
          return CreateJsValue(false);
        }
        return CreateJsValue(nullableobj.get()->isTypedArrayPrototypeObject());
      },
      value);
  VERIFY_EVAL_RESULT(result, false);
  return result.result->asBoolean();
}

#define DEFINE_TYPEDARRAY_CHECK(ArrayType)                                     \
  bool Value::Is##ArrayType##Array() const {                                   \
    auto value = asJsValueRef();                                               \
    if (!value->isObject()) {                                                  \
      return false;                                                            \
    }                                                                          \
    return value->asObject()->is##ArrayType##ArrayObject();                    \
  }

DEFINE_TYPEDARRAY_CHECK(Uint8)
DEFINE_TYPEDARRAY_CHECK(Uint8Clamped)
DEFINE_TYPEDARRAY_CHECK(Int8)
DEFINE_TYPEDARRAY_CHECK(Uint16)
DEFINE_TYPEDARRAY_CHECK(Int16)
DEFINE_TYPEDARRAY_CHECK(Uint32)
DEFINE_TYPEDARRAY_CHECK(Int32)
DEFINE_TYPEDARRAY_CHECK(Float32)
DEFINE_TYPEDARRAY_CHECK(Float64)

bool Value::IsArrayBufferView() const {
  JsValueRef value = asJsValueRef();
  if (value->isObject() == false) {
    return false;
  }

  return value->asObject()->isArrayBufferView();
}

bool Value::IsBoolean() const {
  return asJsValueRef()->isBoolean();
}

bool Value::IsNumber() const {
  return asJsValueRef()->isNumber();
}

bool Value::IsInt32() const {
  if (!IsNumber()) {
    return false;
  }

  double value = NumberValue();

  // check that the value is smaller than int 32 bit maximum
  if (value > INT_MAX || value < INT_MIN) {
    return false;
  }
  // negative zero is not an integer
  if (value == 0 && std::signbit(value)) {
    return false;
  }

  return trunc(value) == value;
}

bool Value::IsUint32() const {
  if (!IsNumber()) {
    return false;
  }

  double value = NumberValue();
  // check that the value is smaller than 32 bit maximum
  if (value < 0 || value > UINT_MAX) {
    return false;
  }
  // negative zero is not an integer
  if (value == 0 && std::signbit(value)) {
    return false;
  }

  return trunc(value) == value;
}

bool Value::IsBigInt() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

#define IS_TYPE_FUNCTION(v8ValueFunc, escargotJsFunc)                          \
  bool Value::v8ValueFunc() const {                                            \
    return Local<Value>(js::escargotJsFunc(asJsValueRef()))->BooleanValue();   \
  }  // namespace v8

IS_TYPE_FUNCTION(IsBooleanObject, isBooleanObject)
IS_TYPE_FUNCTION(IsDate, isDate)
IS_TYPE_FUNCTION(IsMap, isMap)
IS_TYPE_FUNCTION(IsNativeError, isNativeError)
IS_TYPE_FUNCTION(IsPromise, isPromise)
IS_TYPE_FUNCTION(IsProxy, isProxy)
IS_TYPE_FUNCTION(IsRegExp, isRegExp)
IS_TYPE_FUNCTION(IsAsyncFunction, isAsyncFunction)
IS_TYPE_FUNCTION(IsSet, isSet)
IS_TYPE_FUNCTION(IsStringObject, isStringObject)
IS_TYPE_FUNCTION(IsNumberObject, isNumberObject)
IS_TYPE_FUNCTION(IsMapIterator, isMapIterator)
IS_TYPE_FUNCTION(IsSetIterator, isSetIterator)
IS_TYPE_FUNCTION(IsArgumentsObject, isArgumentsObject)
IS_TYPE_FUNCTION(IsGeneratorObject, isGeneratorObject)
IS_TYPE_FUNCTION(IsWeakMap, isWeakMap)
IS_TYPE_FUNCTION(IsWeakSet, isWeakSet)
IS_TYPE_FUNCTION(IsSymbolObject, isSymbolObject)
IS_TYPE_FUNCTION(IsName, isName)
IS_TYPE_FUNCTION(IsDataView, isDataView)
IS_TYPE_FUNCTION(IsFunction, isFunction)

MaybeLocal<Boolean> Value::ToBoolean(Local<Context> context) const {
  // JsValueRef value;
  // if (JsConvertValueToBoolean((JsValueRef) this, &value) != JsNoError) {
  //   return Local<Boolean>();
  // }

  // return Local<Boolean>::New(value);
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Boolean>();
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  JsValueRef value;

  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsValueRef self = asJsValueRef();

  // Try converting value to Number
  auto sbresult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state, JsValueRef self) -> JsValueRef {
                   return CreateJsValue(self->toNumber(state));
                 },
                 self);
  VERIFY_EVAL_RESULT(sbresult, Local<Number>());

  value = sbresult.result;

  return Local<Number>::New(value);
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
  JsValueRef value;
  JsContextRef contextRef = context->asContextShim()->contextRef();
  JsValueRef self = asJsValueRef();

  // Try converting value to string
  auto sbresult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state, JsValueRef self) -> JsValueRef {
                   return CreateJsValue(self->toString(state));
                 },
                 self);
  VERIFY_EVAL_RESULT(sbresult, Local<String>());

  value = sbresult.result;
  return Local<String>::New(IsolateShim::GetCurrent()->asIsolate(), value);
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  return ToString(context);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  JsValueRef value = asJsValueRef();
  if (!value->isObject()) {
    return Local<Object>();
  }

  return Local<Object>::New(value->asObject());
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  Maybe<double> maybe = NumberValue(context);
  if (maybe.IsNothing()) {
    return Local<Integer>();
  }
  Isolate* isolate = context->asContextShim()->isolateShim()->asIsolate();

  double doubleValue = maybe.FromJust();
  double value = isfinite(doubleValue) ? trunc(doubleValue)
                                       : (isnan(doubleValue) ? 0 : doubleValue);
  int intValue = static_cast<int>(value);
  if (value == intValue) {
    return Integer::New(isolate, intValue);
  }

  Value* result = value == doubleValue ? const_cast<Value*>(this)
                                       : *Number::New(nullptr, value);
  return Local<Integer>(reinterpret_cast<Integer*>(result));
}

MaybeLocal<Uint32> Value::ToUint32(Local<Context> context) const {
  Isolate* isolate = context->asContextShim()->isolateShim()->asIsolate();
  Local<Integer> jsValue =
      Integer::NewFromUnsigned(isolate, this->Uint32Value());
  return jsValue.As<Uint32>();
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
  Isolate* isolate =
      ContextShim::ToContextShim(*context)->isolateShim()->asIsolate();
  Local<Integer> jsValue =
      Integer::New(isolate, FromMaybe(Int32Value(context)));
  return jsValue.As<Int32>();
}

Local<Boolean> Value::ToBoolean(Isolate* isolate) const {
  return FromMaybe(
      ToBoolean(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<Number> Value::ToNumber(Isolate* isolate) const {
  return FromMaybe(
      ToNumber(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<String> Value::ToString(Isolate* isolate) const {
  return FromMaybe(
      ToString(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<String> Value::ToDetailString(Isolate* isolate) const {
  return FromMaybe(
      ToDetailString(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<Object> Value::ToObject(Isolate* isolate) const {
  return FromMaybe(
      ToObject(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<Integer> Value::ToInteger(Isolate* isolate) const {
  return FromMaybe(
      ToInteger(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

Local<Uint32> Value::ToUint32(Isolate* isolate) const {
  return FromMaybe(
      ToUint32(IsolateShim::ToIsolateShim(isolate)->currentContext()));
}

MaybeLocal<Uint32> Value::ToArrayIndex(Local<Context> context) const {
  if (IsNumber()) {
    return ToUint32(context);
  }

  // MaybeLocal<String> maybeString = ToString(context);
  // bool isUint32;
  // uint32_t uint32Value;
  // if (maybeString.IsEmpty() ||
  //     jsrt::TryParseUInt32(*FromMaybe(maybeString), &isUint32, &uint32Value)
  //     !=
  //         JsNoError) {
  //   return Local<Uint32>();
  // }

  // return Integer::NewFromUnsigned(nullptr, uint32Value).As<Uint32>();
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Uint32>();
}

Local<Uint32> Value::ToArrayIndex() const {
  return FromMaybe(ToArrayIndex(Local<Context>()));
}

bool Value::BooleanValue(Isolate* isolate) const {
  JsValueRef self = asJsValueRef();
  JsContextRef contextRef =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  auto sbresult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state, JsValueRef self) -> JsValueRef {
                   return CreateJsValue(self->toBoolean(state));
                 },
                 self);
  VERIFY_EVAL_RESULT(sbresult, false);

  JsValueRef result = sbresult.result;
  return result->asBoolean();
}

Maybe<bool> Value::BooleanValue(Local<Context> context) const {
  return Just(
      BooleanValue(context->asContextShim()->isolateShim()->asIsolate()));
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
  JsContextRef contextRef = ContextShim::ToContextShim(*context)->contextRef();
  JsValueRef self = asJsValueRef();

  auto sbresult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state, JsValueRef self) -> JsValueRef {
                   return CreateJsValue(self->toNumber(state));
                 },
                 self);
  JsValueRef result = sbresult.result;
  VERIFY_EVAL_RESULT(sbresult, Nothing<double>());

  double value = result->asNumber();
  return Just(value);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  JsContextRef contextRef = ContextShim::ToContextShim(*context)->contextRef();
  JsValueRef self = asJsValueRef();

  auto sbresult =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state, JsValueRef self) -> JsValueRef {
                   return CreateJsValue(self->toNumber(state));
                 },
                 self);

  JsValueRef result = sbresult.result;
  VERIFY_EVAL_RESULT(sbresult, Nothing<int64_t>());

  double value = result->asNumber();
  if (std::isnan(value)) {
    value = 0;
  }

  int64_t ret = static_cast<int64_t>(value);
  return Just(ret);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  uint32_t intValue;

  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsValueRef self = asJsValueRef();
  auto sbresult = EvalScript(contextRef,
                             [](JsExecutionStateRef state,
                                JsValueRef self,
                                uint32_t* intValue) -> JsValueRef {
                               *intValue = self->toUint32(state);
                               return JsUndefined();
                             },
                             self,
                             &intValue);
  VERIFY_EVAL_RESULT(sbresult, Nothing<uint32_t>());
  return Just(intValue);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsValueRef self = asJsValueRef();
  int32_t val = 0;
  auto sbresult = EvalScript(contextRef,
                             [](JsExecutionStateRef state,
                                JsValueRef self,
                                int32_t* val) -> JsValueRef {
                               *val = self->toInt32(state);
                               return JsUndefined();
                             },
                             self,
                             &val);
  return Just(val);
}

bool Value::BooleanValue() const {
  return BooleanValue(IsolateShim::GetCurrent()->asIsolate());
}

double Value::NumberValue() const {
  return FromMaybe(
      NumberValue(IsolateShim::GetCurrent()->currentContext()->asContext()));
}

int64_t Value::IntegerValue() const {
  return FromMaybe(
      IntegerValue(IsolateShim::GetCurrent()->currentContext()->asContext()));
}

uint32_t Value::Uint32Value() const {
  return FromMaybe(Uint32Value(Local<Context>()));
}

// FIXME: remove it
int32_t Value::Int32Value() const {
  return FromMaybe(Int32Value(Local<Context>()));
}

bool Value::IsModuleNamespaceObject() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsBigIntObject() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsSharedArrayBuffer() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsWebAssemblyCompiledModule() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsBigInt64Array() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool Value::IsBigUint64Array() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

Maybe<bool> Value::Equals(Local<Context> context, Handle<Value> that) const {
  JsValueRef self = asJsValueRef();
  GENERATE_CONTEXT_AND_CHECK_EXIST(context)
  auto result = EvalScript(contextRef,
                           [](JsExecutionStateRef state,
                              JsValueRef self,
                              JsValueRef that) -> JsValueRef {
                             bool equals = self->equalsTo(state, that);
                             return CreateJsValue(equals);
                           },
                           self,
                           that->asJsValueRef());

  VERIFY_EVAL_RESULT(result, Nothing<bool>());
  return Just(result.result->asBoolean());
}

bool Value::Equals(Handle<Value> that) const {
  return FromMaybe(Equals(Local<Context>(), that));
}

bool Value::StrictEquals(Handle<Value> that) const {
  JsValueRef self = asJsValueRef();
  auto result = EvalScript(GetCurrentJsContextRef(),
                           [](JsExecutionStateRef state,
                              JsValueRef self,
                              JsValueRef that) -> JsValueRef {
                             bool strictEquals =
                                 self->abstractEqualsTo(state, that);
                             return CreateJsValue(strictEquals);
                           },
                           self,
                           that->asJsValueRef());
  VERIFY_EVAL_RESULT(result, false);
  return result.result->asBoolean();
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> object) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

bool Value::SameValue(Local<Value> that) const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}
}  // namespace v8
