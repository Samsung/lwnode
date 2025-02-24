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

#include "nd-vm-message-channel.h"
#include <async-uv.h>
#include <channel.h>
#include <future>
#include "es-helper.h"

using namespace Escargot;

struct MessageChannel::Internal {
  std::promise<uv_loop_t*> uv_promise_;

  PersistentHolder<FunctionObjectRef> message_event_class;
};

MessageChannel::MessageChannel() {
  internal_ = std::make_unique<Internal>();
}

MessageChannel::~MessageChannel() {}

void MessageChannel::SetMessageEventClass(FunctionObjectRef* klass) {
  internal_->message_event_class.reset(klass);
}

Escargot::FunctionObjectRef* MessageChannel::MessageEventClass() {
  return internal_->message_event_class.value();
}

void MessageChannel::Init(ContextRef* context, uv_loop_t* loop) {
  auto channel = Channel::New(internal_->uv_promise_.get_future(), "embedder");
  port1_ = channel.port1;
  port2_ = channel.port2;
  context_ = context;
  uv_loop_ = loop;
}

void MessageChannel::Start() {
  internal_->uv_promise_.set_value(uv_loop_);
  Channel::DrainPendingMessages(uv_loop_);
}
