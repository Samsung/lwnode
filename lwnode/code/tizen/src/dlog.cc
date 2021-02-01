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
#include <dlog.h>
#include <string>

namespace nescargot {

using node::Environment;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

static void logI(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  node::nescargot_printf(*String::Utf8Value(args[0]->ToString()));
  args.GetReturnValue().Set(Undefined(env->isolate()));
}

static void logW(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  node::nescargot_printf_warn(*String::Utf8Value(args[0]->ToString()));
  args.GetReturnValue().Set(Undefined(env->isolate()));
}

static void logE(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  node::nescargot_printf_err(*String::Utf8Value(args[0]->ToString()));
  args.GetReturnValue().Set(Undefined(env->isolate()));
}

static void Init(Local<Object> target, Local<Value> unused,
                 Local<Context> context) {
  Environment* env = Environment::GetCurrent(context);

  target->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "info"),
              env->NewFunctionTemplate(logI)->GetFunction());
  target->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "warn"),
              env->NewFunctionTemplate(logW)->GetFunction());
  target->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "error"),
              env->NewFunctionTemplate(logE)->GetFunction());
}

}  // namespace nescargot

NODE_MODULE_CONTEXT_AWARE_BUILTIN(dlog, nescargot::Init)

#endif
