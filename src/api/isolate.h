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

#include <EscargotPublic.h>
#include <v8.h>

#include "engine.h"
#include "handlescope.h"
#include "utils/compiler.h"
#include "utils/gc.h"

namespace EscargotShim {

class ContextWrap;

class IsolateWrap : public gc {
 public:
  virtual ~IsolateWrap();

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
      std::shared_ptr<v8::ArrayBuffer::Allocator> allocator) {
    array_buffer_allocator_shared_ = std::move(allocator);
  }
  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocator) {
    array_buffer_allocator_ = allocator;
  }
  v8::ArrayBuffer::Allocator* array_buffer_allocator() const {
    return array_buffer_allocator_;
  }

  static IsolateWrap* GetCurrent();

  // HandleScope & Handle
  void pushHandleScope(v8::HandleScope* handleScope);
  void popHandleScope(v8::HandleScope* handleScope);
  void addHandle(HandleWrap* value);
  void escapeHandle(HandleWrap* value);

  // Context
  void pushContext(ContextWrap* context);
  void popContext(ContextWrap* context);
  ContextWrap* GetCurrentContext();

  // Eternal
  void addEternal(Escargot::ValueRef* value);

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

  SymbolRef* getPrivateSymbol(StringRef* esString);

 private:
  IsolateWrap();

  GCVector<Escargot::ValueRef*> eternals_;
  GCVector<HandleScopeWrap*> handleScopes_;
  GCVector<ContextWrap*> contextScopes_;
  GCVector<Escargot::SymbolRef*> privateSymbols_;

  // Isolate Scope
  static THREAD_LOCAL IsolateWrap* s_currentIsolate;
  static THREAD_LOCAL IsolateWrap* s_previousIsolate;

  // V8 CreateParams
  v8::ArrayBuffer::Allocator* array_buffer_allocator_ = nullptr;
  std::shared_ptr<v8::ArrayBuffer::Allocator> array_buffer_allocator_shared_;

  bool hasScheduledThrow_ = false;

  VMInstanceRef* vmInstance_ = nullptr;

  v8::PromiseRejectCallback promise_reject_callback_ = nullptr;
  v8::MessageCallback message_callback_ = nullptr;
  v8::FatalErrorCallback fatal_error_callback_ = nullptr;
  v8::PrepareStackTraceCallback prepare_stack_trace_callback_ = nullptr;
  v8::Isolate::AbortOnUncaughtExceptionCallback
      set_abort_on_uncaught_exception_callback_ = nullptr;
};

}  // namespace EscargotShim
