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

#if defined(HOST_TIZEN) && defined(NESCARGOT_TIZEN_TV)

#include "node.h"
#include "v8.h"
#include "env-inl.h"
#include "node_escargot.h"
#include <system_settings.h>
#include <string>

namespace nescargot {

using node::Environment;
using namespace v8;

static std::string GetSystemSettingsTvName() {
  char* value = NULL;
  std::string retValue;

  auto ret = system_settings_get_value_string(
      SYSTEM_SETTINGS_KEY_DEVICE_TV_NAME, &value);
  if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
    node::nescargot_printf_warn("Getting tv name failed");
  } else {
    retValue = value;
    free(value);
  }
  return retValue;
}

static void Init(Local<Object> target, Local<Value> unused,
                 Local<Context> context) {
  Environment* env = Environment::GetCurrent(context);

  enum PropertyAttribute attributes =
      static_cast<PropertyAttribute>(ReadOnly | DontDelete);

  target
      ->DefineOwnProperty(
          env->context(), FIXED_ONE_BYTE_STRING(env->isolate(), "tvName"),
          String::NewFromUtf8(env->isolate(),
                              GetSystemSettingsTvName().c_str()),
          attributes)
      .FromJust();
}

}  // namespace nescargot

NODE_MODULE_CONTEXT_AWARE_BUILTIN(system_settings, nescargot::Init)

#endif
