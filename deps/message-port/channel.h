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

#include <future>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

using uv_loop_t = struct uv_loop_s;
class Port;

struct EXPORT_API Channel {
  std::shared_ptr<Port> port1;
  std::shared_ptr<Port> port2;

  static Channel New(uv_loop_t* loop, const char* origin = nullptr);
  // Messages sent before the uv_loop is available will be queued. Once the loop
  // is ready by resolving the promise for the uv_loop, DrainPendingMessages or
  // the next Port::PostMessage will process the queued messages.
  static Channel New(std::shared_future<uv_loop_t*> loop,
                     const char* origin = nullptr);
  static void DrainPendingMessages(uv_loop_t* loop);

  void Reset();
};
