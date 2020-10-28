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

Local<String> Message::Get() const {
  // CHAKRA-TODO: Figure out what to do here
  NESCARGOT_UNIMPLEMENTED("");
  return Local<String>();
}

Isolate* Message::GetIsolate() const {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

int Message::ErrorLevel() const {
  return 0;
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context)
  auto selfObject = (CastTo<JsValueRef>(this))->asObject();
  JsValueRef value = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, selfObject, CachedStringId::source, value) !=
      JsNoError) {
    return Local<String>();
  };

  Isolate* isolate =
      ContextShim::ToContextShim(*context)->isolateShim()->asIsolate();
  return Local<String>::New(isolate, value);
}

Local<String> Message::GetSourceLine() const {
  return FromMaybe(GetSourceLine(Local<Context>()));
}

ScriptOrigin Message::GetScriptOrigin() const {
  // CHAKRA-TODO: Figure out what to do here
  NESCARGOT_UNIMPLEMENTED("");
  return ScriptOrigin(Local<String>());
}

Local<StackTrace> Message::GetStackTrace() const {
  // CHAKRA-TODO: Figure out what to do here
  NESCARGOT_UNIMPLEMENTED("");
  return Local<StackTrace>();
}

Handle<Value> Message::GetScriptResourceName() const {
  IsolateShim* isolateShim = IsolateShim::GetCurrent();
  JsContextRef contextRef = isolateShim->currentContext()->contextRef();
  auto selfObject = (CastTo<JsValueRef>(this))->asObject();
  JsValueRef value = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, selfObject, CachedStringId::url, value) !=
      JsNoError) {
    return Local<Value>();
  };
  return Local<Value>::New(isolateShim->asIsolate(), value);
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  auto selfObject = (CastTo<JsValueRef>(this))->asObject();
  JsValueRef value = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, selfObject, CachedStringId::line, value) !=
      JsNoError) {
    return Nothing<int>();
  };
  return Just<int>(value->asNumber());
}

int Message::GetLineNumber() const {
  return FromMaybe(GetLineNumber(Local<Context>()));
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  auto selfObject = (CastTo<JsValueRef>(this))->asObject();
  JsValueRef value = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, selfObject, CachedStringId::column, value) !=
      JsNoError) {
    return Nothing<int>();
  };
  return Just<int>(value->asNumber());
}

int Message::GetStartColumn() const {
  return FromMaybe(GetStartColumn(Local<Context>()));
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  int column = GetStartColumn();
  auto selfObject = (CastTo<JsValueRef>(this))->asObject();
  JsValueRef value = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, selfObject, CachedStringId::length, value) !=
      JsNoError) {
    return Nothing<int>();
  };
  int length = value->asNumber();
  return Just<int>(column + length);
}

int Message::GetEndColumn() const {
  return FromMaybe(GetEndColumn(Local<Context>()));
}

}  // namespace v8
