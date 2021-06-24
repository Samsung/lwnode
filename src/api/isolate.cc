/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

#include "isolate.h"
#include "es-helper.h"
#include "utils/gc.h"
#include "utils/misc.h"

namespace v8 {
namespace internal {

// 'exception_' is of type ValueWrap*. Ref: api-exception.cc
void Isolate::SetTerminationOnExternalTryCatch() {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "try_catch_handler_: %p", try_catch_handler_);

  if (try_catch_handler_ == nullptr) {
    return;
  }
  try_catch_handler_->can_continue_ = false;
  try_catch_handler_->has_terminated_ = true;
  try_catch_handler_->exception_ =
      EscargotShim::ExceptionHelper::wrapException(scheduled_exception_);
}

bool Isolate::IsExecutionTerminating() {
  // TODO: IMPLEMENT
  return false;
}

void Isolate::ScheduleThrow(Escargot::ValueRef* result) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  // LWNODE_UNIMPLEMENT;
  // TODO: There are two types of exception handling.
  // 1. An exception raised when it should not. Usually this happens
  // when we are using Escargot API, and caused by incorrect development
  // of our code and/or escargot, i.e., internal error.
  // 2. An exception raised by running external script. In this case,
  // it is the external developer's responsibility to handle an exception
  // using v8:tryCatch, etc. In this case, we should not do any exception
  // handling.
  // TODO: Fix API_HANDLE_EXCEPTION() accordingly
  scheduled_exception_ = result;

  SetTerminationOnExternalTryCatch();
}

void Isolate::RegisterTryCatchHandler(v8::TryCatch* that) {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "%p", that);
  try_catch_handler_ = that;
}

void Isolate::UnregisterTryCatchHandler(v8::TryCatch* that) {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "%p -> %p", that, try_catch_handler_->next_);
  LWNODE_DCHECK(try_catch_handler_ == that);
  try_catch_handler_ = try_catch_handler_->next_;
}

void Isolate::CancelScheduledExceptionFromTryCatch(v8::TryCatch* that) {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "%p", that);
  LWNODE_DCHECK(has_scheduled_exception());
  if (scheduled_exception() ==
      EscargotShim::ExceptionHelper::unwrapException(that->exception_)) {
    clear_scheduled_exception();
  } else {
    // TODO
    LWNODE_RETURN_VOID;
  }
}

v8::TryCatch* Isolate::try_catch_handler() {
  return try_catch_handler_;
}

Escargot::ValueRef* Isolate::scheduled_exception() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  return scheduled_exception_;
}

bool Isolate::has_scheduled_exception() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  return !isHole(scheduled_exception_);
}

void Isolate::clear_scheduled_exception() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  scheduled_exception_ = hole()->value();
}

Escargot::ValueRef* Isolate::pending_exception() {
  LWNODE_CHECK(has_pending_exception());
  return pending_exception_;
}

void Isolate::set_pending_exception(Escargot::ValueRef* exception_obj) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  LWNODE_CHECK_NOT_NULL(exception_obj);
  pending_exception_ = exception_obj;
}

void Isolate::clear_pending_exception() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  pending_exception_ = nullptr;
}

bool Isolate::has_pending_exception() {
  return pending_exception_ != nullptr;
}

}  // namespace internal
}  // namespace v8

using namespace Escargot;

namespace EscargotShim {

THREAD_LOCAL IsolateWrap* IsolateWrap::s_currentIsolate;
THREAD_LOCAL IsolateWrap* IsolateWrap::s_previousIsolate;

IsolateWrap::IsolateWrap() {
  LWNODE_CALL_TRACE_2("malc: %p", this);

  // NOTE: check lock_gc_release(); is needed (and where)
  // lock_gc_release();
  Memory::gcRegisterFinalizer(this, [](void* self) {
    LWNODE_CALL_TRACE_2("free: %p", self);
    LWNODE_CALL_TRACE_GC_START();
    // NOTE: Called when this IsolateWrap is deallocated by gc
    LWNODE_CALL_TRACE_GC_END();
  });
}

IsolateWrap* IsolateWrap::New() {
  IsolateWrap* isolate = new IsolateWrap();
  return isolate;
}

void IsolateWrap::Dispose() {
  LWNODE_CALL_TRACE_GC_START();
  // NOTE: check unlock_gc_release(); is needed (and where)
  // unlock_gc_release();
  LWNODE_CALL_TRACE_GC_END();
}

void IsolateWrap::set_array_buffer_allocator(
    v8::ArrayBuffer::Allocator* allocator) {
  arrayBufferDecorator_ = new ArrayBufferAllocatorDecorator();
  arrayBufferDecorator_->set_array_buffer_allocator(allocator);
}

void IsolateWrap::set_array_buffer_allocator_shared(
    std::shared_ptr<v8::ArrayBuffer::Allocator> allocator) {
  array_buffer_allocator_shared_ = std::move(allocator);
}

void IsolateWrap::InitializeGlobalSlots() {
  globalSlot_[internal::Internals::kUndefinedValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::createUndefined());
  globalSlot_[internal::Internals::kTheHoleValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::createUndefined());
  globalSlot_[internal::Internals::kNullValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::createNull());
  globalSlot_[internal::Internals::kTrueValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::create(true));
  globalSlot_[internal::Internals::kFalseValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::create(false));
  globalSlot_[internal::Internals::kEmptyStringRootIndex] =
      EscargotShim::ValueWrap::createValue(StringRef::emptyString());
  globalSlot_[internal::Internals::kDefaultReturnValueRootIndex] =
      EscargotShim::ValueWrap::createValue(ValueRef::createUndefined());
}

void IsolateWrap::Initialize(const v8::Isolate::CreateParams& params) {
  IsolateWrap* isolate = this;

  LWNODE_CHECK(params.code_event_handler == nullptr ||
               params.snapshot_blob == nullptr ||
               params.counter_lookup_callback == nullptr ||
               params.create_histogram_callback == nullptr ||
               params.add_histogram_sample_callback == nullptr ||
               params.external_references == nullptr ||
               params.allow_atomics_wait == true ||
               params.only_terminate_in_safe_scope == false ||
               params.embedder_wrapper_type_index == -1 ||
               params.embedder_wrapper_object_index == -1);

  if (auto allocator = params.array_buffer_allocator_shared) {
    LWNODE_DLOG_WARN("check: params.array_buffer_allocator_shared exists");
    LWNODE_CHECK(params.array_buffer_allocator == nullptr ||
                 params.array_buffer_allocator == allocator.get());
    isolate->set_array_buffer_allocator(allocator.get());
    isolate->set_array_buffer_allocator_shared(std::move(allocator));
  } else {
    isolate->set_array_buffer_allocator(params.array_buffer_allocator);
  }

  LWNODE_CHECK_NOT_NULL(arrayBufferDecorator_->array_buffer_allocator());

  auto platform = Platform::GetInstance();
  platform->setAllocator(array_buffer_allocator());

  vmInstance_ = VMInstanceRef::create(platform);
  vmInstance_->setOnVMInstanceDelete([](VMInstanceRef* instance) {
    // Do Nothing
    LWNODE_CALL_TRACE_GC_START();
    // NOTE: Calls when vmInstance() is terminated. This happens before GC runs
    LWNODE_CALL_TRACE_GC_END();
  });

  InitializeGlobalSlots();

  scheduled_exception_ = hole()->value();
}

void IsolateWrap::Enter() {
  if (s_currentIsolate == this) {
    return;
  }

  LWNODE_CHECK(s_currentIsolate == nullptr && s_currentIsolate != this);

  s_previousIsolate = s_currentIsolate;
  s_currentIsolate = this;
}

IsolateWrap* IsolateWrap::fromEscargot(VMInstanceRef* vmInstance) {
  LWNODE_CHECK(s_currentIsolate->vmInstance() == vmInstance);
  return s_currentIsolate;
}

void IsolateWrap::Exit() {
  LWNODE_CHECK(s_currentIsolate == this);

  s_currentIsolate = s_previousIsolate;
  s_previousIsolate = nullptr;
}

IsolateWrap* IsolateWrap::GetCurrent() {
  return s_currentIsolate;
}

void IsolateWrap::pushHandleScope(HandleScopeWrap* handleScope) {
  handleScopes_.push_back(handleScope);
}

void IsolateWrap::popHandleScope(v8Scope_t* handleScope) {
  LWNODE_CHECK(handleScopes_.back()->v8Scope() == handleScope);

  LWNODE_CALL_TRACE_2();
  // TODO: remove the following line and simply pop the last
  handleScopes_.back()->clear();

  handleScopes_.pop_back();
}

void IsolateWrap::addHandleToCurrentScope(HandleWrap* value) {
  LWNODE_CHECK(handleScopes_.size() >= 1);
  handleScopes_.back()->add(value);
}

void IsolateWrap::escapeHandle(HandleWrap* value) {
  auto nHandleScopes = handleScopes_.size();
  LWNODE_CHECK(nHandleScopes > 1);

  auto last = handleScopes_.rbegin();

  LWNODE_CHECK((*last)->type() == HandleScopeWrap::Type::Escapable);

  if ((*last)->remove(value)) {
    (*(++last))->add(value);
  }
}

bool IsolateWrap::isCurrentScopeSealed() {
  LWNODE_CHECK(handleScopes_.size() > 0);
  return (handleScopes_.back()->type() == HandleScopeWrap::Type::Sealed);
}

void IsolateWrap::pushContext(ContextWrap* context) {
  LWNODE_CALL_TRACE_2("%p", context);

  if (contextScopes_.size() && (contextScopes_.back() != context)) {
    LWNODE_DLOG_WARN(R"(multiple contexts exist:
contextScopes_.back() != context means that we need to support multiple
contexts. In Node.js at this time, one main Context associated with the
Environment instance is used for most Node.js features (except writing
MessagePort objects.) So, on purpose, we don't store Object's creation
context which is related to Object::CreateContext().
@note: we may ignore this warning if cctest not related runs.)");
  }

  contextScopes_.push_back(context);
}

void IsolateWrap::popContext(ContextWrap* context) {
  LWNODE_CHECK(contextScopes_.back() == context);
  LWNODE_CALL_TRACE_2("%p", context);
  contextScopes_.pop_back();
}

bool IsolateWrap::InContext() {
  return !contextScopes_.empty();
}

ContextWrap* IsolateWrap::GetCurrentContext() {
  LWNODE_CHECK(contextScopes_.size() >= 1);
  return contextScopes_.back();
}

void IsolateWrap::addEternal(GCManagedObject* value) {
  LWNODE_CALL_TRACE_2("%p", value);
  eternals_.push_back(value);
}

void IsolateWrap::addBackingStore(BackingStoreRef* value) {
  backingStores_.insert(value);
}

void IsolateWrap::removeBackingStore(BackingStoreRef* value) {
  backingStores_.erase(value);
}

SymbolRef* IsolateWrap::getPrivateSymbol(StringRef* esString) {
  // @check replace this container if this function is called a lot.
  LWNODE_CALL_TRACE_2();

  for (size_t i = 0; i < privateSymbols_.size(); i++) {
    if (privateSymbols_[i]->description()->equals(esString)) {
      return privateSymbols_[i];
    }
  }

  auto newSymbol = SymbolRef::create(esString);
  privateSymbols_.push_back(newSymbol);

  LWNODE_DLOG_INFO("malc: private symbol: %s",
                   esString->toStdUTF8String().c_str());

  return newSymbol;
}

void IsolateWrap::ReportPendingMessages(
    ValueRef* exception,
    GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);

  exceptionData_.value = exception;
  for (size_t i = 0; i < stackTraceData.size(); i++) {
    exceptionData_.stackTraces.push_back(new StackTraceData(stackTraceData[i]));
  }

  // TODO: remove this and use handling trycatch chains
  ScheduleThrow(exception);

  set_pending_exception(exception);
}

ValueWrap** IsolateWrap::getGlobal(const int index) {
  LWNODE_CHECK(index < internal::Internals::kRootIndexSize);
  return &globalSlot_[index];
}

ValueWrap* IsolateWrap::undefined() {
  return globalSlot_[internal::Internals::kUndefinedValueRootIndex];
}

ValueWrap* IsolateWrap::hole() {
  return globalSlot_[internal::Internals::kTheHoleValueRootIndex];
}

ValueWrap* IsolateWrap::null() {
  return globalSlot_[internal::Internals::kNullValueRootIndex];
}

ValueWrap* IsolateWrap::trueValue() {
  return globalSlot_[internal::Internals::kTrueValueRootIndex];
}

ValueWrap* IsolateWrap::falseValue() {
  return globalSlot_[internal::Internals::kFalseValueRootIndex];
}

ValueWrap* IsolateWrap::emptyString() {
  return globalSlot_[internal::Internals::kEmptyStringRootIndex];
}

ValueWrap* IsolateWrap::defaultReturnValue() {
  return globalSlot_[internal::Internals::kDefaultReturnValueRootIndex];
}

bool IsolateWrap::isHole(const ValueWrap* wrap) {
  return globalSlot_[internal::Internals::kTheHoleValueRootIndex] == wrap;
}

bool IsolateWrap::isHole(const Escargot::ValueRef* ref) {
  return globalSlot_[internal::Internals::kTheHoleValueRootIndex]->value() ==
         ref;
}

void IsolateWrap::onFatalError(const char* location, const char* message) {
  if (fatal_error_callback_) {
    fatal_error_callback_(location, message);
  } else {
    if (location) {
      LWNODE_DLOG_ERROR("FATAL: %s at %s", message, location);
    } else {
      LWNODE_DLOG_ERROR("FATAL: %s", message);
    }
    LWNODE_CHECK(false);
  }
}

}  // namespace EscargotShim
