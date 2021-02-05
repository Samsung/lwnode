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
  Initialize(isolate);
}

void HandleScope::Initialize(Isolate* isolate) {
  isolate_ = (reinterpret_cast<i::Isolate*>(isolate));

  IsolateWrap::fromV8(isolate_)->pushHandleScope(this);
}

HandleScope::~HandleScope() {
  LWNODE_CALL_TRACE("%p", this);
  IsolateWrap::fromV8(isolate_)->popHandleScope(this);
}

void* HandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  // TODO: abort : only stack is available
  return new HandleScope(nullptr);
}
void* HandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new HandleScope(nullptr);
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
  auto handle = HandleScopeWrap::CreateHandle(
      IsolateWrap::fromV8(isolate), reinterpret_cast<HandleWrap*>(value));

  return reinterpret_cast<i::Address*>(handle);
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate)
    : HandleScope(v8_isolate) {
  LWNODE_CALL_TRACE("%p", this);
}

i::Address* EscapableHandleScope::Escape(i::Address* escape_value) {
  LWNODE_CALL_TRACE("%p", escape_value);

  IsolateWrap::fromV8(GetIsolate())->escapeHandle(VAL(escape_value));

  return escape_value;
}

void* EscapableHandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new EscapableHandleScope(nullptr);
}
void* EscapableHandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new EscapableHandleScope(nullptr);
}
void EscapableHandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void EscapableHandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)) {
  LWNODE_UNIMPLEMENT;
}

SealHandleScope::~SealHandleScope() {
  LWNODE_UNIMPLEMENT;
}

void* SealHandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new SealHandleScope(nullptr);
}
void* SealHandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new SealHandleScope(nullptr);
}
void SealHandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void SealHandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

void Context::Enter() {
  VAL(this)->context()->Enter();
}

void Context::Exit() {
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
  LWNODE_RETURN_0;
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
  LWNODE_RETURN_LOCAL(Value);
}

// template <typename T, typename F>
// inline T VALC(F* p) {
//   return reinterpret_cast<const ValueWrap*>(that);
// }

// #define VALCC(that) reinterpret_cast<const ValueWrap*>(
//                    const_cast<typename std::remove_const<F>::type*>(that))

void Context::SetEmbedderData(int index, v8::Local<Value> value) {
  auto v = VAL(*value);

  VAL(this)->context()->SetEmbedderData(index, v);
}

void* Context::SlowGetAlignedPointerFromEmbedderData(int index) {
  LWNODE_RETURN_NULLPTR;
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  LWNODE_RETURN_VOID;
}
}  // namespace v8
