/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <future>  // std::promise
#include <memory>
#include <string>
#include "message-port.h"

#ifndef LWNODE_EXPORT
#define LWNODE_EXPORT __attribute__((visibility("default")))
#endif

class Port;

namespace lwnode {

LWNODE_EXPORT bool ParseAULEvent(int argc, char** argv);

// Sets the path of the root directory of the JavaScript. If you do
// not put the path argument, the root path is the app's resource path by
// default on Tizen AUL mode. Be sure to call this function before lwnode::Start
// function.
LWNODE_EXPORT bool InitScriptRootPath(const std::string path = "");

LWNODE_EXPORT int Start(int argc, char** argv);

// Sets the dlog tag id for debugging. This is only used on Tizen when not in
// AUL mode.
LWNODE_EXPORT void SetDlogID(const std::string& appId);

class LWNODE_EXPORT Runtime {
 public:
  Runtime();
  ~Runtime();

  // Initializes the runtime and returns true if it early termination and sets
  // the exit code to the second return value. Otherwise, it returns false
  // and you need call Run() to run the runtime.
  std::pair<bool, int> Init(int argc,
                            char** argv,
                            std::promise<void>&& promise);

  // Runs the runtime and returns the exit code. The runtime must be
  // initialized.
  int Run();

  std::shared_ptr<Port> GetPort();

 private:
  class Internal;
  Internal* internal_ = nullptr;
};

}  // namespace lwnode
