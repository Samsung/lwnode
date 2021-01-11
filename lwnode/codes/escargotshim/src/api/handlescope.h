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
#include "handle.h"
#include "utils/gc.h"

namespace EscargotShim {

class HandleScope : public gc {
 public:
  enum Type : int {
    Normal = 0,
    Escapable,
    Sealed,
  };

  HandleScope(v8::HandleScope* scope, HandleScope::Type type = Type::Normal);
  virtual ~HandleScope() = default;

  void Add(Handle* value);

  v8::HandleScope* GetV8HandleScope() const { return m_scope; }

 private:
  Type m_type;
  v8::HandleScope* m_scope;

  GCVector<Handle*> m_handles;
};

}  // namespace EscargotShim
