/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
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

#include <dlfcn.h>

#include "node.h"
#include "v8.h"
#include "env-inl.h"
#include "node_escargot_logger.h"

#define SIGNING_CHECKER_CALLER "node"
#define SIGNING_CHECKER_KUEP_PATH "/usr/lib/libkUEPUser.so"
#define SIGNING_CHECKER_VERIFY_FUNC "verifyFile"
#define SIGNING_CHECKER_KEY_FILE "/usr/lib/key.pub"

namespace nescargot {

using namespace node;
using namespace v8;

typedef int (*VerifyPluginInterface)(const char* key, const char* path,
                                     const char* caller);

static void UEPVerify(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  v8::String::Utf8Value filePath(args[0]);
  if (*filePath == NULL) {
    args.GetReturnValue().Set(Boolean::New(isolate, false));
  }

  static VerifyPluginInterface kuepVerifier = NULL;

  char* error = NULL;
  void* pHandle = NULL;

  if (kuepVerifier == NULL) {
    if (pHandle == NULL) {
      if ((pHandle = dlopen(SIGNING_CHECKER_KUEP_PATH,
                            RTLD_LAZY | RTLD_LOCAL)) == NULL) {
        NODE_ESCARGOT_LOG_ERROR("[Signing Checker] pHandle Error!\n");
        args.GetReturnValue().Set(Boolean::New(isolate, false));
        return;
      }
    }

    dlerror();

    kuepVerifier =
        (VerifyPluginInterface)dlsym(pHandle, SIGNING_CHECKER_VERIFY_FUNC);

    if ((error = dlerror()) != NULL) {
      NODE_ESCARGOT_LOG_ERROR("[Signing Checker] dlsym Error!\n");
      dlclose(pHandle);
      pHandle = NULL;
      args.GetReturnValue().Set(Boolean::New(isolate, false));
      return;
    }
  }

  int ret =
      kuepVerifier(SIGNING_CHECKER_KEY_FILE, *filePath, SIGNING_CHECKER_CALLER);

  if (ret) {
    NODE_ESCARGOT_LOG_INFO(
        "[Signing Checker] The file(%s) is signed correctly!\n", *filePath);
  } else {
    NODE_ESCARGOT_LOG_ERROR("[Signing Checker] The file(%s) is NOT signed!\n",
                            *filePath);
  }
  args.GetReturnValue().Set(Boolean::New(isolate, !!ret));
}

static void Init(Local<Object> target, Local<Value> unused,
                 Local<Context> context) {
  Environment* env = Environment::GetCurrent(context);

  target->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "verify"),
              env->NewFunctionTemplate(UEPVerify)->GetFunction());
}

}  // namespace nescargot

NODE_MODULE_CONTEXT_AWARE_BUILTIN(signing_checker, nescargot::Init)

#endif
