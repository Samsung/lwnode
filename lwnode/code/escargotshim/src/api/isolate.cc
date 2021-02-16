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
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

THREAD_LOCAL IsolateWrap* IsolateWrap::s_currentIsolate;
THREAD_LOCAL IsolateWrap* IsolateWrap::s_previousIsolate;

IsolateWrap::IsolateWrap() {}

IsolateWrap::~IsolateWrap() {}

IsolateWrap* IsolateWrap::New() {
  IsolateWrap* isolate = new IsolateWrap();
  return isolate;
}

void IsolateWrap::Dispose() {
  // @check
  // GCVector<HandleScopeWrap*> handleScopes_;
  // GCVector<ContextWrap*> contexts_;
  // vmInstance_.release();
}

bool IsolateWrap::IsExecutionTerminating() {
  if (hasScheduledThrow_) {
    return true;
  }
  return false;
}

void IsolateWrap::scheduleThrow(Escargot::ValueRef* result) {
  LWNODE_UNIMPLEMENT;
  hasScheduledThrow_ = true;
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
    LWNODE_CHECK(params.array_buffer_allocator == nullptr ||
                 params.array_buffer_allocator == allocator.get());
    isolate->set_array_buffer_allocator(allocator.get());
    isolate->set_array_buffer_allocator_shared(std::move(allocator));
  } else {
    isolate->set_array_buffer_allocator(params.array_buffer_allocator);
  }

  LWNODE_CHECK_NOT_NULL(array_buffer_allocator_);

  vmInstance_ = VMInstanceRef::create(new Platform(array_buffer_allocator_));
  vmInstance_->setOnVMInstanceDelete(
      [](VMInstanceRef* instance) { delete instance->platform(); });
}

void IsolateWrap::Enter() {
  if (s_currentIsolate == this) {
    return;
  }

  LWNODE_CHECK(s_currentIsolate == nullptr && s_currentIsolate != this);

  s_previousIsolate = s_currentIsolate;
  s_currentIsolate = this;
}

IsolateWrap* IsolateWrap::fromEscargot(VMInstanceRef* vmInstance)
{
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

  handleScopes_.pop_back();
}

void IsolateWrap::escapeHandle(HandleWrap* value) {
  LWNODE_CHECK(handleScopes_.size() > 1);

  auto last = handleScopes_.rbegin();

  if ((*last)->remove(value)) {
    (*(++last))->add(value);
  }
}

void IsolateWrap::addHandle(HandleWrap* value) {
  LWNODE_CHECK(handleScopes_.size() >= 1);

  handleScopes_.back()->add(value);
}

void IsolateWrap::pushContext(ContextWrap* context) {
  contextScopes_.push_back(context);
}

void IsolateWrap::popContext(ContextWrap* context) {
  LWNODE_CHECK(contextScopes_.back() == context);
  contextScopes_.pop_back();
}

ContextWrap* IsolateWrap::GetCurrentContext() {
  LWNODE_CHECK(contextScopes_.size() >= 1);
  return contextScopes_.back();
}

void IsolateWrap::addEternal(ValueRef* value) {
  eternals_.push_back(value);
}

SymbolRef* IsolateWrap::getPrivateSymbol(StringRef* esString) {
  // @check replace this container if this function is called a lot.
  // LWNODE_CALL_TRACE();

  for (size_t i = 0; i < privateSymbols_.size(); i++) {
    if (privateSymbols_[i]->description()->equals(esString)) {
      return privateSymbols_[i];
    }
  }

  auto newSymbol = SymbolRef::create(esString);
  privateSymbols_.push_back(newSymbol);

  LWNODE_DLOG_INFO("private symbol: %s", esString->toStdUTF8String().c_str());

  return newSymbol;
}

}  // namespace EscargotShim
