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
#include "utils/gc.h"

namespace EscargotShim {

class IsolateWrap;
class HandleWrap;

class HandleScopeWrap : public gc {
 public:
  enum Type : int {
    Normal = 0,
    Escapable,
    Sealed,
  };

  HandleScopeWrap(v8::HandleScope* scope,
                  HandleScopeWrap::Type type = Type::Normal);

  void add(HandleWrap* value);
  bool remove(HandleWrap* value);
  void clear();

  v8::HandleScope* v8HandleScope() const { return scope_; }

  static HandleWrap* CreateHandle(IsolateWrap* isolate, HandleWrap* value);

 private:
  Type type_;
  v8::HandleScope* scope_ = nullptr;
  GCVector<HandleWrap*> handles_;
};

}  // namespace EscargotShim
