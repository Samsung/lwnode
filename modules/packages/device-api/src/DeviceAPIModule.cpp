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

#include <glib.h>
#include <js_native_api.h>
#include <lwnode_api.h>
#include <node_api.h>
#include "Extension.h"
#include "TizenDeviceAPILoaderForEscargot.h"
#include "lwnode/lwnode.h"

#define NAPI_CALL(call)                                                        \
  do {                                                                         \
    napi_status status = call;                                                 \
    assert(status == napi_ok && #call " failed");                              \
  } while (0);

using namespace LWNode;

static void runInitialScript(napi_env env) {
  const char* scriptText =
      "if(!global.WebAPIException){ "
      "global.WebAPIException = xwalk.utils.global.WebAPIException;}";
  napi_value script, result;
  NAPI_CALL(
      napi_create_string_utf8(env, scriptText, strlen(scriptText), &script));
  NAPI_CALL(napi_run_script(env, script, &result));
}

static napi_value InitMethod(napi_env env, napi_callback_info info) {
  napi_context context;

  NAPI_CALL(napi_get_context(env, context));

  auto esContext = Utils::ToEsContext(context);

  struct Params {
    Params(DeviceAPI::ESPostMessageListener::Idler_t idler,
           void* data,
           napi_env env)
        : idler(idler), data(data), env(env) {}
    DeviceAPI::ESPostMessageListener::Idler_t idler{nullptr};
    void* data{nullptr};
    napi_env env{nullptr};
  };

  DeviceAPI::ESPostMessageListener::SetMainThreadIdlerRegister(
      [env](DeviceAPI::ESPostMessageListener::Idler_t idler, void* data) {
        g_idle_add(
            [](void* data) {
              Params* params = reinterpret_cast<Params*>(data);
              napi_handle_scope scope = nullptr;

              NAPI_CALL(napi_open_handle_scope(params->env, &scope));

              auto result = params->idler(params->data);

              NAPI_CALL(napi_close_handle_scope(params->env, scope));

              delete params;
              return result;
            },
            new Params(idler, data, env));
      });

  auto instance = DeviceAPI::ExtensionManagerInstanceGet(esContext);
  if (!instance) {
    DeviceAPI::initialize(esContext);
  }

  runInitialScript(env);

  return nullptr;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value InitModule(napi_env env, napi_value exports) {
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("init", InitMethod);
  NAPI_CALL(napi_define_properties(env, exports, 1, &desc));
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, InitModule)
