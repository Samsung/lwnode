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

#include "lwnode/lwnode.h"
#include <EscargotPublic.h>
#include "api.h"
#include "base.h"

using namespace v8;
using namespace EscargotShim;

namespace LWNode {

void InitializeProcessMethods(Local<Object> target, Local<Context> context) {
  auto esContext = CVAL(*context)->context()->get();
  auto esObject = CVAL(*target)->value()->asObject();

#if !defined(NDEBUG)
  EvalResultHelper::attachBuiltinPrint(esContext, esObject);
#endif
}

}  // namespace LWNode
