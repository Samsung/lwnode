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
  // GCVector<HandleScopeWrap*> m_handleScopes;
  // GCVector<ContextWrap*> m_contexts;
  // m_pureContext.release();
  // m_vmInstance.release();
}

bool IsolateWrap::IsExecutionTerminating() {
  if (m_hasScheduledThrow) {
    return true;
  }
  return false;
}

void IsolateWrap::scheduleThrow(Escargot::ValueRef* result) {
  LWNODE_UNIMPLEMENT;
  m_hasScheduledThrow = true;
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

  LWNODE_CHECK_NOT_NULL(m_array_buffer_allocator);

  m_vmInstance = VMInstanceRef::create(new Platform(m_array_buffer_allocator));
  m_vmInstance->setOnVMInstanceDelete(
      [](VMInstanceRef* instance) { delete instance->platform(); });

  // @note any execution upon this context is NOT allowed. It intends for
  // compiling source only.
  m_pureContext = ContextRef::create(m_vmInstance);
}

void IsolateWrap::Enter() {
  if (s_currentIsolate == this) {
    return;
  }

  LWNODE_CHECK(s_currentIsolate == nullptr && s_currentIsolate != this);

  s_previousIsolate = s_currentIsolate;
  s_currentIsolate = this;
}

void IsolateWrap::Exit() {
  LWNODE_CHECK(s_currentIsolate == this);

  s_currentIsolate = s_previousIsolate;
  s_previousIsolate = nullptr;
}

IsolateWrap* IsolateWrap::currentIsolate() {
  return s_currentIsolate;
}

void IsolateWrap::pushHandleScope(v8::HandleScope* handleScope) {
  m_handleScopes.push_back(new HandleScopeWrap(handleScope));
}

void IsolateWrap::popHandleScope(v8::HandleScope* handleScope) {
  LWNODE_CHECK(m_handleScopes.back()->v8HandleScope() == handleScope);

  m_handleScopes.pop_back();
}

void IsolateWrap::escapeHandleFromCurrentHandleScope(HandleWrap* value) {
  LWNODE_CHECK(m_handleScopes.size() > 1);

  auto last = m_handleScopes.rbegin();
  (*(++last))->add(value);
}

void IsolateWrap::addHandleToCurrentHandleScope(HandleWrap* value) {
  LWNODE_CHECK(m_handleScopes.size() >= 1);

  m_handleScopes.back()->add(value);
}

void IsolateWrap::pushContext(ContextWrap* context) {
  m_contexts.push_back(context);
}

void IsolateWrap::popContext(ContextWrap* context) {
  LWNODE_CHECK(m_contexts.back() == context);
  m_contexts.pop_back();
}

ContextWrap* IsolateWrap::CurrentContext() {
  return m_contexts.back();
}

void IsolateWrap::addEternal(ValueRef* value) {
  eternals_.push_back(value);
}

}  // namespace EscargotShim
