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
#include "escargotutil.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

Local<Value> BooleanObject::New(Isolate* isolate, bool value) {
  NESCARGOT_ASSERT(isolate);

  auto v = CreateJsValue(value);
  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsBooleanObjectRef newBooleanObjectRef;
  if (CreateJsBooleanObject(context, v, newBooleanObjectRef) != JsNoError) {
    return Local<Value>();
  }

  return Local<BooleanObject>::New(isolate, newBooleanObjectRef);
}

Local<Value> BooleanObject::New(bool value) {
  return New(IsolateShim::GetCurrent()->asIsolate(), value);
}

bool BooleanObject::ValueOf() const {
  JsBooleanObjectRef self = asJsValueRef()->asBooleanObject();
  return self->primitiveValue();
}

BooleanObject* BooleanObject::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsBooleanObject());
  return static_cast<BooleanObject*>(obj);
}

}  // namespace v8
