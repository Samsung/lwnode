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

#include "api/global.h"
#include "handle.h"
#include "isolate.h"
#include "utils/misc.h"

#include <algorithm>
#include <sstream>

namespace EscargotShim {

HandleScopeWrap::HandleScopeWrap(v8::HandleScope* scope,
                                 HandleScopeWrap::Type type)
    : type_(type), v8scope_(reinterpret_cast<void*>(scope)) {}

HandleScopeWrap::HandleScopeWrap(v8::SealHandleScope* scope,
                                 HandleScopeWrap::Type type)
    : type_(type), v8scope_(reinterpret_cast<void*>(scope)) {}

HandleScopeWrap::HandleScopeWrap(v8::EscapableHandleScope* scope,
                                 HandleScopeWrap::Type type)
    : type_(type), v8scope_(reinterpret_cast<void*>(scope)) {}

HandleScopeWrap::HandleScopeWrap(HandleScopeWrap::Type type)
    : type_(type), v8scope_(nullptr) {}

void HandleScopeWrap::add(HandleWrap* value) {
  LWNODE_CALL_TRACE_ID(HDLSCOPE,
                       "%s --> %p (lw: %p)",
                       value->getHandleInfoString().c_str(),
                       v8scope_,
                       this);

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
  LWNODE_CALL_TRACE_ID(HDLSCOPE);
  if (Global::flags()->isOn(Flag::Type::TraceCall, "HDLSCOPE")) {
    std::stringstream ss;
    std::vector<std::string> vector;

    const int column = 10;
    int count = 0;
    for (const auto& it : handles_) {
      ss << it << " ";
      if (++count % column == 0) {
        vector.push_back(ss.str());
        ss.str("");
      }
    }
    if (count % column) {
      vector.push_back(ss.str());
    }

    LWNODE_CALL_TRACE_LOG("%p contains %d handles:", this, count);
    for (const auto& it : vector) {
      LWNODE_CALL_TRACE_LOG("%s", it.c_str());
    }
  }
  handles_.clear();
}

HandleScopeWrapGuard::HandleScopeWrapGuard(IsolateWrap* isolate)
    : isolate_(isolate) {
  LWNODE_CHECK_NOT_NULL(isolate_);
  isolate_->pushHandleScope(
      new HandleScopeWrap(HandleScopeWrap::Type::Internal));
}

HandleScopeWrapGuard::~HandleScopeWrapGuard() {
  isolate_->popHandleScope(nullptr);
}

}  // namespace EscargotShim
