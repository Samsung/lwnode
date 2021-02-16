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

// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base.h"
#include "libplatform/libplatform.h"
#include "v8.h"

namespace v8 {
namespace platform {
std::unique_ptr<v8::Platform> NewDefaultPlatform(
    int thread_pool_size,
    IdleTaskSupport idle_task_support,
    InProcessStackDumping in_process_stack_dumping,
    std::unique_ptr<v8::TracingController> tracing_controller) {
  LWNODE_RETURN_NULLPTR;
}

}  // namespace platform
}  // namespace v8
