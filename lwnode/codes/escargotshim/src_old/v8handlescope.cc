/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "escargotisolateshim.h"
#include "v8.h"

namespace v8 {

THREAD_LOCAL HandleScope* s_currentHandleScope = nullptr;

HandleScope::HandleScope(Isolate* isolate)
    : isolate_(isolate), _prev(s_currentHandleScope) {
  s_currentHandleScope = this;
  EscargotShim::IsolateShim::ToIsolateShim(isolate_)->pushLocalScopeData();
}

HandleScope::~HandleScope() {
  s_currentHandleScope = _prev;
  EscargotShim::IsolateShim::ToIsolateShim(isolate_)->popLocalScopeData();
}

HandleScope* HandleScope::current() {
  return s_currentHandleScope;
}

bool HandleScope::AddLocal(JsRef value) {
  if (s_currentHandleScope == nullptr) {
    return false;
  }

  return true;
}

bool HandleScope::AddLocalContext(JsContextRef value) {
  if (s_currentHandleScope == nullptr) {
    return false;
  }

  return true;
}

bool HandleScope::addToLocalScope(void* jsObject) {
  NESCARGOT_ASSERT(s_currentHandleScope);
  EscargotShim::IsolateShim::ToIsolateShim(isolate_)->addToLocalScopeData(
      jsObject);

  return true;
}

}  // namespace v8
