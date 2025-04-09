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

#include "nd-vm-main-message-port.h"
#include <async-uv.h>
#include <channel.h>
#include <future>
#include "api/utils/logger/logger.h"
#include "es-helper.h"

using namespace Escargot;

struct MainMessagePort::Internal {
  std::promise<uv_loop_t*> uv_promise_;
  PersistentHolder<FunctionObjectRef> message_event_class;
};

MainMessagePort::MainMessagePort(std::shared_ptr<Port> port,
                                 std::promise<uv_loop_t*>&& promise) {
  internal_ = std::make_unique<Internal>();
  internal_->uv_promise_ = std::move(promise);
  port_ = port;
}

MainMessagePort::~MainMessagePort() {
  LWNODE_DEV_LOG("[MainMessagePort::~MainMessagePort]");
  Channel::DeletePendingMessages();
}

void MainMessagePort::SetMessageEventClass(FunctionObjectRef* klass) {
  internal_->message_event_class.reset(klass);
}

Escargot::FunctionObjectRef* MainMessagePort::MessageEventClass() {
  return internal_->message_event_class.value();
}

void MainMessagePort::Init(ContextRef* context, uv_loop_t* loop) {
  LWNODE_DEV_LOG("[MainMessagePort::Init]");

  context_ = context;
  uv_loop_ = loop;

  try {
    internal_->uv_promise_.set_value(uv_loop_);
  } catch (const std::exception& e) {
    LWNODE_DEV_LOG("[MainMessagePort::Init] promise error:", e.what());
  }

  Channel::DrainPendingMessages(uv_loop_);
}
