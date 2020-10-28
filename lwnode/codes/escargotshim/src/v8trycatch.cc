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

TryCatch::TryCatch(Isolate* isolate)
    : metadata(nullptr /* JS_INVALID_REFERENCE */),
      rethrow(false),
      user(true),
      verbose(false) {
  //   jsrt::IsolateShim * isolateShim = jsrt::IsolateShim::GetCurrent();
  //   prev = isolateShim->tryCatchStackTop;
  //   isolateShim->tryCatchStackTop = this;

  auto scriptException = IsolateShim::GetCurrent()->GetScriptException();
  prev = scriptException->tryCatchStackTop();
  scriptException->SetTryCatchStackTop(this);
}

TryCatch::~TryCatch() {
  if (!rethrow) {
    GetAndClearException();
  }

  IsolateShim::GetCurrent()->GetScriptException()->SetTryCatchStackTop(prev);
}

bool TryCatch::HasCaught() const {
  if (metadata == JS_INVALID_REFERENCE) {
    CastTo<TryCatch*>(this)->GetAndClearException();
  }

  if (metadata != JS_INVALID_REFERENCE) {
    return true;
  }
  // bool hasException;
  // JsErrorCode errorCode = JsHasException(&hasException);
  // if (errorCode != JsNoError) {
  //   if (errorCode == JsErrorInDisabledState) {
  //     return true;
  //   }
  //   // Should never get errorCode other than JsNoError/JsErrorInDisabledState
  //   CHAKRA_ASSERT(false);
  //   return false;
  // }
  return IsolateShim::GetCurrent()->GetScriptException()->HasException();
}

bool TryCatch::HasTerminated() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
  // return jsrt::IsolateShim::GetCurrent()->IsExeuctionDisabled();
}

bool v8::TryCatch::CanContinue() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

void TryCatch::GetAndClearException() {
  auto scriptException = IsolateShim::GetCurrent()->GetScriptException();

  if (!scriptException->HasException()) {
    metadata = JS_INVALID_REFERENCE;
    return;
  }

  metadata = scriptException->Exception();
  scriptException->ClearException();
}

Handle<Value> TryCatch::ReThrow() {
  JsValueRef error = this->EnsureException();
  if (error == JS_INVALID_REFERENCE) {
    return Local<Value>();
  }

  rethrow = true;
  return Local<Value>::New(error);
}

Local<Value> TryCatch::Exception() const {
  JsValueRef error = this->EnsureException();
  if (error == JS_INVALID_REFERENCE) {
    return Local<Value>();
  }

  //   // NOTE: metadata which create an error v8::Message is generated from
  //   GetAndClearException
  //   // metadata:JsValue has a property named `exception`.

  return Local<Value>(error);
}

// MaybeLocal<Value> TryCatch::StackTrace(Local<Context> context) const {
//   JsValueRef error = this->EnsureException();
//   if (error == JS_INVALID_REFERENCE) {
//     return Local<Value>();
//   }

//   JsPropertyIdRef stack = jsrt::IsolateShim::GetCurrent()
//       ->GetCachedPropertyIdRef(jsrt::CachedPropertyIdRef::stack);

//   JsValueRef trace;
//   if (JsGetProperty(error, stack, &trace) != JsNoError) {
//     return Local<Value>();
//   }

//   return Local<Value>::New(trace);
// }

// Local<Value> TryCatch::StackTrace() const {
//   return FromMaybe(StackTrace(Local<Context>()));
// }

Local<v8::Message> TryCatch::Message() const {
  if (metadata == JS_INVALID_REFERENCE) {
    const_cast<TryCatch*>(this)->GetAndClearException();
  }

  if (metadata == JS_INVALID_REFERENCE) {
    return Local<v8::Message>();
  }
  return Local<v8::Message>::New(metadata);
}

void TryCatch::SetVerbose(bool value) {
  this->verbose = value;
}

bool TryCatch::IsVerbose() const {
  return this->verbose;
}

void TryCatch::CheckReportExternalException() {
  // Let caller TryCatch record the exception
  TryCatch* tryCatch = (prev != nullptr && prev->user) ? prev : this;
  if (tryCatch == prev) {
    tryCatch->GetAndClearException();
  }

  // This is only used by Function::Call. If caller does not use TryCatch to
  // handle external exceptions, or uses a TryCatch and SetVerbose(),
  // we'll report the external exception message (triggers uncaughtException).
  if (prev == nullptr || prev->verbose) {
    IsolateShim::GetCurrent()->ForEachMessageListener(
        [tryCatch](void* messageListener) {
          ((v8::MessageCallback)messageListener)(tryCatch->Message(),
                                                 tryCatch->Exception());
        });
    IsolateShim::GetCurrent()->GetScriptException()->ThrownException();
  } else {
    rethrow = true;  // Otherwise leave the exception as is
  }
}

JsValueRef TryCatch::EnsureException() const {
  if (metadata == JS_INVALID_REFERENCE) {
    const_cast<TryCatch*>(this)->GetAndClearException();
  }

  if (metadata == JS_INVALID_REFERENCE) {
    return JS_INVALID_REFERENCE;
  }

  // JsPropertyIdRef err = jsrt::IsolateShim::GetCurrent()
  //     ->GetCachedPropertyIdRef(jsrt::CachedPropertyIdRef::exception);

  // JsValueRef exception;
  // if (JsGetProperty(metadata, err, &exception) != JsNoError) {
  //   return JS_INVALID_REFERENCE;
  // }

  return metadata;  // TODO: get error object of metadata
}

}  // namespace v8
