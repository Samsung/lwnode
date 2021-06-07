/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

#pragma once

#include <EscargotPublic.h>
#include <v8.h>
#include "handle.h"

using namespace Escargot;

namespace EscargotShim {

class FunctionCallbackInfoWrap : public v8::FunctionCallbackInfo<v8::Value> {
 public:
  using T = v8::FunctionCallbackInfo<v8::Value>;

  FunctionCallbackInfoWrap(v8::Isolate* isolate,
                           ValueRef* holder,
                           ValueRef* thisValue,
                           OptionalRef<ObjectRef> newTarget,
                           ValueWrap* data,
                           int argc,
                           ValueRef** argv);
  ~FunctionCallbackInfoWrap();

  HandleWrap** toWrapperArgs(ValueRef* thisValue, int argc, ValueRef** argv);

 private:
  HandleWrap** m_args{nullptr};
  HandleWrap* m_implicitArgs[T::kArgsLength];
};

template <typename T>
class PropertyCallbackInfoWrap : public v8::PropertyCallbackInfo<T> {
 public:
  using F = v8::PropertyCallbackInfo<T>;

  PropertyCallbackInfoWrap(v8::Isolate* isolate,
                           ValueRef* holder,
                           ValueRef* thisValue,
                           ValueWrap* data);

  bool hasReturnValue();

 private:
  HandleWrap* m_implicitArgs[F::kArgsLength];
};

}  // namespace EscargotShim
