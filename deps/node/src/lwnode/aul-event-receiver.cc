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
      LWNODE_LOG_INFO("AUL_START");
      char* json = nullptr;
      if (BUNDLE_ERROR_NONE != bundle_to_json(b, &json)) {
        LWNODE_LOG_ERROR("bundle_to_json");
        return 0;
      }
      LWNODE_LOG_INFO("[AUL] %s", json);
      // NOTE: usage: process.on('message', (message) => {})
      // nescargot::push_aul_message(json);
      free(json);
      break;
    }
    case AUL_RESUME:
      LWNODE_LOG_INFO("AUL_RESUME");
      break;
    case AUL_TERMINATE:
      LWNODE_LOG_INFO("AUL_TERMINATE");
      // nescargot::push_aul_termination_message();
      break;
    default:
      LWNODE_LOG_INFO("AUL EVENT (%d)", type);
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
            LWNODE_LOG_INFO("bundle - key: %s, value: %s", key, value);
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
    aul_launch_init(aulEventHandler, nullptr);
    aul_launch_argv_handler(argc, argv);

    char appid[kMaxPackageNameSize + 1];
    aul_app_get_appid_bypid(getpid(), appid, kMaxPackageNameSize);
    appid_ = appid;

    if (uv_chdir(app_get_resource_path()) != 0) {
      LWNODE_LOG_ERROR("Failed to change directory. (%d)", -errno);
      exit(-errno);
    }
    isEventReceiverRunning_ = true;
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
  LogOption::setDefaultOutputInstantiator([&]() {
    static thread_local std::shared_ptr<Logger::Output> s_loggerOutput;
    if (s_loggerOutput == nullptr) {
      s_loggerOutput = isEventReceiverRunning()
                           ? std::static_pointer_cast<Logger::Output>(
                                 std::make_shared<DlogOut>())
                           : std::static_pointer_cast<Logger::Output>(
                                 std::make_shared<DlogOut>());
    }
    return s_loggerOutput;
  });
}
