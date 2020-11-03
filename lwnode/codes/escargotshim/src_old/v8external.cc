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

namespace v8 {

using namespace EscargotShim;

Local<Value> External::Wrap(void* data) {
  NESCARGOT_ASSERT(false);
  return External::New(Isolate::GetCurrent(), data);
}

inline void* External::Unwrap(Handle<v8::Value> obj) {
  if (!obj->IsExternal()) {
    return nullptr;
  }

  return obj.As<External>()->Value();
}

Local<External> External::New(Isolate* isolate, void* value) {
  IsolateShim* isolateShim = IsolateShim::ToIsolateShim(isolate);
  JsContextRef context = isolateShim->currentContext()->contextRef();

  JsObjectRef external = JS_INVALID_REFERENCE;
  if (CreateJsObject(context, external) != JsNoError) {
    return Local<External>();
  }
  SetExtraData(external, value);

  auto propertyName =
      isolateShim->Cached()->Symbols(CachedSymbolId::__external__);

  bool result = false;
  if (DefineDataProperty(context,
                         external,
                         propertyName,
                         false,
                         false,
                         false,
                         isolateShim->Cached()->True(),
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         result) != JsNoError ||
      result == false) {
    return Local<External>();
  }

  return Local<External>::New(isolate, external);
}

bool External::IsExternal(const v8::Value* value) {
  if (value->IsObject() == false) {
    return false;
  }

  auto contextRef = IsolateShim::GetCurrent()->currentContext()->contextRef();
  JsObjectRef self = value->asJsValueRef()->asObject();
  auto result = false;
  if (HasOwnProperty(contextRef,
                     self,
                     IsolateShim::GetCurrent()->Cached()->Symbols(
                         CachedSymbolId::__external__),
                     result) != JsNoError) {
    return false;
  };
  return result;
  // NOTE: we can consider using Symbol. With Symbol instance with empty string,
  // the buffer for string itself can be reduced.
  // CreateJsValueString(TEST_SYMBOL));
  // CreateJsValueSymbol(CreateJsStringAscii(TEST_SYMBOL)));
}

External* External::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsExternal());
  return static_cast<External*>(obj);
}

void* External::Value() const {
  NESCARGOT_ASSERT(External::IsExternal(this));

  return asJsValueRef()->asObject()->extraData();
}

}  // namespace v8
