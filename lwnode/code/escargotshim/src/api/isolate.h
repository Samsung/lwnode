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

#pragma once

#include "arraybuffer-allocator.h"
#include "engine.h"
#include "global-handles.h"
#include "handlescope.h"
#include "utils/compiler.h"
#include "utils/gc.h"

namespace v8 {
namespace internal {
class Isolate : public gc {
 public:
  void RegisterTryCatchHandler(v8::TryCatch* that);
  void UnregisterTryCatchHandler(v8::TryCatch* that);
  void SetTerminationOnExternalTryCatch();
  void ScheduleThrow(Escargot::ValueRef* value);
  bool IsExecutionTerminating();
  void CancelScheduledExceptionFromTryCatch(v8::TryCatch* that);
  void ThrowException(Escargot::ValueRef* value);

  TryCatch* try_catch_handler();

  Escargot::ValueRef* scheduled_exception();
  bool has_scheduled_exception();
  void clear_scheduled_exception();

  Escargot::ValueRef* pending_exception();
  bool has_pending_exception();

  Escargot::ValueRef* pending_message_obj();

  virtual void SetPendingExceptionAndMessage(
      Escargot::ValueRef* exception,
      GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData) = 0;
  bool PropagatePendingExceptionToExternalTryCatch();
  void ReportPendingMessages();

  virtual EscargotShim::ValueWrap* hole() = 0;
  virtual bool isHole(const EscargotShim::ValueWrap* wrap) = 0;
  virtual bool isHole(const Escargot::ValueRef* ref) = 0;

 protected:
  Escargot::ValueRef* scheduled_exception_{nullptr};

  void set_pending_exception(Escargot::ValueRef* exception_obj);
  void set_pending_message_obj(Escargot::ValueRef* message_obj);
  void clear_pending_exception();
  void clear_pending_message_obj();

  v8::TryCatch* getExternalTryCatchOnTop();
  bool hasExternalTryCatch();

 private:
  v8::TryCatch* try_catch_handler_{nullptr};
  Escargot::ValueRef* pending_exception_{nullptr};
  Escargot::ValueRef* pending_message_obj_{nullptr};
};
}  // namespace internal
}  // namespace v8

namespace EscargotShim {

class ContextWrap;

typedef gc GCManagedObject;

class IsolateWrap final : public v8::internal::Isolate {
 public:
  static IsolateWrap* New();
  void Initialize(const v8::Isolate::CreateParams& params);
  void Dispose();
  void Enter();
  void Exit();

  // Cast functions
  static v8::Isolate* toV8(IsolateWrap* iso) {
    return reinterpret_cast<v8::Isolate*>(iso);
  }
  static v8::Isolate* toV8(v8::internal::Isolate* iso) {
    return reinterpret_cast<v8::Isolate*>(iso);
  }

  static IsolateWrap* fromV8(v8::Isolate* iso) {
    return reinterpret_cast<IsolateWrap*>(iso);
  }
  static IsolateWrap* fromV8(v8::internal::Isolate* iso) {
    return reinterpret_cast<IsolateWrap*>(iso);
  }

  static IsolateWrap* fromEscargot(VMInstanceRef* vmInstance);

  v8::Isolate* toV8() { return reinterpret_cast<v8::Isolate*>(this); }

  // V8 internals
  void set_array_buffer_allocator_shared(
      std::shared_ptr<v8::ArrayBuffer::Allocator> allocator);
  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocator);

  v8::ArrayBuffer::Allocator* array_buffer_allocator() {
    return arrayBufferDecorator_->array_buffer_allocator();
  }

  ArrayBufferAllocatorDecorator* arrayBufferDecorator_ = nullptr;

  static IsolateWrap* GetCurrent();

  // HandleScope & Handle
  void pushHandleScope(HandleScopeWrap* handleScope);
  void popHandleScope(v8Scope_t* v8HandleScope);
  void addHandleToCurrentScope(HandleWrap* value);
  void escapeHandle(HandleWrap* value);
  bool isCurrentScopeSealed();

  // Context
  void pushContext(ContextWrap* context);
  void popContext(ContextWrap* context);
  bool InContext();
  ContextWrap* GetCurrentContext();

  // Eternal
  void addEternal(GCManagedObject* value);

  void addBackingStore(BackingStoreRef* value);
  void removeBackingStore(BackingStoreRef* value);

  VMInstanceRef* get() { return vmInstance_; }
  VMInstanceRef* vmInstance() { return vmInstance_; }

  void SetPromiseRejectCallback(v8::PromiseRejectCallback callback) {
    promise_reject_callback_ = callback;
  }

  void SetFatalErrorHandler(v8::FatalErrorCallback callback) {
    fatal_error_callback_ = callback;
  }

  void SetPrepareStackTraceCallback(v8::PrepareStackTraceCallback callback) {
    prepare_stack_trace_callback_ = callback;
  }

  void SetAbortOnUncaughtExceptionCallback(
      v8::Isolate::AbortOnUncaughtExceptionCallback callback) {
    abort_on_uncaught_exception_callback_ = callback;
  }

  void AddMessageListenerWithErrorLevel(v8::MessageCallback callback) {
    LWNODE_DCHECK_NULL(message_callback_);
    message_callback_ = callback;
  }

  ValueWrap** getGlobal(const int idex);
  ValueWrap* undefined();
  ValueWrap* hole() override;
  ValueWrap* null();
  ValueWrap* trueValue();
  ValueWrap* falseValue();
  ValueWrap* emptyString();
  ValueWrap* defaultReturnValue();

  bool isHole(const ValueWrap* wrap) override;
  bool isHole(const Escargot::ValueRef* ref) override;

  SymbolRef* getPrivateSymbol(StringRef* esString);

  GlobalHandles* globalHandles() { return &globalHandles_; }
  struct StackTraceData : public gc {
   public:
    StackTraceData(Escargot::Evaluator::StackTraceData& data)
        : src(data.src),
          sourceCode(data.sourceCode),
          loc(data.loc),
          functionName(data.functionName),
          isConstructor(data.isConstructor),
          isAssociatedWithJavaScriptCode(data.isAssociatedWithJavaScriptCode),
          isEval(data.isEval) {}

    StringRef* src{nullptr};
    StringRef* sourceCode{nullptr};
    Escargot::Evaluator::LOC loc{0, 0, 0};
    StringRef* functionName{nullptr};
    bool isFunction{false};
    bool isConstructor{false};
    bool isAssociatedWithJavaScriptCode{false};
    bool isEval{false};
  };

  struct ExceptionData : public gc {
    ValueRef* exception{nullptr};
    GCVector<StackTraceData*> stackTraces;

    void reset() {
      exception = nullptr;
      stackTraces.clear();
    }
  };

  GCVector<StackTraceData*>* stackTrace() {
    return &exceptionData_.stackTraces;
  }

  void SetPendingExceptionAndMessage(
      ValueRef* exception,
      GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData)
      override;
  void ClearPendingExceptionAndMessage();

  void onFatalError(const char* location, const char* message);

 private:
  IsolateWrap();

  void InitializeGlobalSlots();

  GCVector<GCManagedObject*> eternals_;
  GCUnorderedSet<BackingStoreRef*> backingStores_;
  GCVector<HandleScopeWrap*> handleScopes_;
  GCVector<ContextWrap*> contextScopes_;
  GCVector<Escargot::SymbolRef*> privateSymbols_;

  // Isolate Scope
  static THREAD_LOCAL IsolateWrap* s_currentIsolate;
  static THREAD_LOCAL IsolateWrap* s_previousIsolate;

  // V8 CreateParams
  v8::ArrayBuffer::Allocator* array_buffer_allocator_ = nullptr;
  std::shared_ptr<v8::ArrayBuffer::Allocator> array_buffer_allocator_shared_;

  VMInstanceRef* vmInstance_ = nullptr;

  GlobalHandles globalHandles_;

  PersistentRefHolder<IsolateWrap> release_lock_;

  void lock_gc_release() { release_lock_.reset(this); }
  void unlock_gc_release() { release_lock_.release(); }

  v8::PromiseRejectCallback promise_reject_callback_ = nullptr;
  v8::MessageCallback message_callback_ = nullptr;
  v8::FatalErrorCallback fatal_error_callback_ = nullptr;
  v8::PrepareStackTraceCallback prepare_stack_trace_callback_ = nullptr;
  v8::Isolate::AbortOnUncaughtExceptionCallback
      abort_on_uncaught_exception_callback_ = nullptr;

  ValueWrap* globalSlot_[internal::Internals::kRootIndexSize];

  ExceptionData exceptionData_;
};

}  // namespace EscargotShim
