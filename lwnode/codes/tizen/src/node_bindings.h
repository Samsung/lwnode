/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
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

#ifndef NODE_BINDINGS_H
#define NODE_BINDINGS_H

#include <functional>

namespace v8 {
class Isolate;
}

namespace node {
class Environment;
void EmitMessage(v8::Isolate* isolate, const char* fmt, ...);
}  // namespace node

typedef struct uv_loop_s uv_loop_t;

namespace nescargot {

void push_aul_message(const char* message);
void push_aul_termination_message();

class NodeBindings {
 public:
  NodeBindings();
  virtual ~NodeBindings(){};

  struct Platform {
    void (*PumpMessageLoop)(v8::Isolate* isolate);
    void (*EnterIdleMode)(v8::Isolate* isolate);
  };

  struct Environment {
    std::function<v8::Isolate*()> isolate;
    std::function<uv_loop_t*()> event_loop;
  };

  struct Node {
    std::function<void()> EmitBeforeExit;
  };

  void Initialize(Environment&& env, Platform&& platform, Node&& node);
  void StartEventLoop();
  void RunOnce();
  bool HasMoreTasks();
  void TerminateGMainLoop() { m_isTerminated = true; }

 private:
  Environment m_env;
  Platform m_platform;
  Node m_node;
  bool m_isInitialize = {false};
  bool m_hasMoreNodeTasks = {true};
  bool m_isTerminated = {false};
  unsigned int m_idleCheckTimeoutID = {0};
};

}  // namespace nescargot

#endif
