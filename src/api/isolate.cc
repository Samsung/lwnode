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
#include "utils/gc.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

THREAD_LOCAL IsolateWrap* IsolateWrap::s_currentIsolate;
THREAD_LOCAL IsolateWrap* IsolateWrap::s_previousIsolate;

IsolateWrap::IsolateWrap() {
  LWNODE_CALL_TRACE("malc: %p", this);

  lock_gc_release();

  Memory::gcRegisterFinalizer(
      this, [](void* self) { LWNODE_CALL_TRACE("free: %p", self); });
}

IsolateWrap* IsolateWrap::New() {
  IsolateWrap* isolate = new IsolateWrap();
  return isolate;
}

void IsolateWrap::Dispose() {
  unlock_gc_release();
  MemoryUtil::gcFull();
}

bool IsolateWrap::IsExecutionTerminating() {
  if (hasScheduledThrow_) {
    return true;
  }
  return false;
}

void IsolateWrap::scheduleThrow(Escargot::ValueRef* result) {
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
  hasScheduledThrow_ = true;
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

  vmInstance_ = VMInstanceRef::create(new Platform(array_buffer_allocator()));
  vmInstance_->setOnVMInstanceDelete(
      [](VMInstanceRef* instance) { delete instance->platform(); });

  InitializeGlobalSlots();
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

void IsolateWrap::pushHandleScope(v8::HandleScope* handleScope) {
  handleScopes_.push_back(new HandleScopeWrap(handleScope));
}

void IsolateWrap::popHandleScope(v8::HandleScope* handleScope) {
  LWNODE_CHECK(handleScopes_.back()->v8HandleScope() == handleScope);

  LWNODE_CALL_TRACE();
  handleScopes_.back()->clear();

  handleScopes_.pop_back();
}

void IsolateWrap::addHandle(HandleWrap* value) {
  LWNODE_CHECK(handleScopes_.size() >= 1);
  handleScopes_.back()->add(value);
}

void IsolateWrap::escapeHandle(HandleWrap* value) {
  auto nHandleScopes = handleScopes_.size();
  LWNODE_CHECK(nHandleScopes > 1);

  auto last = handleScopes_.rbegin();

  if ((*last)->remove(value)) {
    (*(++last))->add(value);
  }
}

void IsolateWrap::pushContext(ContextWrap* context) {
  LWNODE_CALL_TRACE("%p", context);
  contextScopes_.push_back(context);
}

void IsolateWrap::popContext(ContextWrap* context) {
  LWNODE_CHECK(contextScopes_.back() == context);
  LWNODE_CALL_TRACE("%p", context);
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
  LWNODE_CALL_TRACE("%p", value);
  eternals_.push_back(value);
}

SymbolRef* IsolateWrap::getPrivateSymbol(StringRef* esString) {
  // @check replace this container if this function is called a lot.
  LWNODE_CALL_TRACE();

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

ValueWrap* IsolateWrap::getGlobal(const int index) {
  LWNODE_CHECK(index < internal::Internals::kRootIndexSize);
  return globalSlot_[index];
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

}  // namespace EscargotShim
