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
#include "v8utils.h"

using namespace EscargotShim;

namespace v8 {

template <class Func>
Local<Value> Utils::NewError(Handle<String> message) {
  return NewError(message, JsErrorObjectCode::None);
}

Local<Value> Utils::NewError(Handle<String> message,
                             JsErrorObjectCode errorCode) {
  JsContextRef context = GetCurrentJsContextRef();
  JsValueRef value = message->asJsValueRef();

  JsErrorObjectRef errorObject = JS_INVALID_REFERENCE;
  if (CreateJsErrorObject(context, errorCode, value, errorObject) !=
      JsNoError) {
    return Local<Value>();
  }

  return Local<Value>::New(errorObject);
}

Local<Value> Exception::RangeError(Handle<String> message) {
  return Utils::NewError<JsRangeErrorObjectRef>(message);
}

Local<Value> Exception::ReferenceError(Handle<String> message) {
  return Utils::NewError<JsReferenceErrorObjectRef>(message);
}

Local<Value> Exception::SyntaxError(Handle<String> message) {
  return Utils::NewError<JsSyntaxErrorObjectRef>(message);
}

Local<Value> Exception::TypeError(Handle<String> message) {
  return Utils::NewError<JsTypeErrorObjectRef>(message);
}

Local<Value> Exception::Error(Handle<String> message) {
  return Utils::NewError(message, JsErrorObjectCode::None);
}

Local<Message> Exception::CreateMessage(Isolate* isolate,
                                        Local<Value> exception) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Message>();
}

}  // namespace v8
