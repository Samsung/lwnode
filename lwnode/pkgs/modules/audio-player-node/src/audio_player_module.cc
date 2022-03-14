/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include <node_api.h>
#include <stdio.h>
#include <mutex>
#include <condition_variable>

#include "common.h"
#include "bridge/service.h"

namespace modules {

struct WorkerHandler {
  napi_async_work handle;
};

static WorkerHandler g_workerHandler;
static bool g_isCreatedWorkerQueue = false;
static bool g_isStopRequested = false;
static std::mutex g_mutex;
static std::condition_variable g_condition;

void Execute(napi_env env, void* data) {
  WorkerHandler* handler = static_cast<WorkerHandler*>(data);

  if (handler != &g_workerHandler) {
    napi_throw_type_error(env, "Wrong data parameter to Execute.");
    return;
  }

  std::unique_lock<std::mutex> lock(g_mutex);

  printf("Execute: Service starts (%s)\n", SPOTIFY_USERNAME);
  service_start(SPOTIFY_USERNAME, SPOTIFY_PASSWORD);

  g_condition.wait(lock, [] { return g_isStopRequested; });

  service_stop();
}

void Complete(napi_env env, napi_status status, void* data) {
  printf("Complete\n");

  WorkerHandler* handler = static_cast<WorkerHandler*>(data);
  if (handler != &g_workerHandler) {
    napi_throw_type_error(env, "Wrong data parameter to Complete.");
    return;
  }

  if (status != napi_ok) {
    napi_throw_type_error(env, "Execute callback failed.");
    return;
  }

  g_isCreatedWorkerQueue = false;

  NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, handler->handle));
}

napi_value Stop(napi_env env, napi_callback_info info) {
  g_isStopRequested = true;
  g_condition.notify_one();
  return nullptr;
}

napi_value Start(napi_env env, napi_callback_info info) {
  if (g_isCreatedWorkerQueue) {
    return nullptr;
  }

  g_isStopRequested = false;
  g_isCreatedWorkerQueue = true;

  NAPI_CALL(env,
            napi_create_async_work(env, Execute, Complete, &g_workerHandler,
                                   &g_workerHandler.handle));

  NAPI_CALL(env, napi_queue_async_work(env, g_workerHandler.handle));

  return nullptr;
}

void Init(napi_env env, napi_value exports, napi_value module, void* priv) {
  napi_property_descriptor properties[] = {
      DECLARE_NAPI_PROPERTY("start", Start),
      DECLARE_NAPI_PROPERTY("stop", Stop),
  };

  NAPI_CALL_RETURN_VOID(
      env,
      napi_define_properties(
          env, exports, sizeof(properties) / sizeof(*properties), properties));
}

NAPI_MODULE(audio_player, Init)

}  // namespace modules
