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

namespace EscargotShim {

class ContextWrap;

typedef gc GCManagedObject;

class IsolateWrap : public gc {
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
    return arrayBufferDecorator_;
  }

  ArrayBufferAllocatorDecorator* arrayBufferDecorator_ = nullptr;

  static IsolateWrap* GetCurrent();

  // HandleScope & Handle
  void pushHandleScope(v8::HandleScope* handleScope);
  void popHandleScope(v8::HandleScope* handleScope);
  void addHandle(HandleWrap* value);
  void escapeHandle(HandleWrap* value);

  // Context
  void pushContext(ContextWrap* context);
  void popContext(ContextWrap* context);
  bool InContext();
  ContextWrap* GetCurrentContext();

  // Eternal
  void addEternal(GCManagedObject* value);

  v8::TryCatch* try_catch_handler();
  void registerTryCatchHandler(v8::TryCatch* that);
  void unregisterTryCatchHandler(v8::TryCatch* that);
  void setTerminationOnExternalTryCatch();
  void scheduleThrow(Escargot::ValueRef* result);
  bool IsExecutionTerminating();

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
    set_abort_on_uncaught_exception_callback_ = callback;
  }

  void AddMessageListenerWithErrorLevel(v8::MessageCallback callback) {
    LWNODE_DCHECK_NULL(message_callback_);
    message_callback_ = callback;
  }

  ValueWrap* getGlobal(const int idex);
  ValueWrap* undefined();
  ValueWrap* hole();
  ValueWrap* null();
  ValueWrap* trueValue();
  ValueWrap* falseValue();
  ValueWrap* emptyString();
  ValueWrap* defaultReturnValue();

  SymbolRef* getPrivateSymbol(StringRef* esString);

  GlobalHandles* globalHandles() { return &globalHandles_; }

 private:
  IsolateWrap();

  void InitializeGlobalSlots();

  GCVector<GCManagedObject*> eternals_;
  GCVector<HandleScopeWrap*> handleScopes_;
  GCVector<ContextWrap*> contextScopes_;
  GCVector<Escargot::SymbolRef*> privateSymbols_;

  // Isolate Scope
  static THREAD_LOCAL IsolateWrap* s_currentIsolate;
  static THREAD_LOCAL IsolateWrap* s_previousIsolate;

  // V8 CreateParams
  v8::ArrayBuffer::Allocator* array_buffer_allocator_ = nullptr;
  std::shared_ptr<v8::ArrayBuffer::Allocator> array_buffer_allocator_shared_;
  v8::TryCatch* try_catch_handler_ = nullptr;
  Escargot::ValueRef* last_error_result = nullptr;
  bool has_scheduled_throw_ = false;

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
      set_abort_on_uncaught_exception_callback_ = nullptr;

  ValueWrap* globalSlot_[internal::Internals::kRootIndexSize];
};

}  // namespace EscargotShim
