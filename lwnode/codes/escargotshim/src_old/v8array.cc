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
#include "v8.h"

using namespace EscargotShim;

namespace v8 {

uint32_t Array::Length() const {
  JsContextRef context =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  JsValueRef obj = asJsValueRef();

  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state, JsValueRef obj) -> JsValueRef {
                   return obj->asObject()->getOwnProperty(
                       state, GetCachedJsValue(CachedStringId::length));
                 },
                 obj);
  VERIFY_EVAL_RESULT(result, 0);
  return result.result->asUint32();
}

Local<Array> Array::New(Isolate* isolate, int length) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsArrayObjectRef arrayObject = JS_INVALID_REFERENCE;
  if (CreateJsArrayObject(context, arrayObject) != JsNoError) {
    return Local<Array>();
  }
  if (length > 0) {
    bool result = false;
    if (SetProperty(context,
                    arrayObject,
                    CachedStringId::length,
                    CreateJsValue(length),
                    result) != JsNoError ||
        result == false) {
      return Local<Array>();
    }
  }

  return Local<Array>::New(isolate, arrayObject);
}

Local<Array> Array::New(Isolate* isolate,
                        Local<Value>* elements,
                        size_t length) {
  Local<Array> arr = Array::New(isolate, length);

  if (elements == nullptr) {
    return arr;
  }

  for (uint32_t index = 0; index < length; index++) {
    arr->Set(index, elements[index]);
  }

  return arr;
}

Array* Array::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsArray());
  return static_cast<Array*>(obj);
}

}  // namespace v8
