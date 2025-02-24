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

using namespace node;

namespace lwnode {

class Runtime::Internal {
  friend Runtime;

 public:
  ~Internal() {
    if (is_initialized && instance_) {
      DisposeNode(instance_);
    }
  }

  std::pair<bool, int> Init(int argc, char** argv) {
    is_initialized = true;
    return InitializeNode(argc, argv, &instance_);
  }

  int Run() {
    if (instance_ == nullptr) {
      return -1;
    }

    return runner_.Run(*instance_);
    ;
  }

 private:
  NodeMainInstance* instance_{nullptr};
  LWNode::LWNodeMainRunner runner_;
  bool is_initialized{false};
};

Runtime::Runtime() {
  internal_ = new Internal();
}

Runtime::~Runtime() {
  delete internal_;
}

std::pair<bool, int> Runtime::Init(int argc,
                                   char** argv,
                                   std::promise<void>&& promise) {
  internal_->runner_.SetInitPromise(std::move(promise));
  return internal_->Init(argc, argv);
}

int Runtime::Run() {
  return internal_->Run();
}

std::shared_ptr<Port> Runtime::GetPort() {
  return internal_->runner_.GetPort();
}

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
