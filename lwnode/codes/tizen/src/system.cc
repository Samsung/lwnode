/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

#if defined(HOST_TIZEN)

#include "node.h"
#include "v8.h"
#include "env-inl.h"
#include "node_escargot.h"
#include <system_info.h>
#include <string>

namespace nescargot {

using node::Environment;
using namespace v8;

static std::string GetSystemInfo(char* name) {
  char* value = NULL;
  char key[256];
  snprintf(key, sizeof(key), "http://tizen.org/system/%s", name);
  std::string retValue;

  int ret = system_info_get_platform_string(key, &value);
  if (ret != SYSTEM_INFO_ERROR_NONE) {
    node::nescargot_printf_warn("Getting %s failed", name);
  } else {
    retValue = value;
    free(value);
  }
  return retValue;
}

#define SYSINFO_GET_API(F) \
  F(tizenid, tizenID)      \
  F(model_name, modelName)

static void Init(Local<Object> target, Local<Value> unused,
                 Local<Context> context) {
  Environment* env = Environment::GetCurrent(context);

  enum PropertyAttribute attributes =
      static_cast<PropertyAttribute>(ReadOnly | DontDelete);

#define SYSINFO_SET_PROPERY(name, Name)                                      \
  target                                                                     \
      ->DefineOwnProperty(                                                   \
          env->context(), FIXED_ONE_BYTE_STRING(env->isolate(), #Name),      \
          String::NewFromUtf8(env->isolate(), GetSystemInfo(#name).c_str()), \
          attributes)                                                        \
      .FromJust();

  SYSINFO_GET_API(SYSINFO_SET_PROPERY)

#undef SYSINFO_SET_PROPERY
#undef SYSINFO_GET_API
}

}  // namespace nescargot

NODE_MODULE_CONTEXT_AWARE_BUILTIN(system, nescargot::Init)

#endif
