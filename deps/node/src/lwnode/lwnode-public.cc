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
#include "trace.h"

namespace lwnode {

bool ParseAULEvent(int argc, char** argv) {
  bool result = AULEventReceiver::getInstance()->start(argc, argv);
  if (result) {
    LWNode::SystemInfo::getInstance()->add("aul");
  }

  return result;
}

bool InitScriptRootPath(const std::string path) {
#if defined(HOST_TIZEN)
  int result;
  if (path.empty()) {
    char* path = app_get_resource_path();
    result = uv_chdir(path);
    free(path);
  } else {
    result = uv_chdir(path.c_str());
  }

  if (result != 0) {
    LWNODE_DEV_LOGF("ERROR: Failed to change directory. (%d)\n", -errno);

    return false;
  }

  return true;
#else
  return false;
#endif
}

int Start(int argc, char** argv) {
  return node::Start(argc, argv);
}

}  // namespace LWNode
