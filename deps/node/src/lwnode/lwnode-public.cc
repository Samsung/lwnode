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

#include <uv.h>

#include "global-configuration.h"
#include "lwnode-public.h"
#include "lwnode-version.h"
#include "lwnode.h"
#include "lwnode/aul-event-receiver.h"
#include "node.h"
#include "node_main_lw_runner-inl.h"
#include "trace.h"
#include "v8.h"

#include <atomic>
#include <sstream>
#include <thread>

static bool g_allow_mulitple_instance = false;

LWNODE_EXPORT void ForceAllowMultipleInstance() {
  g_allow_mulitple_instance = true;
  LWNODE_DEV_LOG("[LWNODE] TEST MODE");
}

namespace node {
namespace native_module {
extern bool initializeLWNodeBuiltinFile(const std::string path = "");
}
}  // namespace node

using namespace node;

namespace lwnode {

struct Runtime::Configuration::Internal {
  Runtime::SendMessageSyncCallback send_message_sync_callback{nullptr};
  void* send_message_sync_callback_data{nullptr};
  std::string lwnode_data_path;
};

class Runtime::Internal {
  friend Runtime;

 public:
  static std::atomic<int> instance_count_;

  enum class State {
    kNotInitialized,
    kInitialized,
    kRunning,
    kStopped,
    kReleased
  };

  enum ExitCode {
    kSuccess = 0,
    kFailure = 1,
    kNoBuiltinFile = 100,
  };

  Internal() { LWNODE_DEV_LOG("[Runtime::Internal::Internal] new"); }

  std::pair<bool, int> Init(int argc, char** argv) {
    if (state_ != State::kNotInitialized) {
      LWNODE_DEV_LOG("[Runtime::Internal::Init] already initialized");
      return std::make_pair(true, ExitCode::kFailure);
    }

    if (!native_module::initializeLWNodeBuiltinFile(
            config_.internal_->lwnode_data_path)) {
      LWNODE_DEV_LOG(
          "[Runtime::Internal::Init] failed to initialize builtin file");
      return std::make_pair(true, ExitCode::kNoBuiltinFile);
    }

    LWNODE_DEV_LOG("[Runtime::Internal::Init]");
    state_ = State::kInitialized;

    // Set sendMessageSync callback to isolate context embedder data.
    runner_.SetOnMainEnvCreationCallback(
        [this](v8::Local<v8::Context> context) {
          context->SetAlignedPointerInEmbedderData(
              LWNode::ContextEmbedderIndex::kSendMessageSyncCallback,
              reinterpret_cast<void*>(
                  config_.internal_->send_message_sync_callback));
          context->SetAlignedPointerInEmbedderData(
              LWNode::ContextEmbedderIndex::kSendMessageSyncCallbackData,
              config_.internal_->send_message_sync_callback_data);
        });

    return InitializeNode(argc, argv, &instance_);
  }

  int Run() {
    if (state_ != State::kInitialized) {
      LWNODE_DEV_LOG("[Runtime::Internal::Run] not initialized");
      return -1;
    }

    CHECK_NOT_NULL(instance_);
    LWNODE_DEV_LOG("[Runtime::Internal::Run]");
    state_ = State::kRunning;

    int result = runner_.Run(*instance_);

    state_ = State::kStopped;

    return result;
  }

  void Stop() {
    std::unique_lock<std::mutex> lock(stop_mutex_);

    if (state_ != State::kRunning) {
      LWNODE_DEV_LOG("[Runtime::Internal::Stop] already stopped");
      return;
    }

    CHECK_NOT_NULL(instance_);
    LWNODE_DEV_LOG("[Runtime::Internal::Stop]");
    state_ = State::kStopped;

    runner_.Stop();
  }

  void Free() {
    if (state_ != State::kStopped && state_ != State::kInitialized) {
      LWNODE_DEV_LOG("[Runtime::Internal::Free] not stopped");
      return;
    }

    state_ = State::kReleased;
    if (instance_) {
      LWNODE_DEV_LOG("[Runtime::Internal::Free]");
      DisposeNode(instance_);
    }

    instance_ = nullptr;
  }

 private:
  NodeMainInstance* instance_{nullptr};
  LWNode::LWNodeMainRunner runner_;
  Runtime::Configuration config_;
  std::atomic<State> state_{State::kNotInitialized};
  std::mutex stop_mutex_;
};

std::atomic<int> Runtime::Internal::instance_count_{0};

/**************************************************************************
 * Runtime class
 **************************************************************************/

Runtime::Runtime() : internal_(new Internal()) {
  Internal::instance_count_++;

  std::stringstream ss;
  ss << "pid " << std::to_string(uv_os_getpid()) << " " << "tid "
     << std::this_thread::get_id();

  LWNODE_DEV_LOG("[Runtime::Runtime] %d %s",
                 Internal::instance_count_.load(),
                 ss.str().c_str());
}

Runtime::Runtime(Configuration&& config) : Runtime() {
  internal_->config_ = std::move(config);
}

Runtime::~Runtime() {
  delete internal_;
  LWNODE_DEV_LOG("[Runtime::~Runtime]");
}

int Runtime::Start(int argc, char** argv, std::promise<void>&& promise) {
  LWNODE_PERF_LOG("[Runtime::Start]");
  LWNODE_DEV_LOG("[Runtime] version: %s", LWNODE_VERSION_TAG);
#if defined(NDEBUG)
  LWNODE_DEV_LOG("[Runtime] release mode");
#else
  LWNODE_DEV_LOG("[Runtime] debug mode");
#endif

  if (!g_allow_mulitple_instance && Runtime::Internal::instance_count_ > 1) {
    LWNODE_DEV_LOG("[Runtime] Runtime can only be started once per process.");
    promise.set_exception(std::make_exception_ptr(
        std::runtime_error("Runtime can only be started once per process.")));
    return Runtime::Internal::ExitCode::kFailure;
  }

  internal_->runner_.SetInitPromise(std::move(promise));
  std::pair<bool, int> init_result = internal_->Init(argc, argv);

  if (init_result.first) {
    internal_->Free();
    return init_result.second;
  }

  int result = internal_->Run();
  internal_->Free();

  return result;
}

void Runtime::Stop() {
  internal_->runner_.Stop();
}

std::shared_ptr<Port> Runtime::GetPort() {
  return internal_->runner_.GetPort();
}

/**************************************************************************
 * Runtime::Configuration class
 **************************************************************************/

Runtime::Configuration::Configuration()
    : internal_(new Runtime::Configuration::Internal()) {}

Runtime::Configuration::~Configuration() {
  delete internal_;
}

Runtime::Configuration& Runtime::Configuration::operator=(
    Configuration&& other) {
  delete internal_;
  internal_ = other.internal_;
  other.internal_ = nullptr;
  return *this;
}

void Runtime::Configuration::OnSendMessageSync(
    Runtime::SendMessageSyncCallback callback, void* user_data) {
  internal_->send_message_sync_callback = callback;
  internal_->send_message_sync_callback_data = user_data;
}

bool Runtime::Configuration::Set(const std::string& key, const char* value) {
  std::string value_string = value ? value : "";

  if (key == "lwnode_data_path") {
    LWNODE_DEV_LOG("[Runtime::Configuration::Set] data path set to %s",
                   value_string.c_str());
    internal_->lwnode_data_path = value_string;
    return true;
  }
  return false;
}

bool Runtime::Configuration::Set(const std::string& key, int value) {
  if (key == "gc_interval") {
    LWNODE_DEV_LOG("[Runtime::Configuration::Set] GC interval set to %dms",
                   value);
    LWNode::GlobalConfiguration::GetInstance().set_gc_interval(value);
    return true;
  } else if (key == "gc_free_space_divisor") {
    LWNODE_DEV_LOG(
        "[Runtime::Configuration::Set] GC free space divisor set to %d", value);
    LWNode::GlobalConfiguration::GetInstance().set_gc_free_space_divisor(value);
    return true;
  }
  return false;
}

bool Runtime::Configuration::Set(const std::string& key, bool value) {
  return false;
}

/**************************************************************************
 * Static functions
 **************************************************************************/

bool ParseAULEvent(int argc, char** argv) {
  bool result = AULEventReceiver::getInstance()->start(argc, argv);
  if (result) {
    LWNode::SystemInfo::getInstance()->add("aul");
  }

  return result;
}

bool InitScriptRootPath(const std::string path) {
  int result;

#if defined(HOST_TIZEN) && defined(LWNODE_TIZEN_AUL)
  if (path.empty()) {
    char* path = app_get_resource_path();
    result = uv_chdir(path);
    free(path);
    return result == 0;
  }
#endif

  result = uv_chdir(path.c_str());

  if (result != 0) {
    LWNODE_DEV_LOG("ERROR: Failed to change directory. (%d)\n", -errno);

    return false;
  }

  return true;
}

void SetDlogID(const std::string& tag) {
#if defined(HOST_TIZEN) && !defined(LWNODE_TIZEN_AUL)
  if (!tag.empty()) {
    LogKind::user()->tag = tag;
  }

  LogOption::setDefaultOutputInstantiator([]() {
    static thread_local std::shared_ptr<Logger::Output> s_loggerOutput;
    if (s_loggerOutput == nullptr) {
      s_loggerOutput =
          std::static_pointer_cast<Logger::Output>(std::make_shared<DlogOut>());
    }
    return s_loggerOutput;
  });
#endif
}

int Start(int argc, char** argv) {
  return node::Start(argc, argv);
}

}  // namespace lwnode
