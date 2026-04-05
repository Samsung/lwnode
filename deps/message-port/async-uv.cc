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
#include "debug-mem-trace.h"

struct AsyncUV::LoopData {
  uv_loop_t* loop{nullptr};
  uv_async_t* handle{nullptr};
  std::queue<Task> queue;
  std::mutex queue_mutex;
};

std::map<uv_loop_t*, AsyncUV::LoopData*> AsyncUV::loop_data_;
std::mutex AsyncUV::loop_data_mutex_;
std::queue<AsyncUV::Task> AsyncUV::pending_queue_;
std::mutex AsyncUV::pending_queue_mutex_;

AsyncUV::AsyncUV(uv_loop_t* loop, Task task) : loop_(loop), task_(task) {
  TRACE_ADD(ASYNC, this);
}

AsyncUV::~AsyncUV() {
  TRACE(ASYNC, "~AsyncUV");
  TRACE_REMOVE(ASYNC, this);
}

void AsyncUV::OnAsyncCalled(uv_async_t* handle) {
  auto* loop_data = static_cast<LoopData*>(handle->data);
  if (loop_data == nullptr) {
    return;
  }

  std::queue<Task> tasks_to_run;
  {
    std::lock_guard<std::mutex> lock(loop_data->queue_mutex);
    loop_data->queue.swap(tasks_to_run);
  }

  while (!tasks_to_run.empty()) {
    Task& task = tasks_to_run.front();
    if (task) {
      TRACE(MSGPORT, "run task");
      task(handle);
      TRACE(MSGPORT, "/run task");
    }
    tasks_to_run.pop();
  }
}

AsyncUV::LoopData* AsyncUV::GetLoopData(uv_loop_t* loop,
                                        bool create_if_not_found) {
  std::lock_guard<std::mutex> lock(loop_data_mutex_);
  auto it = loop_data_.find(loop);
  if (it != loop_data_.end()) {
    return it->second;
  }

  if (!create_if_not_found || loop == nullptr) {
    return nullptr;
  }

  auto* loop_data = new LoopData();
  loop_data->loop = loop;
  loop_data->handle = new uv_async_t();
  loop_data->handle->data = loop_data;
  uv_async_init(loop, loop_data->handle, OnAsyncCalled);
  uv_unref(reinterpret_cast<uv_handle_t*>(loop_data->handle));
  TRACE_ADD(ASYNC_UV, loop_data->handle);
  loop_data_[loop] = loop_data;
  return loop_data;
}

bool AsyncUV::InitPerThread(uv_loop_t* loop) {
  if (loop == nullptr) {
    return false;
  }

  return GetLoopData(loop, true) != nullptr;
}

void AsyncUV::CleanupPerThread(uv_loop_t* loop) {
  std::lock_guard<std::mutex> lock(loop_data_mutex_);
  auto it = loop_data_.find(loop);
  if (it == loop_data_.end()) {
    return;
  }

  LoopData* loop_data = it->second;
  loop_data_.erase(it);

  if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(loop_data->handle))) {
    uv_close(reinterpret_cast<uv_handle_t*>(loop_data->handle),
             [](uv_handle_t* handle) {
               TRACE(ASYNC, "~uv_close");
               TRACE_REMOVE(ASYNC_UV, handle);
               auto* loop_data = static_cast<LoopData*>(handle->data);
               delete loop_data->handle;
               delete loop_data;
             });
  }
}

bool AsyncUV::Send(uv_loop_t* loop, Task task) {
  if (loop == nullptr || !task) {
    TRACE(MSGPORT, "invalid loop");
    return false;
  }

  LoopData* loop_data = GetLoopData(loop, true);
  if (loop_data == nullptr) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(loop_data->queue_mutex);
    loop_data->queue.push(std::move(task));
  }

  uv_async_send(loop_data->handle);
  return true;
}

size_t AsyncUV::EnqueueTask(Task task) {
  TRACE(MSGPORT, "EnqueueTask");
  std::lock_guard<std::mutex> lock(pending_queue_mutex_);
  pending_queue_.push(std::move(task));
  return pending_queue_.size();
}

bool AsyncUV::DrainPendingTasks(uv_loop_t* loop) {
  if (loop == nullptr) {
    TRACE(MSGPORT, "invalid loop");
    return false;
  }

  TRACE(MSGPORT, "DrainPendingTasks");
  std::queue<Task> pending_tasks;
  {
    std::lock_guard<std::mutex> lock(pending_queue_mutex_);
    TRACE(MSGPORT, "drain pending tasks %zu", pending_queue_.size());
    pending_queue_.swap(pending_tasks);
  }

  while (!pending_tasks.empty()) {
    AsyncUV::Send(loop, std::move(pending_tasks.front()));
    pending_tasks.pop();
  }
  TRACE(MSGPORT, "/drain pending tasks");
  return true;
}

void AsyncUV::DeletePendingTasks() {
  TRACE(MSGPORT, "DeletePendingTasks");
  std::lock_guard<std::mutex> lock(pending_queue_mutex_);
  TRACE(MSGPORT, "delete pending tasks %zu", pending_queue_.size());
  if (!pending_queue_.empty()) {
    std::queue<Task> empty;
    std::swap(pending_queue_, empty);
  }
}

bool AsyncUV::IsPendingTasksEmpty() {
  TRACE(MSGPORT, "IsPendingTasksEmpty");
  std::lock_guard<std::mutex> lock(pending_queue_mutex_);
  return pending_queue_.empty();
}

void AsyncUV::Init(uv_loop_t* loop, Task task) {
  loop_ = loop;
  task_ = task;
}

bool AsyncUV::Send() {
  if (loop_ == nullptr || !task_) {
    return false;
  }
  return Send(loop_, std::move(task_));
}
