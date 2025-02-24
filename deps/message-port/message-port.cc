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

#include "message-port.h"
#include <nd-logger.h>
#include <uv.h>
#include <chrono>
#include <future>
#include "async-uv.h"
#include "channel.h"

// MessageEvent::Internal
// -----------------------------------------------------------------------------

struct MessageEvent::Internal {
  Internal(const std::string& data) : data(data) {}
  ~Internal() { TRACE(MSGEVENT, "~MessageEvent"); }

  std::string data;
  std::string origin;
  std::vector<std::weak_ptr<Port>> ports;
  std::weak_ptr<Port> target;
};

std::shared_ptr<MessageEvent> MessageEvent::New(const std::string& data) {
  return std::shared_ptr<MessageEvent>(new MessageEvent(data));
}

std::shared_ptr<MessageEvent> MessageEvent::Clone() const {
  return MessageEvent::New(internal_->data);
}

MessageEvent::MessageEvent(const std::string& data)
    : internal_(std::make_unique<Internal>(data)) {}

MessageEvent::~MessageEvent() {}

const std::string& MessageEvent::data() const {
  return internal_->data;
}

const std::string& MessageEvent::origin() const {
  return internal_->origin;
}

const std::vector<std::weak_ptr<Port>> MessageEvent::ports() const {
  return internal_->ports;
}

const std::weak_ptr<Port>& MessageEvent::target() const {
  return internal_->target;
}

// Port::Internal
// -----------------------------------------------------------------------------

struct Port::Internal {
  Internal() : loop(nullptr) {}
  ~Internal() { TRACE(MSGPORT, "~Port"); }
  void RefSink() { sink_holder = sink.lock(); }
  void UnrefSink() { sink_holder.reset(); }
  void SetLoop(uv_loop_t* loop_handle) { loop = loop_handle; }
  void SetLoop(std::shared_future<uv_loop_t*> loop_future) {
    future = loop_future;
  }

  uv_loop_t* loop;
  std::string origin;
  std::shared_future<uv_loop_t*> future;
  std::weak_ptr<Port> sink;
  std::shared_ptr<Port> sink_holder;
  OnMessageCallback callback;
};

Port::Port() : internal_(std::make_unique<Internal>()) {}

Port::~Port() {}

void Port::OnMessage(const OnMessageCallback& callback) {
  internal_->callback = std::move(callback);
}

Port::Result Port::PostMessage(std::shared_ptr<MessageEvent> event) {
  // Check if the sink is valid.
  std::shared_ptr<Port> sink = internal_->sink.lock();
  if (sink == nullptr) {
    TRACE(MSGPORT, "sink port released.");
    return Result::NoSink;
  }

  // Check if the sink has a receiver.
  if (sink->internal_->callback == nullptr) {
    TRACE(MSGPORT, "sink has no callback.");
    return Result::NoOnMessage;
  }

  // Set target and origin field of the given event.
  if (event->internal_->target.lock() == nullptr) {
    event->internal_->target = internal_->sink;
    event->internal_->origin = internal_->origin;
  }

  // It's not allowed to use MessageEvents to be sent to different sinks.
  if (event->internal_->target.lock() != internal_->sink.lock()) {
    return Result::InvalidMessageEvent;
  }

  // Get a valid loop handle if invalid.
  if (internal_->loop == nullptr) {
    if (!internal_->future.valid()) {
      return Result::InvalidPortLoop;
    }
    if (internal_->future.wait_for(std::chrono::milliseconds(1)) ==
        std::future_status::ready) {
      auto loop = internal_->future.get();
      if (loop == nullptr) {
        return Result::InvalidPortLoop;
      }
      internal_->loop = loop;
      AsyncUV::DrainPendingTasks(internal_->loop);
    }
  }

  // Sends a task immediately if the loop handle is valid. If the loop handle is
  // nullptr, the task is enqueued to the pending queue and will be executed
  // later when a valid loop handle is provided.
  AsyncUV::Send(internal_->loop,
                [event = event, sink_weak = internal_->sink](uv_async_t*) {
                  // event: shared_ptr, sink_weak: weak_ptr
                  auto sink = sink_weak.lock();
                  if (sink && sink->internal_->callback) {
                    // Since sink is locked, event->target() is always valid
                    // inside the callback.
                    sink->internal_->callback(event.get());
                  } else {
                    TRACE(MSGPORT, "sink port released, or no callback");
                  }
                });
  return Result::NoError;
}

void Port::Unref() {
  internal_->UnrefSink();
}

// Channel
// -----------------------------------------------------------------------------

Channel Channel::New(uv_loop_t* loop, const char* origin) {
  auto port1 = std::shared_ptr<Port>(new Port());
  auto port2 = std::shared_ptr<Port>(new Port());
  if (origin) port1->internal_->origin = origin;
  port1->internal_->SetLoop(loop);
  port2->internal_->SetLoop(loop);
  port1->internal_->sink = port2;
  port2->internal_->sink = port1;
  // This makes port1 refer to port2. If port2 isn't shared with anyone else by
  // `std::shared_ptr`, releasing port1 will also release port2.
  port1->internal_->RefSink();
  return {port1, port2};
}

Channel Channel::New(std::shared_future<uv_loop_t*> loop, const char* origin) {
  auto port1 = std::shared_ptr<Port>(new Port());
  auto port2 = std::shared_ptr<Port>(new Port());
  if (origin) port1->internal_->origin = origin;
  port1->internal_->SetLoop(loop);
  port2->internal_->SetLoop(loop);
  port1->internal_->sink = port2;
  port2->internal_->sink = port1;
  // This makes port1 refer to port2. If port2 isn't shared with anyone else by
  // `std::shared_ptr`, releasing port1 will also release port2.
  port1->internal_->RefSink();
  return {port1, port2};
}

void Channel::DrainPendingMessages(uv_loop_t* loop) {
  AsyncUV::DrainPendingTasks(loop);
}

void Channel::Reset() {
  port1.reset();
  port2.reset();
}
