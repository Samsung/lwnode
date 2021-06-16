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
#include "handle.h"
#include "isolate.h"
#include "utils/misc.h"

#include <algorithm>

namespace EscargotShim {

HandleScopeWrap::HandleScopeWrap(v8::HandleScope* scope,
                                 HandleScopeWrap::Type type)
    : type_(type), v8scope_(reinterpret_cast<void*>(scope)) {}

void HandleScopeWrap::add(HandleWrap* value) {
  LWNODE_CALL_TRACE_2("%p", value);

  handles_.push_back(value);
}

bool HandleScopeWrap::remove(HandleWrap* value) {
  auto it = std::find(handles_.begin(), handles_.end(), value);

  if (it != handles_.end()) {
    handles_.erase(it);
    return true;
  }
  return false;
}

void HandleScopeWrap::clear() {
  if (Flags::isTraceCallEnabled()) {
    for (auto it = handles_.begin(); it != handles_.end(); it++) {
      LWNODE_CALL_TRACE_2("%p", *it);
    }
  }

  handles_.clear();
}

}  // namespace EscargotShim
