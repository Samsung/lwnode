// Copyright Microsoft. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8-profiler.h"
#include "v8.h"

using namespace EscargotShim;

namespace v8 {

HeapProfiler dummyHeapProfiler;
CpuProfiler dummyCpuProfiler;

Isolate* Isolate::New(const CreateParams& params) {
  // TODO: Get only relevant params to EscargotShim
  EscargotShim::IsolateShim::CreateParams p;
  return EscargotShim::IsolateShim::New(p)->asIsolate();
}

Isolate* Isolate::GetCurrent() {
  return EscargotShim::IsolateShim::GetCurrent()->asIsolate();
}

void Isolate::Enter() {
  return EscargotShim::IsolateShim::ToIsolateShim(this)->Enter();
}

void Isolate::Exit() {
  return EscargotShim::IsolateShim::ToIsolateShim(this)->Exit();
}

void Isolate::Dispose() {
  auto isolateshim = EscargotShim::IsolateShim::ToIsolateShim(this);
  isolateshim->Dispose();
  delete isolateshim;
}

int64_t Isolate::AdjustAmountOfExternalAllocatedMemory(
    int64_t change_in_bytes) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(
      "We don't support adding external memory pressure at the moment");
  return 0;
}

void Isolate::SetData(uint32_t slot, void* data) {
  NESCARGOT_UNIMPLEMENTED("");
}

void* Isolate::GetData(uint32_t slot) {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

bool Isolate::RunSingleStepOfReverseMoveLoop(v8::Isolate* isolate,
                                             uint64_t* moveMode,
                                             int64_t* nextEventTime) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

uint32_t Isolate::GetNumberOfDataSlots() {
  return 0;
}

bool Isolate::InContext() {
  return !GetCurrentContext().IsEmpty();
}

Local<Context> Isolate::GetCurrentContext() {
  IsolateShim* isolateShim = EscargotShim::IsolateShim::ToIsolateShim(this);
  return Local<Context>::New(isolateShim->asIsolate(),
                             isolateShim->currentContext()->asContext());
}

void Isolate::SetPromiseRejectCallback(PromiseRejectCallback callback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(
      "process.on('unhandledRejection', ...) isn't supported as of now.");
}

void Isolate::SetPromiseHook(PromiseHook hook) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

bool Isolate::AddMessageListener(MessageCallback that, Handle<Value> data) {
  // Ignore data parameter.  Node doesn't use it.
  return EscargotShim::IsolateShim::ToIsolateShim(this)->AddMessageListener(
      reinterpret_cast<void*>(that));
}

void Isolate::RemoveMessageListeners(MessageCallback that) {
  EscargotShim::IsolateShim::ToIsolateShim(this)->RemoveMessageListeners(
      reinterpret_cast<void*>(that));
}

void Isolate::SetAbortOnUncaughtExceptionCallback(
    AbortOnUncaughtExceptionCallback callback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(
      "Ignoring since Escargot have no equivalent concept.");
}

void Isolate::SetFatalErrorHandler(FatalErrorCallback that) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(
      "Ignoring since Escargot have no equivalent concept.");
}

void Isolate::SetCaptureStackTraceForUncaughtExceptions(
    bool capture, int frame_limit, StackTrace::StackTraceOptions options) {
  // kpathak: you might want to look into mechanism of captureStackTrace in
  // chakra_shim.js.
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(" Figure out what to do here");
}

void Isolate::SetJitCodeEventHandler(JitCodeEventOptions options,
                                     JitCodeEventHandler event_handler) {
  // CHAKRA-TODO: This is for ETW events, we don't have equivalent but might not
  // need it because we do our own ETW tracing.
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("Figure out what to do here");
}

void Isolate::RunMicrotasks() {
  auto isolateshim = EscargotShim::IsolateShim::ToIsolateShim(this);
  while (isolateshim->HasPendingPromiseJob()) {
    auto result = isolateshim->ExecutePendingPromiseJob();
    VERIFY_EVAL_RESULT(result, );
  }
}

void Isolate::SetAutorunMicrotasks(bool autorun) {
  NESCARGOT_ASSERT(autorun == false);
}

Local<Value> Isolate::ThrowException(Local<Value> exception) {
  auto exceptionRef = exception->asJsValueRef()->asObject();
  IsolateShim::GetCurrent()->GetScriptException()->SetException(exceptionRef);

  return JsUndefined();
}

HeapProfiler* Isolate::GetHeapProfiler() {
  return &dummyHeapProfiler;
}

CpuProfiler* Isolate::GetCpuProfiler() {
  return &dummyCpuProfiler;
}

void Isolate::AddGCPrologueCallback(GCCallback callback,
                                    GCType gc_type_filter) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::AddGCPrologueCallback(GCCallbackWithData callback,
                                    void* data,
                                    GCType gc_type_filter) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::RemoveGCPrologueCallback(GCCallback callback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::RemoveGCPrologueCallback(GCCallbackWithData callback,
                                       void* data) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::AddGCEpilogueCallback(GCCallbackWithData callback,
                                    void* data,
                                    GCType gc_type_filter) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::AddGCEpilogueCallback(GCCallback callback,
                                    GCType gc_type_filter) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::RemoveGCEpilogueCallback(GCCallback callback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::RemoveGCEpilogueCallback(GCCallbackWithData callback,
                                       void* data) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::CancelTerminateExecution() {
  // jsrt::IsolateShim::FromIsolate(this)->EnableExecution();
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::RequestInterrupt(InterruptCallback callback, void* data) {
  // jsrt::IsolateShim::FromIsolate(this)->RequestInterrupt(callback, data);
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::TerminateExecution() {
  // jsrt::IsolateShim::FromIsolate(this)->DisableExecution();
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::RequestGarbageCollectionForTesting(GarbageCollectionType type) {
  // JsCollectGarbage(jsrt::IsolateShim::FromIsolate(this)->GetRuntimeHandle());
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetCounterFunction(CounterLookupCallback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::SetCreateHistogramFunction(CreateHistogramCallback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

void Isolate::SetAddHistogramSampleFunction(AddHistogramSampleCallback) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

bool Isolate::IdleNotificationDeadline(double deadline_in_seconds) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return false;
}

bool Isolate::IdleNotification(int idle_time_in_ms) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return false;
}

void Isolate::LowMemoryNotification() {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

int Isolate::ContextDisposedNotification() {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return 0;
}

void Isolate::GetHeapStatistics(HeapStatistics* heap_statistics) {
  // size_t memoryUsage;
  // if (!jsrt::IsolateShim::FromIsolate(this)->GetMemoryUsage(&memoryUsage)) {
  //   return;
  // }
  // // CONSIDER: V8 distinguishes between "total" size and "used" size
  // heap_statistics->set_heap_size(memoryUsage);
  NESCARGOT_UNIMPLEMENTED("");
}

size_t Isolate::NumberOfHeapSpaces() {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("not expose HEAP space stats");
  return 0;
}

bool Isolate::GetHeapSpaceStatistics(HeapSpaceStatistics* space_statistics,
                                     size_t index) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("not expose HEAP space stats");
  return true;
}

Isolate::DisallowJavascriptExecutionScope::DisallowJavascriptExecutionScope(
    Isolate* isolate, OnFailure on_failure) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("Figure out what to do here");
}

Isolate::DisallowJavascriptExecutionScope::~DisallowJavascriptExecutionScope() {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("Figure out what to do here");
}

bool Isolate::AddMessageListenerWithErrorLevel(MessageCallback that,
                                               int message_levels,
                                               Local<Value> data) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

void Isolate::SetPrepareStackTraceCallback(PrepareStackTraceCallback callback) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetAllowWasmCodeGenerationCallback(
    AllowWasmCodeGenerationCallback callback) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetHostCleanupFinalizationGroupCallback(
    HostCleanupFinalizationGroupCallback callback) {
  NESCARGOT_UNIMPLEMENTED("");
}

Isolate* Isolate::Allocate() {
  return EscargotShim::IsolateShim::Allocate()->asIsolate();
}

void Isolate::Initialize(Isolate* isolate, const CreateParams& params) {
  IsolateShim* isolateShim = EscargotShim::IsolateShim::ToIsolateShim(isolate);
  // TODO: get relevant params
  EscargotShim::IsolateShim::CreateParams p;
  p.array_buffer_allocator = params.array_buffer_allocator;
  EscargotShim::IsolateShim::Initialize(isolateShim, p);
}

ArrayBuffer::Allocator* Isolate::GetArrayBufferAllocator() {
  IsolateShim* isolateShim = EscargotShim::IsolateShim::ToIsolateShim(this);
  return isolateShim->array_buffer_allocator();
  // FIXME: Should return an allocator
}

void Isolate::SetIdle(bool is_idle) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::ClearKeptObjects() {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::EnqueueMicrotask(Local<Function> microtask) {
  NESCARGOT_UNIMPLEMENTED("");
}

Isolate::AllowJavascriptExecutionScope::AllowJavascriptExecutionScope(
    Isolate* isolate) {
  NESCARGOT_UNIMPLEMENTED("");
}

Isolate::AllowJavascriptExecutionScope::~AllowJavascriptExecutionScope() {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetHostImportModuleDynamicallyCallback(
    HostImportModuleDynamicallyCallback callback) {
  NESCARGOT_UNIMPLEMENTED("");
}

bool Isolate::GetHeapCodeAndMetadataStatistics(
    HeapCodeStatistics* code_statistics) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

void Isolate::AddNearHeapLimitCallback(v8::NearHeapLimitCallback callback,
                                       void* data) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetStackLimit(uintptr_t stack_limit) {
  NESCARGOT_UNIMPLEMENTED("");
}

void Isolate::SetHostInitializeImportMetaObjectCallback(
    HostInitializeImportMetaObjectCallback callback) {
  NESCARGOT_UNIMPLEMENTED("");
}

}  // namespace v8
