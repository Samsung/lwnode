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

#include <node.h>
#include <v8.h>
#include "TizenDeviceAPILoaderForEscargot.h"
#include "lwnode/lwnode.h"

using namespace LWNode;

void Init(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  auto esContext = Utils::ToEsContext(*isolate->GetCurrentContext());

  auto instance = DeviceAPI::ExtensionManagerInstanceGet(esContext);
  if (!instance) {
    DeviceAPI::initialize(esContext);
  }
}

void initModule(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  NODE_SET_METHOD(exports, "init", Init);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, initModule)
