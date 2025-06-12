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

/**
 * Sets the path of the root directory of the JavaScript. If you do
 * not put the path argument, the root path is the app's resource path by
 * default on Tizen AUL mode. Be sure to call this function before lwnode::Start
 * function.
 **/
LWNODE_EXPORT bool InitScriptRootPath(const std::string path = "");

LWNODE_EXPORT int Start(int argc, char** argv);

/**
 * Sets the dlog tag id for debugging. This is only used on Tizen when not in
 * AUL mode.
 **/
LWNODE_EXPORT void SetDlogID(const std::string& appId);

class LWNODE_EXPORT Runtime {
 public:
  using SendMessageSyncCallback = std::string (*)(const std::string&,
                                                  void* user_data);

  class Configuration {
   public:
    friend Runtime;
    Configuration();
    ~Configuration();

    Configuration(Configuration&) = delete;
    Configuration(Configuration&&) = delete;
    Configuration& operator=(const Configuration& t) = delete;
    Configuration& operator=(Configuration&&);

    void OnSendMessageSync(SendMessageSyncCallback callback, void* user_data);

    bool Set(const std::string& key, const std::string& value);
    bool Set(const std::string& key, int value);
    bool Set(const std::string& key, bool value);

   private:
    struct Internal;
    Internal* internal_ = nullptr;
  };

  Runtime();
  Runtime(Configuration&& config);

  ~Runtime();

  /**
   * Start the runtime and returns the exit code. It initializes the runtime
   * and runs it. When the runtime is initialized, the promise object is set.
   *
   * @param argc - Argument count.
   * @param argv - Argument vector. The element should be the starting file
   * name of the application.
   * @return Returns the exit code of the runtime.
   **/
  int Start(int argc, char** argv);

  /**
   * Stop the runtime. You can use this function to stop the runtime from
   * another thread.
   **/
  void Stop();

  /**
   * Get the message port instance.
   *
   * @return Returns the shared pointer of the message port instance. If the
   * runtime is not ready, it returns the unavailable port instance. Otherwise,
   * it returns the shared pointer of the available message port instance.
   *
   **/
  std::shared_ptr<Port> GetPort();

  /**
   * Wait until the runtime is ready.
   *
   * @param ms - Timeout in milliseconds. If it is negative, it waits
   * indefinitely.
   * @return Returns the status of the future object. If it is ready, the
   * runtime is ready.
   **/
  std::future_status WaitForReady(int64_t ms = -1);

 private:
  class Internal;
  Internal* internal_ = nullptr;
};

}  // namespace lwnode
