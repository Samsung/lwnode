/*
 * Copyright (c) 2025-present Samsung Electronics Co., Ltd
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

#include <memory>

namespace Escargot {
class ContextRef;
class FunctionObjectRef;
}  // namespace Escargot

class Port;
using uv_loop_t = struct uv_loop_s;

class MessageChannel {
 public:
  MessageChannel();
  ~MessageChannel();

  void Init(Escargot::ContextRef* context, uv_loop_t* loop);
  void Start();

  std::shared_ptr<Port> port1() { return port1_; }
  std::shared_ptr<Port> port2() { return port2_; }
  Escargot::ContextRef* context() { return context_; }

  void SetMessageEventClass(Escargot::FunctionObjectRef* klass);
  Escargot::FunctionObjectRef* MessageEventClass();

 private:
  // In runtime perspective, port1 is a sink.
  std::shared_ptr<Port> port1_;
  std::shared_ptr<Port> port2_;
  Escargot::ContextRef* context_;
  uv_loop_t* uv_loop_;

  struct Internal;
  std::unique_ptr<Internal> internal_;
};
