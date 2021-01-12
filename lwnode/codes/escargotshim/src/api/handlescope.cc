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

#include "handlescope.h"

namespace EscargotShim {

HandleScopeWrap::HandleScopeWrap(v8::HandleScope* scope, HandleScopeWrap::Type type)
    : m_type(type), m_scope(scope) {}

void HandleScopeWrap::add(HandleWrap* value) {
  m_handles.push_back(value);
}

}  // namespace EscargotShim
