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
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define NAPI_CALL_ASSERT(the_call) \
  do {                             \
    assert((the_call) == napi_ok); \
  } while (0)

struct WorkerHandler {
  napi_async_work handle;
  napi_value jsObject;
};

pthread_t ThreadId() { return pthread_self(); }

static WorkerHandler g_workerHandler;
static pthread_t g_mainThreadId;

void* ThreadWork(void* data) {
  napi_env env = (napi_env)data;

  // Call JavaScript function in main thread
  WorkerHandler* handler = new WorkerHandler;
  NAPI_CALL_ASSERT(napi_create_async_work(
      env, [](napi_env env, void* data) {},
      [](napi_env env, napi_status status, void* data) {
        assert(g_mainThreadId == ThreadId());
        WorkerHandler* handler = (WorkerHandler*)data;

        napi_value global, test_str, test_fn;
        NAPI_CALL_ASSERT(napi_get_global(env, &global));
        NAPI_CALL_ASSERT(napi_create_string_utf8(env, "test", 4, &test_str));
        NAPI_CALL_ASSERT(napi_get_property(env, g_workerHandler.jsObject,
                                           test_str, &test_fn));
        NAPI_CALL_ASSERT(
            napi_call_function(env, global, test_fn, 0, NULL, NULL));

        delete handler;
      },
      handler, &handler->handle));
  NAPI_CALL_ASSERT(napi_queue_async_work(env, handler->handle));
}

void Execute(napi_env env, void* data) {
  pthread_t thread_t;
  int status;

  // Create thread
  if (pthread_create(&thread_t, NULL, ThreadWork, (void*)env) < 0) {
    perror("thread create error:");
    exit(1);
  }
  pthread_join(thread_t, (void**)&status);
  printf("Thread End %d\n", status);
}

napi_value Run(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL_ASSERT(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  g_workerHandler.jsObject = args[0];

  NAPI_CALL_ASSERT(napi_create_async_work(env, Execute, NULL, &g_workerHandler,
                                          &g_workerHandler.handle));
  NAPI_CALL_ASSERT(napi_queue_async_work(env, g_workerHandler.handle));

  return nullptr;
}

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

void Init(napi_env env, napi_value exports, napi_value module, void* priv) {
  g_mainThreadId = ThreadId();

  napi_property_descriptor desc = DECLARE_NAPI_METHOD("run", Run);
  NAPI_CALL_ASSERT(napi_define_properties(env, exports, 1, &desc));
}

NAPI_MODULE(async, Init)
