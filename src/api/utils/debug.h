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

#include "EscargotPublic.h"
#include "v8.h"

namespace EscargotShim {
class DebugUtils {
 public:
  static void printStackTrace();
  static void printObject(Escargot::ObjectRef* value, int depth = 0);
  static void printToString(Escargot::ValueRef* value);
  static std::string v8StringToStd(v8::Isolate* isolate,
                                   v8::Local<v8::Value> value);
};
}  // namespace EscargotShim
