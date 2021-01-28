/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "api.h"
#include "escargotshim-base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {
// --- E x c e p t i o n s ---

v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(nullptr),
      next_(nullptr),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {}

v8::TryCatch::~TryCatch() {}

void* v8::TryCatch::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new TryCatch(nullptr);
}
void* v8::TryCatch::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new TryCatch(nullptr);
}
void v8::TryCatch::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void v8::TryCatch::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

bool v8::TryCatch::HasCaught() const {
  LWNODE_RETURN_FALSE;
}

bool v8::TryCatch::CanContinue() const {
  LWNODE_RETURN_FALSE;
}

bool v8::TryCatch::HasTerminated() const {
  LWNODE_RETURN_FALSE;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::TryCatch::Exception() const {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::TryCatch::StackTrace(Local<Context> context,
                                           Local<Value> exception) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::TryCatch::StackTrace(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<v8::Message> v8::TryCatch::Message() const {
  LWNODE_RETURN_LOCAL(v8::Message);
}

void v8::TryCatch::Reset() {}

void v8::TryCatch::ResetInternal() {}

void v8::TryCatch::SetVerbose(bool value) {}

bool v8::TryCatch::IsVerbose() const {
  LWNODE_RETURN_FALSE;
}

void v8::TryCatch::SetCaptureMessage(bool value) {}

// --- M e s s a g e ---

Local<String> Message::Get() const {
  LWNODE_RETURN_LOCAL(String);
}

v8::Isolate* Message::GetIsolate() const {
  LWNODE_RETURN_NULLPTR;
}

ScriptOrigin Message::GetScriptOrigin() const {
  LWNODE_UNIMPLEMENT;
  return ScriptOrigin(Local<Value>());
}

v8::Local<Value> Message::GetScriptResourceName() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  LWNODE_RETURN_LOCAL(StackTrace);
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int)
}

int Message::GetStartPosition() const {
  LWNODE_RETURN_0;
}

int Message::GetEndPosition() const {
  LWNODE_RETURN_0;
}

int Message::ErrorLevel() const {
  LWNODE_RETURN_0;
}

int Message::GetStartColumn() const {
  LWNODE_RETURN_0;
}

int Message::GetWasmFunctionIndex() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int);
}

int Message::GetEndColumn() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int);
}

bool Message::IsSharedCrossOrigin() const {
  LWNODE_RETURN_FALSE;
}

bool Message::IsOpaque() const {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(String);
}

void Message::PrintCurrentStackTrace(Isolate* isolate, FILE* out) {
  LWNODE_UNIMPLEMENT;
}

// --- S t a c k T r a c e ---

Local<StackFrame> StackTrace::GetFrame(Isolate* v8_isolate,
                                       uint32_t index) const {
  LWNODE_RETURN_LOCAL(StackFrame);
}

int StackTrace::GetFrameCount() const {
  LWNODE_RETURN_0;
}

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  LWNODE_RETURN_LOCAL(StackTrace);
}

// --- S t a c k F r a m e ---

int StackFrame::GetLineNumber() const {
  LWNODE_RETURN_0;
}

int StackFrame::GetColumn() const {
  LWNODE_RETURN_0;
}

int StackFrame::GetScriptId() const {
  LWNODE_RETURN_0;
}

Local<String> StackFrame::GetScriptName() const {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> StackFrame::GetScriptNameOrSourceURL() const {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> StackFrame::GetFunctionName() const {
  LWNODE_RETURN_LOCAL(String);
}

bool StackFrame::IsEval() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsConstructor() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsWasm() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsUserJavaScript() const {
  LWNODE_RETURN_FALSE;
}
}  // namespace v8
