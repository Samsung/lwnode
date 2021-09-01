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
#include "api.h"
#include "base.h"
#include "context.h"
#include "es-helper.h"
#include "extra-data.h"
#include "utils/gc.h"
#include "utils/misc.h"

namespace v8 {
namespace internal {

using namespace EscargotShim;

// 'exception_' is of type ValueWrap*. Ref: api-exception.cc
void Isolate::SetTerminationOnExternalTryCatch() {
  LWNODE_CALL_TRACE_ID(TRYCATCH, "try_catch_handler_: %p", try_catch_handler_);

  if (try_catch_handler_ == nullptr) {
    return;
  }
  try_catch_handler_->can_continue_ = false;
  try_catch_handler_->has_terminated_ = true;
  try_catch_handler_->exception_ = nullptr;
}

bool Isolate::IsExecutionTerminating() {
  // TODO: IMPLEMENT
  return false;
}

void Isolate::ScheduleThrow(Escargot::ValueRef* value) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  // TODO: There are two types of exception handling.
  // 1. An exception raised when it should not. Usually this happens
  // when we are using Escargot API, and caused by incorrect development
  // of our code and/or escargot, i.e., internal error.
  // 2. An exception raised by running external script. In this case,
  // it is the external developer's responsibility to handle an exception
  // using v8:tryCatch, etc. In this case, we should not do any exception
  // handling.

  set_scheduled_exception(value);

  // Note: No stack data exist
  GCManagedVector<Escargot::Evaluator::StackTraceData> stackTraceData;
  SetPendingExceptionAndMessage(value, stackTraceData);
  PropagatePendingExceptionToExternalTryCatch();
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
  // Note: we have no scheduled_exception, but have pending_exception only.
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

void Isolate::set_scheduled_exception(Escargot::ValueRef* exception_obj) {
  scheduled_exception_ = exception_obj;
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

void Isolate::set_pending_message_obj(Escargot::ValueRef* message_obj) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  pending_message_obj_ = message_obj;
}

void Isolate::clear_pending_message_obj() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  pending_message_obj_ = nullptr;
}

v8::TryCatch* Isolate::getExternalTryCatchOnTop() {
  return try_catch_handler();
}

bool Isolate::hasExternalTryCatch() {
  return (try_catch_handler() != nullptr);
}

bool Isolate::PropagatePendingExceptionToExternalTryCatch() {
  // Propagate the pending exception only when any try-catch exists.
  // If not exists, propagation pregress is done with no action.
  if (hasExternalTryCatch()) {
    v8::TryCatch* handler = getExternalTryCatchOnTop();
    handler->can_continue_ = true;
    handler->has_terminated_ = false;

    LWNODE_CHECK_NULL(handler->exception_);
    LWNODE_CHECK_NULL(handler->message_obj_);

    // gets the status changed from `pending` to `done' through
    // setting them to an external v8::TryCatch handler.
    handler->exception_ = reinterpret_cast<void*>(pending_exception_);
    handler->message_obj_ = reinterpret_cast<void*>(pending_message_obj_);
  }

  return true;
}

void Isolate::ReportPendingMessages() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);

  PropagatePendingExceptionToExternalTryCatch();

  bool should_report_exception = true;

  if (hasExternalTryCatch()) {
    should_report_exception = getExternalTryCatchOnTop()->is_verbose_;
  }

  // Actually report the message to all message handlers.
  if (should_report_exception) {
    v8::HandleScope scope(EscargotShim::IsolateWrap::toV8(this));

    v8::Local<v8::Value> exception = v8::Utils::NewLocal<v8::Value>(
        EscargotShim::IsolateWrap::toV8(this), pending_exception_);

    v8::Local<v8::Message> message = v8::Exception::CreateMessage(
        EscargotShim::IsolateWrap::toV8(this), exception);

    if (message_callback_ != nullptr) {
      message_callback_(message, exception);
    }
  }
}

void Isolate::RunPromiseHook(PromiseHookType type,
                             Escargot::PromiseObjectRef* promise,
                             Escargot::ValueRef* parent) {
  if (!promise_hook_ || !promise) {
    return;
  }

  promise_hook_(type,
                v8::Utils::ToLocal<Promise>(promise),
                v8::Utils::ToLocal<Value>(parent));
}

}  // namespace internal
}  // namespace v8

using namespace Escargot;

namespace EscargotShim {

THREAD_LOCAL IsolateWrap* IsolateWrap::s_currentIsolate;
THREAD_LOCAL IsolateWrap* IsolateWrap::s_previousIsolate;

IsolateWrap::IsolateWrap() {
  LWNODE_CALL_TRACE_ID(ISOWRAP, "malc: %p", this);

  globalHandles_ = new GlobalHandles(toV8());

  privateValuesSymbol_ = PersistentRefHolder<SymbolRef>(
      SymbolRef::create(StringRef::createFromASCII(PRIVATE_VALUES.data(),
                                                   PRIVATE_VALUES.length())));

  // NOTE: check lock_gc_release(); is needed (and where)
  // lock_gc_release();
  Memory::gcRegisterFinalizer(this, [](void* self) {
    reinterpret_cast<IsolateWrap*>(self)->~IsolateWrap();
  });

  MemoryUtil::gcSetWarningListener([](WarnEventType type) {
    switch (type) {
      case OUT_OF_MEMORY:
      case FAILED_TO_EXPAND_HEAP:
        if (s_currentIsolate) {
          s_currentIsolate->onFatalError(nullptr, "Out of memory");
        }
        break;
      default:
        break;
    }
  });
}

IsolateWrap::~IsolateWrap() {
  LWNODE_CALL_TRACE_ID(ISOWRAP, "free: %p", this);
  globalHandles_->Dispose();
  LWNODE_CALL_TRACE_GC_START();
  // NOTE: Called when this IsolateWrap is deallocated by gc
  LWNODE_CALL_TRACE_GC_END();
}

IsolateWrap* IsolateWrap::New() {
  IsolateWrap* isolate = new IsolateWrap();
  return isolate;
}

void IsolateWrap::Dispose() {
  LWNODE_CALL_TRACE_ID(ISOWRAP);
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
  LWNODE_CALL_TRACE_ID(ISOWRAP);
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

  vmInstance_ = VMInstanceRef::create();
  vmInstance_->setOnVMInstanceDelete([](VMInstanceRef* instance) {
    // Do Nothing
    LWNODE_CALL_TRACE_GC_START();
    // NOTE: Calls when vmInstance() is terminated. This happens before GC runs
    LWNODE_CALL_TRACE_GC_END();
  });

  InitializeGlobalSlots();

  scheduled_exception_ = hole()->value();

  // Register lwnode internal promise hook to create the internal field.
  LWNODE_ONCE(LWNODE_DLOG_INFO("v8::Promise::kEmbedderFieldCount: %d",
                               v8::Promise::kEmbedderFieldCount));
  if (v8::Promise::kEmbedderFieldCount > 0) {
    auto fn = [](ExecutionStateRef* state,
                 VMInstanceRef::PromiseHookType type,
                 PromiseObjectRef* promise,
                 ValueRef* parent) {
      // 1. create internal field on Init
      if (type == VMInstanceRef::PromiseHookType::Init) {
        LWNODE_DCHECK(v8::Promise::kEmbedderFieldCount > 0);
        if (ObjectRefHelper::getInternalFieldCount(promise) == 0) {
          ObjectRefHelper::setInternalFieldCount(promise,
                                                 Promise::kEmbedderFieldCount);
          /*
            @note
            In Node.js, promise internal field count is supposed to be set to 1.

            reference:
            - configure.py: v8_promise_internal_field_count = 1
            - tools/v8_gypfiles/features.gypi: V8_PROMISE_INTERNAL_FIELD_COUNT
            - deps/v8/src/heap/factory.cc: Factory::NewJSPromise()

            v8 sets Smi::zero() to promise's internal fields, which are:
              - Number from getInternalField
              - nullptr from GetAlignedPointerFromInternalField

            In lwnode, the returned value will be:
              - Undefined from getInternalField
              - nullptr from GetAlignedPointerFromInternalField

            The difference from getInternalField seems not that meaningful in
            Node.js.
          */
        }
      }

      // 2. run PromiseHook
      IsolateWrap::GetCurrent()->RunPromiseHook(
          (v8::PromiseHookType)type, promise, parent);
    };

    vmInstance_->registerPromiseHook(fn);
  }
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

  LWNODE_CALL_TRACE_ID(ISOWRAP);
  // TODO: remove the following line and simply pop the last
  handleScopes_.back()->clear();

  handleScopes_.pop_back();
}

void IsolateWrap::addHandleToCurrentScope(HandleWrap* value) {
  LWNODE_CALL_TRACE_ID(ISOWRAP, "%p", value);
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
  contextScopes_.push_back(context);
}

size_t IsolateWrap::getNumberOfContexts() {
  GCUnorderedSet<ContextWrap*> uniqueContexts;
  for (const auto& val : contextScopes_) {
    uniqueContexts.insert(val);
  }
  return uniqueContexts.size();
};

void IsolateWrap::popContext(ContextWrap* context) {
  LWNODE_CHECK(contextScopes_.back() == context);
  LWNODE_CALL_TRACE_ID(ISOWRAP,
                       "%p (%zu -> %zu)",
                       context,
                       contextScopes_.size(),
                       contextScopes_.size() - 1);
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
  LWNODE_CALL_TRACE_ID(ISOWRAP, "%p", value);
  eternals_.push_back(value);
}

void IsolateWrap::addBackingStore(BackingStoreRef* value) {
  backingStores_.insert(value);
}

void IsolateWrap::removeBackingStore(BackingStoreRef* value) {
  backingStores_.erase(value);
}

SymbolRef* IsolateWrap::createApiSymbol(StringRef* name) {
  // FIXME: Use a set
  auto newSymbol = SymbolRef::create(name);
  bool found = false;
  for (size_t i = 0; i < apiSymbols_.size(); i++) {
    if (apiSymbols_[i]->description()->equals(name)) {
      apiSymbols_[i] = newSymbol;
      found = true;
      break;
    }
  }

  if (!found) {
    apiSymbols_.push_back(newSymbol);
    LWNODE_DLOG_INFO("malc: api symbol: %s", name->toStdUTF8String().c_str());
  }

  return newSymbol;
}

SymbolRef* IsolateWrap::getApiSymbol(StringRef* name) {
  // FIXME: Use a set
  LWNODE_CALL_TRACE_ID(ISOWRAP);

  for (size_t i = 0; i < apiSymbols_.size(); i++) {
    if (apiSymbols_[i]->description()->equals(name)) {
      return apiSymbols_[i];
    }
  }

  return createApiSymbol(name);
}

SymbolRef* IsolateWrap::createApiPrivateSymbol(StringRef* name) {
  // FIXME: Use a set
  auto newSymbol = SymbolRef::create(name);
  bool found = false;
  for (size_t i = 0; i < apiPrivateSymbols_.size(); i++) {
    if (apiPrivateSymbols_[i]->description()->equals(name)) {
      apiPrivateSymbols_[i] = newSymbol;
      found = true;
      break;
    }
  }

  if (!found) {
    apiPrivateSymbols_.push_back(newSymbol);
    LWNODE_DLOG_INFO("malc: private symbol: %s",
                     name->toStdUTF8String().c_str());
  }

  return newSymbol;
}

SymbolRef* IsolateWrap::getApiPrivateSymbol(StringRef* name) {
  // FIXME: Use a set
  LWNODE_CALL_TRACE_ID(ISOWRAP);

  for (size_t i = 0; i < apiPrivateSymbols_.size(); i++) {
    if (apiPrivateSymbols_[i]->description()->equals(name)) {
      return apiPrivateSymbols_[i];
    }
  }

  return createApiPrivateSymbol(name);
}

void IsolateWrap::ClearPendingExceptionAndMessage() {
  clear_pending_exception();
  clear_pending_message_obj();
}

void IsolateWrap::CollectGarbage() {
  globalHandles_->PostGarbageCollectionProcessing();
}

void IsolateWrap::SetPendingExceptionAndMessage(
    ValueRef* exception,
    GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);

  if (exception->isObject()) {
    ObjectRefHelper::setExtraData(exception->asObject(),
                                  new ExceptionObjectData(stackTraceData));
  }

  set_pending_exception(exception);
  // @note
  // pending_message_obj: v8::Message isn't created. Instead v8::Message
  // uses `stackTraceData.stackTraces()` directly to make a result requested.
  set_pending_message_obj(nullptr);
}

void IsolateWrap::Throw(Escargot::ExecutionStateRef* state) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  LWNODE_DCHECK(has_scheduled_exception());

  auto exception = scheduled_exception();
  clear_scheduled_exception();
  if (hasExternalTryCatch()) {
    return;
  }
  state->throwException(exception);
}

ValueWrap** IsolateWrap::getGlobal(const int index) {
  LWNODE_CHECK(index < internal::Internals::kRootIndexSize);
  return &globalSlot_[index];
}

ValueWrap* IsolateWrap::undefined_value() {
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

void IsolateWrap::SetPromiseHook(v8::PromiseHook callback) {
  promise_hook_ = callback;

  if (v8::Promise::kEmbedderFieldCount > 0) {
    // @todo LWNODE_CHECK(isPromiseHookRegistered());
    return;
  }

  // @note
  // the following won't be used in Node.js since
  // v8::Promise::kEmbedderFieldCount will be set to 1.
  LWNODE_DCHECK(false);

  auto lwIsolate = GetCurrent();
  if (promise_hook_ == nullptr) {
    lwIsolate->vmInstance()->unregisterPromiseHook();
    return;
  }

  auto fn = [](ExecutionStateRef* state,
               VMInstanceRef::PromiseHookType type,
               PromiseObjectRef* promise,
               ValueRef* parent) {
    IsolateWrap::GetCurrent()->RunPromiseHook(
        (v8::PromiseHookType)type, promise, parent);
  };

  lwIsolate->vmInstance()->registerPromiseHook(fn);
}
}  // namespace EscargotShim
