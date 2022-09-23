/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

#include "aul-event-receiver.h"
#include <unistd.h>  // getpid
#include <uv.h>
#include "trace.h"

#ifdef HOST_TIZEN

int AULEventReceiver::aulEventHandler(aul_type type, bundle* b, void* data) {
  switch (type) {
    case AUL_START: {
      LWNODE_DEV_LOG("AUL_START");
      char* json = nullptr;
      if (BUNDLE_ERROR_NONE != bundle_to_json(b, &json)) {
        LWNODE_DEV_LOG("ERROR: bundle_to_json");
        return 0;
      }
      LWNODE_DEV_LOG("[AUL]", json);
      // NOTE: usage: process.on('message', (message) => {})
      // push_aul_message(json);
      free(json);
      break;
    }
    case AUL_RESUME:
      LWNODE_DEV_LOG("AUL_RESUME");
      break;
    case AUL_TERMINATE:
      LWNODE_DEV_LOG("AUL_TERMINATE");
      // push_aul_termination_message();
      break;
    default:
      LWNODE_DEV_LOGF("AUL EVENT (%d)", type);
      break;
  }
  return 0;
}

bool AULEventReceiver::hasAulArguments(int argc, char* argv[]) {
  bool result = false;

  bundle* parsed = bundle_import_from_argv(argc, argv);
  if (parsed) {
    if (bundle_get_val(parsed, AUL_K_STARTTIME)) {
      bundle_iterate(
          parsed,
          [](const char* key, const char* value, void* d) {
            LWNODE_DEV_LOGF("bundle - key: %s, value: %s", key, value);
          },
          NULL);
      result = true;
    }
    bundle_free(parsed);
  }

  return result;
}

bool AULEventReceiver::start(int argc, char* argv[]) {
  isEventReceiverRunning_ = false;

  if (hasAulArguments(argc, argv)) {
    isEventReceiverRunning_ = true;
    initLoggerOutput();

    aul_launch_init(aulEventHandler, nullptr);
    aul_launch_argv_handler(argc, argv);

    char appid[kMaxPackageNameSize + 1];
    aul_app_get_appid_bypid(getpid(), appid, kMaxPackageNameSize);
    appid_ = appid;

    LWNODE_DEV_LOG("appid: ", appid_);

    char* path = app_get_resource_path();
    if (uv_chdir(path) != 0) {
      LWNODE_DEV_LOGF("ERROR: Failed to change directory. (%d)", -errno);
      exit(-errno);
    }
    free(path);
    return isEventReceiverRunning_;
  }

  initLoggerOutput();
  return isEventReceiverRunning_;
}
#endif

AULEventReceiver* AULEventReceiver::getInstance() {
  static AULEventReceiver s_instance;
  return &s_instance;
}

bool AULEventReceiver::isEventReceiverRunning() {
  return isEventReceiverRunning_;
}

void AULEventReceiver::initLoggerOutput() {
  if (!appid_.empty()) {
    LogKind::user()->tag = appid_;
  }

  LogOption::setDefaultOutputInstantiator([&]() {
    static thread_local std::shared_ptr<Logger::Output> s_loggerOutput;
    if (s_loggerOutput == nullptr) {
      s_loggerOutput = isEventReceiverRunning()
                           ? std::static_pointer_cast<Logger::Output>(
                                 std::make_shared<DlogOut>())
                           : std::static_pointer_cast<Logger::Output>(
                                 std::make_shared<StdOut>());
    }
    return s_loggerOutput;
  });
}
