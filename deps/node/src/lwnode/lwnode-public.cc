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

#include "lwnode-public.h"
#include "lwnode.h"
#include "lwnode/aul-event-receiver.h"
#include "node.h"
#include "node_main_lw_runner-inl.h"
#include "trace.h"
#include "v8.h"

using namespace node;

namespace lwnode {

struct Runtime::Configuration::Internal {
  Runtime::SendMessageSyncCallback send_message_sync_callback{nullptr};
  void* send_message_sync_callback_data{nullptr};
};

class Runtime::Internal {
  friend Runtime;

 public:
  std::pair<bool, int> Init(int argc, char** argv) {
    is_initialized = true;

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
    if (instance_ == nullptr) {
      return -1;
    }

    return runner_.Run(*instance_);
  }

  void Free() {
    if (is_initialized && instance_) {
      DisposeNode(instance_);
    }
  }

 private:
  NodeMainInstance* instance_{nullptr};
  LWNode::LWNodeMainRunner runner_;
  Runtime::Configuration config_;
  bool is_initialized{false};
};

/**************************************************************************
 * Runtime class
 **************************************************************************/

Runtime::Runtime() : internal_(new Internal()) {}

Runtime::Runtime(Configuration&& config) : Runtime() {
  internal_->config_ = std::move(config);
}

Runtime::~Runtime() {
  delete internal_;
}

int Runtime::Start(int argc, char** argv, std::promise<void>&& promise) {
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
    LWNODE_DEV_LOGF("ERROR: Failed to change directory. (%d)\n", -errno);

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
