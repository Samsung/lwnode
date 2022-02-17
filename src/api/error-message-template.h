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

#pragma once

namespace EscargotShim {

#define ERROR_MESSAGE_TEMPLATES(T)                                             \
  T(None, None, "")                                                            \
  T(DataCloneErrorOutOfMemory,                                                 \
    RangeError,                                                                \
    "Data cannot be cloned, out of memory.")                                   \
  T(InternalFieldsOutOfRange, RangeError, "Internal field out of bounds.")     \
  T(NotReadValue, RangeError, "Cannot read value")                             \
  T(IllegalInvocation, TypeError, "Illegal invocation")                        \
  T(DisallowCodeGeneration,                                                    \
    EvalError,                                                                 \
    "Code generation from strings disallowed for this context")
}  // namespace EscargotShim
