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
#include "base.h"

#include <sstream>

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {
// --- E x c e p t i o n s ---

// 'exception_' is of type ValueWrap*.
v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)),
      next_(isolate_->try_catch_handler()),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {
  ResetInternal();
  isolate_->RegisterTryCatchHandler(this);
}

v8::TryCatch::~TryCatch() {
  if (rethrow_) {
    v8::Isolate* v8Isolate = IsolateWrap::toV8(isolate_);
    v8::HandleScope scope(v8Isolate);
    v8::Local<v8::Value> v8Exception =
        v8::Local<v8::Value>::New(v8Isolate, Exception());
    if (HasCaught() && capture_message_) {
      // TODO
      // If an exception was caught and rethrow_ is indicated, the saved
      // message, script, and location need to be restored to Isolate TLS
      // for reuse.  capture_message_ needs to be disabled so that Throw()
      // does not create a new message.
    }
    isolate_->UnregisterTryCatchHandler(this);
    reinterpret_cast<Isolate*>(isolate_)->ThrowException(v8Exception);
  } else {
    if (HasCaught() && isolate_->has_scheduled_exception()) {
      // If an exception was caught but is still scheduled because no API call
      // promoted it, then it is canceled to prevent it from being propagated.
      // Note that this will not cancel termination exceptions.
      isolate_->CancelScheduledExceptionFromTryCatch(this);
    }
    isolate_->UnregisterTryCatchHandler(this);
  }
}

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
  return !IsolateWrap::fromV8(isolate_)->isHole(
      ExceptionHelper::unwrapException(exception_));
}

bool v8::TryCatch::CanContinue() const {
  LWNODE_RETURN_FALSE;
}

bool v8::TryCatch::HasTerminated() const {
  LWNODE_RETURN_FALSE;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  if (!HasCaught()) {
    return v8::Local<v8::Value>();
  }
  rethrow_ = true;
  return v8::Utils::ToLocal<Value>(IsolateWrap::fromV8(isolate_)->undefined());
}

v8::Local<Value> v8::TryCatch::Exception() const {
  if (HasCaught()) {
    return v8::Utils::NewLocal<Value>(
        IsolateWrap::toV8(isolate_),
        EscargotShim::ExceptionHelper::unwrapException(exception_));
  } else {
    return v8::Local<Value>();
  }
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

void v8::TryCatch::Reset() {
  rethrow_ = false;
  isolate_->CancelScheduledExceptionFromTryCatch(this);
  ResetInternal();
}

void v8::TryCatch::ResetInternal() {
  exception_ = IsolateWrap::fromV8(isolate_)->hole();
  message_obj_ = IsolateWrap::fromV8(isolate_)->hole()->value();
}

void v8::TryCatch::SetVerbose(bool value) {
  is_verbose_ = value;
}

bool v8::TryCatch::IsVerbose() const {
  return is_verbose_;
}

void v8::TryCatch::SetCaptureMessage(bool value) {
  capture_message_ = value;
}

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
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto stackTrace = lwIsolate->stackTrace();
  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return Utils::NewLocal<String>(lwIsolate->toV8(), StringRef::emptyString());
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  return Utils::NewLocal<String>(lwIsolate->toV8(), top->src);
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  LWNODE_RETURN_LOCAL(StackTrace);
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  auto lwIsolate = CVAL(*context)->context()->GetIsolate();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return Just<int>(0);
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  return Just<int>(top->loc.line);
}

int Message::GetStartPosition() const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return 0;
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  return top->loc.index;
}

int Message::GetEndPosition() const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return 0;
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  return top->loc.index + 1;
}

int Message::ErrorLevel() const {
  LWNODE_RETURN_0;
}

int Message::GetStartColumn() const {
  return GetStartColumn(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(0);
}

int Message::GetWasmFunctionIndex() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  auto lwIsolate = CVAL(*context)->context()->GetIsolate();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return Nothing<int>();
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  return Just<int>(top->loc.column - 1);
}

int Message::GetEndColumn() const {
  return GetEndColumn(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(0);
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  auto lwIsolate = CVAL(*context)->context()->GetIsolate();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return Nothing<int>();
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  int endCol = top->loc.column;

  return Just<int>(endCol);
}

bool Message::IsSharedCrossOrigin() const {
  LWNODE_RETURN_FALSE;
}

bool Message::IsOpaque() const {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  auto lwIsolate = CVAL(*context)->context()->GetIsolate();
  auto stackTrace = lwIsolate->stackTrace();

  if (stackTrace->empty()) {
    LWNODE_LOG_WARN("No StackTrace found");
    return Utils::NewLocal<String>(lwIsolate->toV8(), StringRef::emptyString());
  }

  IsolateWrap::StackTraceData* top = stackTrace->front();
  std::string code = top->sourceCode->toStdUTF8String();
  std::stringstream ss(code);
  std::string line;
  for (size_t i = 1; std::getline(ss, line); i++) {
    if (i == top->loc.line) {
      break;
    }
  }

  return Utils::NewLocal<String>(
      lwIsolate->toV8(), StringRef::createFromASCII(line.data(), line.size()));
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
