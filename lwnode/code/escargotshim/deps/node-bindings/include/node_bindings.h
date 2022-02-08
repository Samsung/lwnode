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

typedef struct uv_loop_s uv_loop_t;

namespace LWNode {

void push_aul_message(const char* message);
void push_aul_termination_message();

class GmainLoopWork {
 public:
  virtual bool RunOnce() = 0;
};

class GmainLoopNodeBindings {
 public:
  GmainLoopNodeBindings(GmainLoopWork* bindingWork);
  virtual ~GmainLoopNodeBindings(){};

  void StartEventLoop();
  void RunOnce();
  bool HasMoreTasks();
  void TerminateGMainLoop() { m_isTerminated = true; }

 private:
  bool m_isInitialize = {false};
  bool m_hasMoreNodeTasks = {true};
  bool m_isTerminated = {false};
  GmainLoopWork* gmainLoopWork_{nullptr};
};

}  // namespace LWNode

#endif
