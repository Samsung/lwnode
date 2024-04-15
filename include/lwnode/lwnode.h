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
#include <memory>
#include <string>
#include <unordered_map>

namespace Escargot {
class ContextRef;
class ValueRef;
}  // namespace Escargot

#ifndef LWNODE_EXPORT
#define LWNODE_EXPORT __attribute__((visibility("default")))
#endif

namespace LWNode {

void InitializeProcessMethods(v8::Local<v8::Object> target,
                              v8::Local<v8::Context> context);

void IdleGC(v8::Isolate* isolate = nullptr);
void initDebugger();
bool dumpSelfMemorySnapshot();

class MessageLoop {
  using WakeupMainloopHandler = std::function<void()>;

  struct PlatformHandler {
    WakeupMainloopHandler wakeup{nullptr};
  };

 public:
  static MessageLoop* GetInstance();

  // Prepare callback is called right before polling I/O events
  void onPrepare(v8::Isolate* isolate);

  void wakeupMainloopOnce();
  void setWakeupMainloopOnceHandler(PlatformHandler handler);

 private:
  MessageLoop();

  PlatformHandler platformHandler_;

  class Internal;
  std::unique_ptr<Internal> internal_;
};

class LWNODE_EXPORT Utils {
 public:
  static Escargot::ContextRef* ToEsContext(v8::Context* context);

  static v8::Local<v8::Value> NewLocal(v8::Isolate* isolate,
                                       Escargot::ValueRef* ptr);

  static bool CompileRun(Escargot::ContextRef* context,
                         const char* source,
                         bool isModule = false);

  static bool IsRunningIsolate(Escargot::ContextRef* context);

  static std::string trimLastNewLineIfNeeded(std::string&& str);
};

class LWNODE_EXPORT SystemInfo {
 public:
  static SystemInfo* getInstance();

  void add(const char* key, const char* value = "");
  bool has(const std::string& info);
  bool get(const char* info, std::string& value);

 private:
  SystemInfo();
  std::unordered_map<std::string, std::string> infos_;
};

}  // namespace LWNode
