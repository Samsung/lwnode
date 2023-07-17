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

thread_local int s_depth = 0;

// 'v8::TryCatch::exception_' is of type ObjectRef*.
static ObjectRef* fromV8Exception(void* exception) {
  return reinterpret_cast<ObjectRef*>(exception);
}

v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)),
      next_(isolate_->try_catch_handler()),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {
  ++s_depth;
  LWNODE_CALL_TRACE_ID(
      TRYCATCH, "depth: %d, this: %p, next: %p", s_depth, this, next_);

  ResetInternal();
  isolate_->RegisterTryCatchHandler(this);
}

v8::TryCatch::~TryCatch() {
  --s_depth;
  LWNODE_CALL_TRACE_ID(TRYCATCH, "depth: %d -> %d", s_depth + 1, s_depth);
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
      isolate_->RestorePendingMessageFromTryCatch(this);
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

  if (isolate_->sholdReportPendingMessage(is_verbose_)) {
    isolate_->ReportPendingMessages(true);
  }
}

void* v8::TryCatch::operator new(size_t) {
  std::abort();
}
void* v8::TryCatch::operator new[](size_t) {
  std::abort();
}
void v8::TryCatch::operator delete(void*, size_t) {
  std::abort();
}
void v8::TryCatch::operator delete[](void*, size_t) {
  std::abort();
}

bool v8::TryCatch::HasCaught() const {
  bool hasCaught = (exception_ != nullptr);

  LWNODE_CALL_TRACE_ID(TRYCATCH, "hasCaught: %b", hasCaught);

  return hasCaught;
}

bool v8::TryCatch::CanContinue() const {
  LWNODE_UNIMPLEMENT;
  return false;
}

bool v8::TryCatch::HasTerminated() const {
  return has_terminated_;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  if (!HasCaught()) {
    return v8::Local<v8::Value>();
  }
  rethrow_ = true;
  return v8::Utils::ToLocal<Value>(
      IsolateWrap::fromV8(isolate_)->undefined_value());
}

v8::Local<Value> v8::TryCatch::Exception() const {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  if (HasCaught()) {
    return v8::Utils::NewLocal<Value>(IsolateWrap::toV8(isolate_),
                                      fromV8Exception(exception_));
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
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  rethrow_ = false;
  isolate_->CancelScheduledExceptionFromTryCatch(this);
  ResetInternal();
}

void v8::TryCatch::ResetInternal() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  exception_ = nullptr;
  message_obj_ = nullptr;
}

void v8::TryCatch::SetVerbose(bool value) {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "%b", value);
  is_verbose_ = value;
}

bool v8::TryCatch::IsVerbose() const {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  return is_verbose_;
}

void v8::TryCatch::SetCaptureMessage(bool value) {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "%b", value);
  capture_message_ = value;
}

// --- M e s s a g e ---

static bool hasStackTraceData(ValueRef* exception) {
  if (!exception->isObject()) {
    return false;
  }

  auto stackTrace = ExceptionObjectData::stackTrace(exception->asObject());
  auto empty = stackTrace->empty();
  if (empty) {
    LWNODE_LOG_WARN("No StackTrace found");
  }
  return !empty;
}

Local<String> Message::Get() const {
  LWNODE_RETURN_LOCAL(String);
}

v8::Isolate* Message::GetIsolate() const {
  return IsolateWrap::GetCurrent()->toV8();
}

ScriptOrigin Message::GetScriptOrigin() const {
  v8::Isolate* isolate = GetIsolate();

  // todo: fill up the information if required
  Local<Value> resource_name =
      String::NewFromUtf8(isolate, "").ToLocalChecked();
  Local<Integer> resource_line_offset = Integer::New(isolate, 0);
  Local<Integer> resource_column_offset = Integer::New(isolate, 0);
  Local<Boolean> resource_is_shared_cross_origin = False(isolate);
  Local<Integer> script_id = Integer::New(isolate, 0);
  Local<Value> source_map_url = Undefined(isolate);
  Local<Boolean> resource_is_opaque = True(isolate);
  Local<Boolean> is_wasm = False(isolate);
  Local<Boolean> is_module = False(isolate);
  Local<PrimitiveArray> host_defined_options = PrimitiveArray::New(isolate, 0);

  return v8::ScriptOrigin(resource_name,
                          resource_line_offset,
                          resource_column_offset,
                          resource_is_shared_cross_origin,
                          script_id,
                          source_map_url,
                          resource_is_opaque,
                          is_wasm,
                          is_module,
                          host_defined_options);
}

v8::Local<Value> Message::GetScriptResourceName() const {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return Utils::NewLocal<String>(lwIsolate->toV8(), StringRef::emptyString());
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  return Utils::NewLocal<String>(lwIsolate->toV8(), top->src());
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  LWNODE_RETURN_LOCAL(StackTrace);
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return Just<int>(0);
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  return Just<int>(top->loc().line);
}

int Message::GetStartPosition() const {
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return 0;
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  return top->loc().index;
}

int Message::GetEndPosition() const {
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return 0;
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  return top->loc().index + 1;
}

int Message::ErrorLevel() const {
  /* @note
    Isolate::MessageErrorLevel::kMessageError and
    Isolate::MessageErrorLevel::kMessageWarning are used in Node.js.
    Only MessageErrorLevel::kMessageError is supported for now.
  */
  LWNODE_UNIMPLEMENT_WORKAROUND;
  return v8::Isolate::MessageErrorLevel::kMessageError;
}

int Message::GetStartColumn() const {
  return GetStartColumn(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(0);
}

int Message::GetWasmFunctionIndex() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return Nothing<int>();
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  return Just<int>(top->loc().column - 1);
}

int Message::GetEndColumn() const {
  return GetEndColumn(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(0);
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  auto self = CVAL(this)->value();

  if (!hasStackTraceData(self)) {
    return Nothing<int>();
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  int endCol = top->loc().column;

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
  auto self = CVAL(this)->value();
  if (!hasStackTraceData(self)) {
    return Utils::NewLocal<String>(lwIsolate->toV8(), StringRef::emptyString());
  }

  auto stackTrace = ExceptionObjectData::stackTrace(self->asObject());

  auto top = stackTrace->front();
  std::string code = top->sourceCode()->toStdUTF8String();
  std::stringstream ss(code);
  std::string line;
  for (size_t i = 1; std::getline(ss, line); i++) {
    if (i == top->loc().line) {
      break;
    }
  }

  return String::NewFromUtf8(
      lwIsolate->toV8(), line.data(), NewStringType::kNormal, line.size());
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
