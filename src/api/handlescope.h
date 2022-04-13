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

#include <v8.h>
#include "utils/gc-util.h"

namespace EscargotShim {

class IsolateWrap;
class HandleWrap;
class HandleScopeWrapGuard;

typedef void v8Scope_t;

class HandleScopeWrap : public gc {
 public:
  enum Type : uint8_t {
    None = 0,
    Normal,
    Escapable,
    Sealed,
    Internal,
  };

  HandleScopeWrap(v8::HandleScope* scope, HandleScopeWrap::Type type);
  HandleScopeWrap(v8::SealHandleScope* scope, HandleScopeWrap::Type type);
  HandleScopeWrap(v8::EscapableHandleScope* scope, HandleScopeWrap::Type type);
  Type type() const { return type_; }
  v8Scope_t* v8Scope() const { return v8scope_; }

 private:
  HandleScopeWrap(HandleScopeWrap::Type type);
  void add(HandleWrap* value);
  bool remove(HandleWrap* value);
  void clear();

  Type type_{None};
  v8Scope_t* v8scope_{nullptr};
  GCVector<HandleWrap*> handles_;

  friend class IsolateWrap;
  friend class HandleScopeWrapGuard;
};

class HandleScopeWrapGuard : public gc {
 public:
  HandleScopeWrapGuard(IsolateWrap* isolate);
  ~HandleScopeWrapGuard();

  void* operator new(size_t size) = delete;
  void* operator new[](size_t size) = delete;
  void operator delete(void*, size_t) = delete;
  void operator delete[](void*, size_t) = delete;

 private:
  IsolateWrap* isolate_{nullptr};
};

}  // namespace EscargotShim
