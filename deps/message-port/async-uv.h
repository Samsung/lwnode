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

#include <functional>
#include <mutex>
#include <queue>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

using uv_loop_t = struct uv_loop_s;
using uv_async_t = struct uv_async_s;

class EXPORT_API AsyncUV {
 public:
  using Task = std::function<void(uv_async_t*)>;

  // If loop is nullptr, the task is enqueued to the pending queue.
  // When a valid loop is provided later, all pending tasks will be executed.
  static void Send(uv_loop_t* loop, Task task);
  static bool DrainPendingTasks(uv_loop_t* loop);
  static void EnqueueTask(Task task);

  AsyncUV(uv_loop_t* loop = nullptr, Task task = nullptr);
  ~AsyncUV();

  void Init(uv_loop_t* loop, Task task);
  void Send();

 private:
  uv_async_t* uv_h_;
  Task task_;

  static std::queue<Task> queue_;
  static std::mutex queue_mutex_;
};
