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

#include "utils/gc.h"
#include "utils/compiler.h"
#include "handlescope.h"

namespace EscargotShim {

// class HandleScopeManager

class Isolate : public gc {
 public:
  virtual ~Isolate();

  static Isolate* New();
  void Initialize(const v8::Isolate::CreateParams& params);
  void Enter();
  void Exit();

  // HandleScope
  void PushHandleScope(v8::HandleScope* handleScope);
  void PopHandleScope(v8::HandleScope* handleScope);
  void EscapeHandle(Handle* value);

  // Cast functions
  static v8::Isolate* toV8(Isolate* iso) {
    return reinterpret_cast<v8::Isolate*>(iso);
  }
  static Isolate* fromV8(v8::Isolate* iso) {
    return reinterpret_cast<Isolate*>(iso);
  }
  static Isolate* fromV8(v8::internal::Isolate* iso) {
    return reinterpret_cast<Isolate*>(iso);
  }

  // V8 internals
  void set_array_buffer_allocator_shared(
      std::shared_ptr<v8::ArrayBuffer::Allocator> allocator) {
    m_array_buffer_allocator_shared = std::move(allocator);
  }

  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocator) {
    m_array_buffer_allocator = allocator;
  }
  v8::ArrayBuffer::Allocator* array_buffer_allocator() const {
    return m_array_buffer_allocator;
  }

 private:
  Isolate();

  GCVector<HandleScope*> m_handleScopes;

  // Isolate Scope
  static THREAD_LOCAL Isolate* s_currentIsolate;
  static THREAD_LOCAL Isolate* s_previousIsolate;

  // V8 CreateParams
  v8::ArrayBuffer::Allocator* m_array_buffer_allocator = nullptr;
  std::shared_ptr<v8::ArrayBuffer::Allocator> m_array_buffer_allocator_shared;
};

}  // namespace EscargotShim
