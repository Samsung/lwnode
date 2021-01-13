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

#include "context.h"
#include "isolate.h"

namespace EscargotShim {

ContextWrap::ContextWrap(IsolateWrap* isolate) {
  m_isolate = isolate;
  m_context = isolate->createContext();
  isolate->initializeGlobal(m_context);
}

ContextWrap::~ContextWrap() {}

ContextWrap* ContextWrap::New(IsolateWrap* isolate) {
  auto context = new ContextWrap(isolate);
  return context;
}

void ContextWrap::Enter() {
  m_isolate->Enter();
  m_isolate->pushContext(this);
}

void ContextWrap::Exit() {
  m_isolate->popContext(this);
}

IsolateWrap* ContextWrap::GetIsolate() {
  return m_isolate;
}

}  // namespace EscargotShim
