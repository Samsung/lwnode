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

#include "async-uv.h"
#include <nd-logger.h>
#include <uv.h>
#include <functional>
#include "debug-mem-trace.h"

std::queue<AsyncUV::Task> AsyncUV::queue_;
std::mutex AsyncUV::queue_mutex_;

AsyncUV::AsyncUV(uv_loop_t* loop, Task task) : uv_h_(nullptr), task_(task) {
  if (loop && task) {
    Init(loop, task);
  }
  TRACE_ADD(ASYNC, this);
}

AsyncUV::~AsyncUV() {
  if (!uv_h_) {
    return;
  }

  TRACE(ASYNC, "~AsyncUV");
  TRACE_REMOVE(ASYNC, this);

  uv_close(reinterpret_cast<uv_handle_t*>(uv_h_), [](uv_handle_t* handle) {
    TRACE(ASYNC, "~uv_close");
    TRACE_REMOVE(ASYNC_UV, handle);
    delete reinterpret_cast<uv_async_t*>(handle);
  });
}

void AsyncUV::Send(uv_loop_t* loop, Task task) {
  if (loop == nullptr) {
    EnqueueTask(task);
    return;
  }
  (new AsyncUV(loop, task))->Send();
}

void AsyncUV::EnqueueTask(Task task) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  queue_.push(task);
}

bool AsyncUV::DrainPendingTasks(uv_loop_t* loop) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  TRACE(MSGPORT, "drain pending queue", queue_.size());

  if (loop == nullptr) {
    return false;
  }

  while (!queue_.empty()) {
    AsyncUV::Send(loop, queue_.front());
    queue_.pop();
  }
  return true;
}

void AsyncUV::Init(uv_loop_t* loop, Task task) {
  task_ = task;

  uv_h_ = new uv_async_t();
  uv_h_->data = this;
  uv_async_init(loop, uv_h_, [](uv_async_t* handle) {
    auto event = static_cast<AsyncUV*>(handle->data);
    if (event->task_) {
      event->task_(handle);
    }
    delete event;
  });
  TRACE_ADD(ASYNC_UV, uv_h_);
}

void AsyncUV::Send() {
  if (!uv_h_) {
    return;
  }
  uv_async_send(uv_h_);
}
