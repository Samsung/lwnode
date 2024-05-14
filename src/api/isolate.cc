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
#include "utils/compiler.h"
#include "utils/gc-util.h"
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
  return scheduled_exception_ != nullptr;
}

void Isolate::set_scheduled_exception(Escargot::ValueRef* exception_obj) {
  scheduled_exception_ = exception_obj;
}

void Isolate::clear_scheduled_exception() {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  scheduled_exception_ = nullptr;
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

    if (handler->exception_) {
      LWNODE_CALL_TRACE_ID(TRYCATCH,
                           "The previous exception has not yet been handled.");
      return false;
    }

    handler->can_continue_ = true;
    handler->has_terminated_ = false;

    // gets the status changed from `pending` to `done' through
    // setting them to an external v8::TryCatch handler.
    handler->exception_ = reinterpret_cast<void*>(pending_exception_);
    handler->message_obj_ = reinterpret_cast<void*>(pending_message_obj_);

    return true;
  }

  return false;
}

void Isolate::ReportPendingMessages(bool isVerbose) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);

  bool should_report_exception = true;
  auto pendingException = pending_exception();

  if (!isVerbose) {
    PropagatePendingExceptionToExternalTryCatch();

    if (try_catch_handler() != nullptr) {
      should_report_exception = getExternalTryCatchOnTop()->is_verbose_;
    }

    clear_pending_exception();

    if (pending_exception_ == scheduled_exception_) {
      clear_scheduled_exception();
    }
  }

  // Actually report the message to all message handlers.
  if (isVerbose || should_report_exception) {
    v8::HandleScope scope(EscargotShim::IsolateWrap::toV8(this));

    v8::Local<v8::Value> exception = v8::Utils::NewLocal<v8::Value>(
        EscargotShim::IsolateWrap::toV8(this), pendingException);

    v8::Local<v8::Message> message = v8::Exception::CreateMessage(
        EscargotShim::IsolateWrap::toV8(this), exception);

    if (message_callback_ != nullptr) {
      message_callback_(message, exception);
    }
  }
}

void Isolate::RestorePendingMessageFromTryCatch(v8::TryCatch* handler) {
  LWNODE_DCHECK(handler == try_catch_handler());
  LWNODE_DCHECK(handler->HasCaught());
  LWNODE_DCHECK(handler->rethrow_);
  LWNODE_DCHECK(handler->capture_message_);
  LWNODE_DCHECK(!has_pending_exception());

  set_pending_exception(VAL(*handler->Exception())->value());
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

ValueRef* Isolate::RunPrepareStackTraceCallback(ExecutionStateRef* state,
                                                ContextWrap* lwContext,
                                                ValueRef* error,
                                                ArrayObjectRef* sites) {
  if (prepare_stack_trace_callback_) {
    LWNODE_CALL_TRACE_ID_LOG(STACKTRACE,
                             "RunPrepareStackTraceCallback: %p",
                             PrepareStackTraceCallback());

    auto v8Isolate = lwContext->GetIsolate()->toV8();
    v8::MaybeLocal<v8::Value> maybyResult = prepare_stack_trace_callback_(
        v8::Utils::NewLocal<Context>(v8Isolate, lwContext),
        v8::Utils::NewLocal<Value>(v8Isolate, error),
        v8::Utils::NewLocal<Array>(v8Isolate, sites));

    if (!maybyResult.IsEmpty()) {
      Local<Value> v8Result;
      if (maybyResult.ToLocal(&v8Result)) {
        return CVAL(*v8Result)->value();
      }
    }
  }

  return ValueRef::createUndefined();
}

bool Isolate::hasCallDepth() {
  return callDepth() > 0;
}

bool Isolate::sholdReportPendingMessage(bool isVerbose) {
  if (has_pending_exception() && (callDepth() == 0 || isVerbose)) {
    return true;
  }
  return false;
}

}  // namespace internal
}  // namespace v8

using namespace Escargot;

namespace EscargotShim {

/*
  Note: why should these global variables be exported?

  v8 uses std::unique_ptr or std::shared_ptr as a holder when a BackingStore is
  transferred. In order to keep or destroy the BackingStore at right time, we
  use the `BackingStore` destructor.

  v8::BackingStore::~BackingStore() {
    auto lwIsolate = IsolateWrap::GetCurrent(); // <-
    ...
  }

  Since the holder is outside this module, if the global variables aren't
  exported, they will be invisible when linked as shared library.
*/
LWNODE_EXPORT THREAD_LOCAL IsolateWrap* IsolateWrap::s_currentIsolate;
LWNODE_EXPORT THREAD_LOCAL IsolateWrap* IsolateWrap::s_previousIsolate;

IsolateWrap::IsolateWrap() {
  LWNODE_CALL_TRACE_ID(ISOWRAP, "malc: %p", this);

  global_handles_ = new GlobalHandles(this);

  privateValuesSymbol_ = PersistentRefHolder<SymbolRef>(
      SymbolRef::create(StringRef::createFromUTF8(PRIVATE_VALUES.data(),
                                                  PRIVATE_VALUES.length())));

  threadManager_ = new ThreadManager();

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

  RegisterExtension(std::make_unique<ExternalizeStringExtension>());
  RegisterExtension(std::make_unique<ExternalizeGcExtension>());

  state_ = State::Active;
}

IsolateWrap::~IsolateWrap() {
  LWNODE_CALL_TRACE_ID(ISOWRAP, "free: %p", this);
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

  global_handles()->dispose();
  RegisteredExtension::unregisterAll();

  state_ = State::Disposed;

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

  vmInstance_->registerErrorCreationCallback(
      [](ExecutionStateRef* state, ErrorObjectRef* error) {
        ExceptionHelper::addStackPropertyCallback(state, error);
      });

  InitializeGlobalSlots();

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
        // In Node.js, a promise always sets the internal field count to 1.
        //
        // reference:
        // - configure.py: v8_promise_internal_field_count = 1
        // - tools/v8_gypfiles/features.gypi: V8_PROMISE_INTERNAL_FIELD_COUNT
        // - deps/v8/src/heap/factory.cc: Factory::NewJSPromise()
        //
        // v8 initializes the promise's internal fields with Smi::zero().
        // These values are interpreted differently depending on getters.
        //   - Number is returned from getInternalField()
        //   - nullptr is returned from GetAlignedPointerFromInternalField()
        //
        // In constrast, lwnode interprets the same values as follows.
        //   - Undefined is returned from getInternalField()
        //   - nullptr is returned from GetAlignedPointerFromInternalField()
        //
        // This difference does not seem to make a different behaviour in
        // node.js
        //
        // This promise was newly created by Escargot internally
        LWNODE_CHECK(!ExtraDataHelper::getExtraData(promise));
        ExtraDataHelper::setExtraData(promise, new ObjectData());
        ExtraDataHelper::getExtraData(promise)
            ->asInternalFieldData()
            ->setInternalFieldCount(v8::Promise::kEmbedderFieldCount);
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
  auto itr = backingStoreCounter_.find(value);
  if (itr != backingStoreCounter_.end()) {
    ++itr->second;
  } else {
    backingStoreCounter_.insert(std::make_pair(value, 1));
  }
}

void IsolateWrap::removeBackingStore(BackingStoreRef* value) {
  auto itr = backingStoreCounter_.find(value);
  if (itr != backingStoreCounter_.end()) {
    if (itr->second == 1) {
      backingStoreCounter_.erase(itr);
    } else {
      --itr->second;
    }
  } else {
    LWNODE_CHECK_MSG(false, "increment/decrement count do not match");
  }
}

SymbolRef* IsolateWrap::createApiSymbol(StringRef* name) {
  // FIXME: Use a set
  auto newSymbol = SymbolRef::create(name);
  bool found = false;
  for (size_t i = 0; i < apiSymbols_.size(); i++) {
    if (apiSymbols_[i]->descriptionString()->equals(name)) {
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

  for (auto apiSymbols : apiSymbols_) {
    if (apiSymbols->descriptionString()->equals(name)) {
      return apiSymbols;
    }
  }

  return createApiSymbol(name);
}

SymbolRef* IsolateWrap::createApiPrivateSymbol(StringRef* name) {
  // FIXME: Use a set
  auto newSymbol = SymbolRef::create(name);
  bool found = false;
  for (size_t i = 0; i < apiPrivateSymbols_.size(); i++) {
    if (apiPrivateSymbols_[i]->descriptionString()->equals(name)) {
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

  for (auto apiPrivateSymbol : apiPrivateSymbols_) {
    if (apiPrivateSymbol->descriptionString()->equals(name)) {
      return apiPrivateSymbol;
    }
  }

  return createApiPrivateSymbol(name);
}

void IsolateWrap::ClearPendingExceptionAndMessage() {
  clear_pending_exception();
  clear_pending_message_obj();
}

void IsolateWrap::CollectGarbage(GarbageCollectionReason reason) {
  if (reason == GarbageCollectionReason::kTesting) {
    global_handles_->releaseWeakValues();
  } else {
    global_handles_->PostGarbageCollectionProcessing();
  }
}

void IsolateWrap::ScheduleThrow(Escargot::ValueRef* value) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  // TODO: There are two types of exception handling.
  // 1. An exception raised when it should not. Usually this happens
  // when we are using Escargot API, and caused by incorrect development
  // of our code and/or escargot, i.e., internal error.
  // 2. An exception raised by running external script. In this case,
  // it is the external developer's responsibility to handle an exception
  // using v8:tryCatch, etc. In this case, we should not do any exception
  // handling.

  bool rethrow = has_pending_exception();

  set_scheduled_exception(value);
  set_pending_exception(value);

  if (PropagatePendingExceptionToExternalTryCatch()) {
    clear_pending_exception();
    if (!rethrow) {
      clear_scheduled_exception();
    }
  }
}

void IsolateWrap::handleException(EscargotShim::EvalResult&& evalResult) {
  LWNODE_DCHECK(!evalResult.isSuccessful());

  auto exception = evalResult.error.get();

  if (hasCallDepth()) {
    if (exception->isObject()) {
      ExtraDataHelper::setExtraData(
          exception->asObject(),
          new ExceptionObjectData(evalResult.stackTrace));
    }
    ScheduleThrow(evalResult.error.get());
  } else {
    SetPendingExceptionAndMessage(exception, evalResult.stackTrace);
    ReportPendingMessages();
  }
}

void IsolateWrap::SetPendingExceptionAndMessage(
    ValueRef* exception,
    GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);

  if (exception->isObject()) {
    auto extraData = ExtraDataHelper::getExtraData(exception->asObject());

    if (extraData) {
      LWNODE_CHECK(extraData->isExceptionObjectData());
      // NOTE: Exception has created in the `else` below.
      LWNODE_LOG_WARN("esException already contains an extraData: %p\n",
                      extraData);
    } else {
      ExtraDataHelper::setExtraData(exception->asObject(),
                                    new ExceptionObjectData(stackTraceData));
    }
  }

  set_pending_exception(exception);
  // @note
  // pending_message_obj: v8::Message isn't created. Instead v8::Message
  // uses `stackTraceData.stackTraces()` directly to make a result requested.
  set_pending_message_obj(nullptr);
}

static bool isCatchableByJavascript(Escargot::ExecutionStateRef* state) {
  OptionalRef<ExecutionStateRef> e = state;
  while (e) {
    if (e->onTry()) {
      return true;
    }
    e = e->parent();
  }
  return false;
}

void IsolateWrap::ThrowErrorIfHasException(Escargot::ExecutionStateRef* state) {
  LWNODE_CALL_TRACE_ID(TRYCATCH);
  if (!has_scheduled_exception()) {
    return;
  }

  auto exception = scheduled_exception();
  clear_scheduled_exception();

  if (isCatchableByJavascript(state)) {
    ClearPendingExceptionAndMessage();
    if (hasExternalTryCatch()) {
      SetTerminationOnExternalTryCatch();
    }
    state->throwException(exception);
    return;
  } else if (!hasExternalTryCatch()) {
    ReportPendingMessages();
    return;
  }

  if (has_pending_exception()) {
    ReportPendingMessages();
  }
}

ValueWrap** IsolateWrap::getGlobal(const int index) {
  LWNODE_CHECK(index < internal::Internals::kRootIndexSize);
  return &globalSlot_[index];
}

ValueWrap* IsolateWrap::undefined_value() {
  return globalSlot_[internal::Internals::kUndefinedValueRootIndex];
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

  if (promise_hook_ == nullptr) {
    vmInstance()->unregisterPromiseHook();
    return;
  }

  auto fn = [](ExecutionStateRef* state,
               VMInstanceRef::PromiseHookType type,
               PromiseObjectRef* promise,
               ValueRef* parent) {
    IsolateWrap::GetCurrent()->RunPromiseHook(
        (v8::PromiseHookType)type, promise, parent);
  };

  vmInstance()->registerPromiseHook(fn);
}

void IsolateWrap::SetPromiseRejectCallback(v8::PromiseRejectCallback callback) {
  promise_reject_callback_ = callback;

  auto fn = [](ExecutionStateRef* state,
               Escargot::PromiseObjectRef* promise,
               Escargot::ValueRef* value,
               Escargot::VMInstanceRef::PromiseRejectEvent event) {
    // first check stack and trigger an exception if stack is already full
    state->checkStackOverflow();
    // then, temporally disable stack overflow to execute the callback without
    // any exception
    StackOverflowDisabler disabler(state);
    IsolateWrap::GetCurrent()->ReportPromiseReject(promise, value, event);
  };

  vmInstance()->registerPromiseRejectCallback(fn);
}

void IsolateWrap::ReportPromiseReject(
    Escargot::PromiseObjectRef* promise,
    Escargot::ValueRef* value,
    Escargot::VMInstanceRef::PromiseRejectEvent event) {
  PromiseRejectMessage v8Message(v8::Utils::ToLocal<Promise>(promise),
                                 static_cast<v8::PromiseRejectEvent>(event),
                                 v8::Utils::ToLocal<Value>(value));
  if (promise_reject_callback_ && !promise->hasRejectHandlers()) {
    promise_reject_callback_(v8Message);
  }
}

}  // namespace EscargotShim
