/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

#include "escargotutil.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

Local<PrimitiveArray> PrimitiveArray::New(Isolate* isolate, int length) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();
  JsArrayObjectRef arrayObj = JS_INVALID_REFERENCE;
  if (CreateJsArrayObject(context, arrayObj) != JsNoError) {
    return Local<PrimitiveArray>();
  }

  if (length > 0) {
    bool ok = false;
    JsErrorCode ret = SetProperty(
        context, arrayObj, CachedStringId::length, CreateJsValue(length), ok);
    if (ret != JsNoError || !ok) {
      return Local<PrimitiveArray>();
    }
  }

  return Local<PrimitiveArray>(arrayObj);
}

void PrimitiveArray::Set(int index, Local<Primitive> item) {
  return Set(Isolate::GetCurrent(), index, item);
}

void PrimitiveArray::Set(Isolate* isolate, int index, Local<Primitive> item) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();
  JsArrayObjectRef self = CastTo<JsArrayObjectRef>(this);
  JsValueRef indexObj = CreateJsValue(index);
  JsValueRef valObj = item->asJsValueRef();

  bool ok = false;
  JsErrorCode ret = SetProperty(context, self, indexObj, valObj, ok);
  if (ret != JsNoError || !ok) {
    NESCARGOT_LOG_ERROR("%s: failed to set an item\n", __func__);
  }
}

Local<Primitive> PrimitiveArray::Get(int index) {
  return Get(Isolate::GetCurrent(), index);
}

Local<Primitive> PrimitiveArray::Get(Isolate* isolate, int index) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();
  JsArrayObjectRef self = CastTo<JsArrayObjectRef>(this);
  JsValueRef indexObj = CreateJsValue(index);
  JsValueRef valObj = JS_INVALID_REFERENCE;

  JsErrorCode ret = GetProperty(context, self, indexObj, valObj);
  if (ret != JsNoError) {
    return Local<Primitive>(CreateJsUndefined());
  }

  return Local<Primitive>(valObj);
}

int PrimitiveArray::Length() const {
  JsContextRef context =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  JsArrayObjectRef self = CastTo<JsArrayObjectRef>(this);
  JsValueRef valObj = JS_INVALID_REFERENCE;

  JsErrorCode ok = GetProperty(context, self, CachedStringId::length, valObj);
  if (ok != JsNoError) {
    return 0;
  }

  int length = 0;
  JsEvaluator::EvaluatorResult ret = EvalScript(
      context,
      [](JsExecutionStateRef state, JsValueRef val, int* length) -> JsValueRef {
        *length = val->toNumber(state);
        return CreateJsValue(length);
      },
      valObj,
      &length);

  VERIFY_EVAL_RESULT(ret, 0);
  return length;
}
}  // namespace v8
