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
#include "api/handle.h"

namespace EscargotShim {

class IsolateWrap;
class CallSite;

typedef GCUnorderedMap<int, void*> EmbedderDataMap;

class ContextWrap : public ValueWrap {
 public:
  static ContextWrap* New(
      IsolateWrap* isolate,
      v8::ExtensionConfiguration* extensionConfiguration = nullptr);
  static ContextWrap* fromEscargot(Escargot::ContextRef* context);
  static ContextWrap* fromV8(v8::Context* context);

  void Enter();
  void Exit();
  IsolateWrap* GetIsolate();

  Escargot::ContextRef* get() { return context_; }

  void SetEmbedderData(int index, ValueWrap* value);
  ValueWrap* GetEmbedderData(int index);
  uint32_t GetNumberOfEmbedderDataFields();

  void SetAlignedPointerInEmbedderData(int index, void* value);
  void* GetAlignedPointerFromEmbedderData(int index);

  Escargot::ObjectRef* GetExtrasBindingObject();

  void SetSecurityToken(Escargot::ValueRef* token);
  Escargot::ValueRef* GetSecurityToken();
  void UseDefaultSecurityToken();

  CallSite* callSite() { return callSite_; }

  void SetAbortScriptExecution(
      v8::Context::AbortScriptExecutionCallback callback);
  v8::Context::AbortScriptExecutionCallback getAbortScriptExecution();

  void initDebugger();

 private:
  EmbedderDataMap* embedder_data_{nullptr};

  ContextWrap(IsolateWrap* isolate,
              v8::ExtensionConfiguration* extensionConfiguration);
  void setEmbedderData(int index, void* value);
  void* getEmbedderData(int index);

  IsolateWrap* isolate_ = nullptr;
  Escargot::ContextRef* context_ = nullptr;
  Escargot::ObjectRef* bindingObject_ = nullptr;
  Escargot::ValueRef* security_token_ = nullptr;

  CallSite* callSite_ = nullptr;

  v8::Context::AbortScriptExecutionCallback abortScriptExecutionCallback_ =
      nullptr;
};

}  // namespace EscargotShim
