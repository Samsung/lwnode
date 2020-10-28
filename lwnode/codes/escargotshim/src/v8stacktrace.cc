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

// using CachedPropertyIdRef = jsrt::CachedPropertyIdRef;
// using jsrt::IsolateShim;
// using jsrt::ContextShim;

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  // IsolateShim* iso = IsolateShim::FromIsolate(isolate);
  // ContextShim* contextShim = iso->GetCurrentContextShim();

  // JsValueRef getStackTrace = contextShim->GetgetStackTraceFunction();
  // JsValueRef stackTrace;
  // if (jsrt::CallFunction(getStackTrace, &stackTrace) != JsNoError) {
  //   return Local<StackTrace>();
  // }

  // return static_cast<StackTrace*>(stackTrace);
  NESCARGOT_UNIMPLEMENTED("");
  return Local<StackTrace>();
}

Local<StackFrame> StackTrace::GetFrame(uint32_t index) const {
  JsValueRef frame = nullptr;
  UNUSED(frame);
  //   if (jsrt::GetIndexedProperty(const_cast<StackTrace*>(this),
  //                                static_cast<int>(index), &frame) !=
  //                                JsNoError) {
  //     return Local<StackFrame>();
  //   }

  //   return static_cast<StackFrame*>(frame);
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

Local<StackFrame> StackTrace::GetFrame(Isolate* isolate, uint32_t index) const {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<StackFrame>();
}

int StackTrace::GetFrameCount() const {
  unsigned int length = 0;
  // if (jsrt::GetArrayLength(const_cast<StackTrace*>(this), &length) !=
  //     JsNoError) {
  //   return 0;
  // }
  NESCARGOT_UNIMPLEMENTED("GetArrayLength");
  return static_cast<int>(length);
}

Local<Array> StackTrace::AsArray() {
  return Local<Array>(reinterpret_cast<Array*>(this));
}

int StackFrame::GetLineNumber() const {
  // JsValueRef frame = const_cast<StackFrame*>(this);
  int result = 0;
  // if (jsrt::CallGetter(frame, CachedPropertyIdRef::getLineNumber, &result) !=
  //     JsNoError) {
  //   return 0;
  // }
  NESCARGOT_UNIMPLEMENTED("CallGetter");
  return result;
}

int StackFrame::GetColumn() const {
  // JsValueRef frame = const_cast<StackFrame*>(this);
  int result = 0;
  // if (jsrt::CallGetter(frame, CachedPropertyIdRef::getColumnNumber, &result)
  // !=
  //     JsNoError) {
  //   return 0;
  // }
  NESCARGOT_UNIMPLEMENTED("getColumnNumber");
  return result;
}

int StackFrame::GetScriptId() const {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return 0;
}

Local<String> StackFrame::GetScriptName() const {
  //   JsValueRef frame = const_cast<StackFrame*>(this);
  //   JsValueRef result;
  //   if (jsrt::CallGetter(frame, CachedPropertyIdRef::getFileName, &result) !=
  //       JsNoError) {
  //     return Local<String>();
  //   }
  //   return static_cast<String*>(result);
  NESCARGOT_UNIMPLEMENTED("GetScriptName");
  return Local<String>();
}

Local<String> StackFrame::GetScriptNameOrSourceURL() const {
  NESCARGOT_ASSERT(false);
  return Local<String>();
}

Local<String> StackFrame::GetFunctionName() const {
  // JsValueRef frame = const_cast<StackFrame*>(this);
  // JsValueRef result;
  // if (jsrt::CallGetter(frame, CachedPropertyIdRef::getFunctionName, &result)
  // !=
  //     JsNoError) {
  //   return Local<String>();
  // }
  // return static_cast<String*>(result);

  NESCARGOT_UNIMPLEMENTED("CallGetter");
  return Local<String>();
}

bool StackFrame::IsEval() const {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return false;
}

}  // namespace v8
