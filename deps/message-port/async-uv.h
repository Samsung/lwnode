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
#include <map>
#include <mutex>
#include <queue>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

using uv_loop_t = struct uv_loop_s;
using uv_async_t = struct uv_async_s;
using uv_handle_t = struct uv_handle_s;

class EXPORT_API AsyncUV {
 public:
  using Task = std::function<void(uv_async_t*)>;

  // If loop is nullptr, the task is enqueued to the pending queue.
  // When a valid loop is provided later, all pending tasks will be executed.
  static bool Send(uv_loop_t* loop, Task task);
  static bool DrainPendingTasks(uv_loop_t* loop);
  static size_t EnqueueTask(Task task);
  static void DeletePendingTasks();
  static bool IsPendingTasksEmpty();

  // Register and cleanup one shared async handle per loop.
  static bool InitPerThread(uv_loop_t* loop);
  static void CleanupPerThread(uv_loop_t* loop);

  AsyncUV(uv_loop_t* loop = nullptr, Task task = nullptr);
  ~AsyncUV();

  void Init(uv_loop_t* loop, Task task);
  bool Send();

 private:
  struct LoopData;
  static LoopData* GetLoopData(uv_loop_t* loop, bool create_if_not_found);
  static void OnAsyncCalled(uv_async_t* handle);

  uv_loop_t* loop_;
  Task task_;

  static std::map<uv_loop_t*, LoopData*> loop_data_;
  static std::mutex loop_data_mutex_;
  static std::queue<Task> pending_queue_;
  static std::mutex pending_queue_mutex_;
};
