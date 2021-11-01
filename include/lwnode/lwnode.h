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

#pragma once

#include <v8.h>
#include <functional>
#include <string>

namespace LWNode {

#define DEFAULT_DELAYED_GC_TIMEOUT 1500
#define DEFAULT_PERIODIC_GC_DURATION 5000

void InitializeProcessMethods(v8::Local<v8::Object> target,
                              v8::Local<v8::Context> context);

bool dumpSelfMemorySnapshot();

class MessageLoop {
 public:
  using WakeupMainloopHandler = std::function<void()>;

  struct PlatformHandler {
    WakeupMainloopHandler wakeup{nullptr};
  };

  static MessageLoop* GetInstance();

  // Prepare callback is called right before polling I/O events
  void onPrepare(v8::Isolate* isolate);

  void wakeupMainloopOnce();
  void setWakeupMainloopOnceHandler(PlatformHandler handler);
  unsigned delayedGCTimeout() { return delayedGCTimeout_; }

 private:
  MessageLoop() = default;
  void handleDelayedGC(v8::Isolate* isolate);

  PlatformHandler platformHandler_;
  unsigned delayedGCTimeout_{DEFAULT_DELAYED_GC_TIMEOUT};
};

}  // namespace LWNode
