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

#include <stdint.h>

namespace v8 {
class Isolate;
}

namespace EscargotShim {
namespace internal {

typedef uintptr_t Address;

class Internals {
 public:
  static const int kUndefinedValueRootIndex = 0;
  static const int kTheHoleValueRootIndex = 1;
  static const int kNullValueRootIndex = 2;
  static const int kTrueValueRootIndex = 3;
  static const int kFalseValueRootIndex = 4;
  static const int kEmptyStringRootIndex = 5;
  static const int kDefaultReturnValueRootIndex = 6;
  static const int kRootIndexSize = 7;

  static Address* GetRoot(v8::Isolate* isolate, int index);
};

}  // namespace internal
}  // namespace EscargotShim
