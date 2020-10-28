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

Local<Value> SymbolObject::New(Isolate* isolate, Local<Symbol> value) {
  NESCARGOT_ASSERT(isolate);

  JsSymbolRef symbolRef = value->asJsValueRef()->asSymbol();
  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsSymbolObjectRef symbolObjectRef = JS_INVALID_REFERENCE;
  if (CreateJsSymbolObject(context, symbolRef, symbolObjectRef) != JsNoError) {
    return Local<Value>();
  }

  return Local<SymbolObject>::New(isolate, symbolObjectRef);
}

Local<Symbol> SymbolObject::ValueOf() const {
  JsSymbolObjectRef self = asJsValueRef()->asSymbolObject();
  JsSymbolRef symbolRef = self->primitiveValue();

  return Local<Symbol>::New(IsolateShim::GetCurrent()->asIsolate(), symbolRef);
}

SymbolObject* SymbolObject::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsSymbolObject());
  return static_cast<SymbolObject*>(obj);
}

}  // namespace v8
