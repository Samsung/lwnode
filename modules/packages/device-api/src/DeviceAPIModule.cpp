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

using namespace LWNode;

static napi_value InitMethod(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_context context;

  status = napi_get_context(env, context);
  assert(status == napi_ok);

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
              napi_status status;

              status = napi_open_handle_scope(params->env, &scope);
              assert(status == napi_ok);

              auto result = params->idler(params->data);

              status = napi_close_handle_scope(params->env, scope);
              assert(status == napi_ok);

              delete params;
              return result;
            },
            new Params(idler, data, env));
      });

  auto instance = DeviceAPI::ExtensionManagerInstanceGet(esContext);
  if (!instance) {
    DeviceAPI::initialize(esContext);
  }

  return nullptr;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value InitModule(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("init", InitMethod);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, InitModule)
