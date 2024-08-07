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

#include "api.h"
#include "base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {

// --- H a n d l e s ---
HandleScope::HandleScope(Isolate* isolate) {
  LWNODE_CALL_TRACE("%p", this);
  LWNODE_CALL_TRACE_INDENT();
  Initialize(isolate);

  IsolateWrap::fromV8(isolate_)->pushHandleScope(
      new HandleScopeWrap(this, HandleScopeWrap::Type::Normal));
}

void HandleScope::Initialize(Isolate* isolate) {
  isolate_ = reinterpret_cast<i::Isolate*>(isolate);
}

HandleScope::~HandleScope() {
  LWNODE_CALL_TRACE_UNINDENT();
  LWNODE_CALL_TRACE("%p", this);
  IsolateWrap::fromV8(isolate_)->popHandleScope(this);
}

void* HandleScope::operator new(size_t) {
  std::abort();
}
void* HandleScope::operator new[](size_t) {
  std::abort();
}
void HandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void HandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

int HandleScope::NumberOfHandles(Isolate* isolate) {
  LWNODE_RETURN_0;
}

i::Address* HandleScope::CreateHandle(i::Isolate* isolate, i::Address value) {
  auto lwIsolate = IsolateWrap::fromV8(isolate);

  if (lwIsolate->isCurrentScopeSealed()) {
    lwIsolate->onFatalError(
        __CODE_LOCATION__,
        "the current scope isn't allowed to allocate a handle");
  }

  if (value == 0) {
    return nullptr;
  }

  auto handle = reinterpret_cast<HandleWrap*>(value);

  LWNODE_CHECK(handle->isValid());

  switch (handle->location()) {
    case HandleWrap::Location::Local:
      lwIsolate->addHandleToCurrentScope(handle);
      return reinterpret_cast<i::Address*>(handle);

    case HandleWrap::Location::Strong:
    case HandleWrap::Location::Weak: {
      auto cloned = handle->clone(HandleWrap::Location::Local);
      lwIsolate->addHandleToCurrentScope(cloned);
      return reinterpret_cast<i::Address*>(cloned);
    }

    default:
      break;
  }

  LWNODE_CHECK_NOT_REACH_HERE();

  return reinterpret_cast<i::Address*>(value);
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate) {
  LWNODE_CALL_TRACE("%p", this);
  LWNODE_CALL_TRACE_INDENT();

  Initialize(v8_isolate);

  IsolateWrap::fromV8(v8_isolate)
      ->pushHandleScope(
          new HandleScopeWrap(this, HandleScopeWrap::Type::Escapable));
}

i::Address* EscapableHandleScope::Escape(i::Address* escape_value) {
  LWNODE_CALL_TRACE("%p", escape_value);

  IsolateWrap::fromV8(GetIsolate())->escapeHandle(VAL(escape_value));

  return escape_value;
}

void* EscapableHandleScope::operator new(size_t) {
  std::abort();
}
void* EscapableHandleScope::operator new[](size_t) {
  std::abort();
}
void EscapableHandleScope::operator delete(void*, size_t) {
  std::abort();
}
void EscapableHandleScope::operator delete[](void*, size_t) {
  std::abort();
}

SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)) {
  LWNODE_CALL_TRACE("%p", this);
  IsolateWrap::fromV8(isolate_)->pushHandleScope(
      new HandleScopeWrap(this, HandleScopeWrap::Type::Sealed));
}

SealHandleScope::~SealHandleScope() {
  LWNODE_CALL_TRACE();
  IsolateWrap::fromV8(isolate_)->popHandleScope(this);
}

void* SealHandleScope::operator new(size_t) {
  std::abort();
}
void* SealHandleScope::operator new[](size_t) {
  std::abort();
}
void SealHandleScope::operator delete(void*, size_t) {
  std::abort();
}
void SealHandleScope::operator delete[](void*, size_t) {
  std::abort();
}

void Context::Enter() {
  LWNODE_CALL_TRACE("%s", VAL(this)->getHandleInfoString().c_str());
  VAL(this)->context()->Enter();
}

void Context::Exit() {
  LWNODE_CALL_TRACE();
  VAL(this)->context()->Exit();
}

Context::BackupIncumbentScope::BackupIncumbentScope(
    Local<Context> backup_incumbent_context)
    : backup_incumbent_context_(backup_incumbent_context) {
  LWNODE_UNIMPLEMENT;
}

Context::BackupIncumbentScope::~BackupIncumbentScope() {
  LWNODE_UNIMPLEMENT;
}

uint32_t Context::GetNumberOfEmbedderDataFields() {
  return VAL(this)->context()->GetNumberOfEmbedderDataFields();
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
  auto lwIsolate = VAL(this)->context()->GetIsolate();
  return Utils::NewLocal<Value>(lwIsolate->toV8(),
                                VAL(this)->context()->GetEmbedderData(index));
}

void Context::SetEmbedderData(int index, v8::Local<Value> value) {
  VAL(this)->context()->SetEmbedderData(index, VAL(*value));
}

void* Context::SlowGetAlignedPointerFromEmbedderData(int index) {
  return VAL(this)->context()->GetAlignedPointerFromEmbedderData(index);
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  VAL(this)->context()->SetAlignedPointerInEmbedderData(index, value);
}
}  // namespace v8
