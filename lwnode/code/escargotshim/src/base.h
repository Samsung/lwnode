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

#pragma once

#include "unimplemented.h"

#define VAL(that) reinterpret_cast<const ValueWrap*>(that)

#define PRIVATE_UTIL_1(_isolate, bailout_value)                                \
  if (_isolate->IsExecutionTerminating()) {                                    \
    return bailout_value;                                                      \
  }

#define API_ENTER(isolate, bailout_value)                                      \
  auto _isolate = IsolateWrap::fromV8(isolate);                                \
  PRIVATE_UTIL_1(_isolate, bailout_value)

#define API_ENTER_NO_EXCEPTION(isolate)                                        \
  auto _isolate = IsolateWrap::fromV8(isolate);

#define API_ENTER_WITH_CONTEXT(context, bailout_value)                         \
  auto _isolate = context.IsEmpty() ? IsolateWrap::currentIsolate()            \
                                    : VAL(*context)->context()->GetIsolate();  \
  PRIVATE_UTIL_1(_isolate, bailout_value)

// V has parameters (Type, type, TYPE, C type)
#define TYPED_ARRAYS(V)                                                        \
  V(Uint8, uint8, UINT8, uint8_t)                                              \
  V(Int8, int8, INT8, int8_t)                                                  \
  V(Uint16, uint16, UINT16, uint16_t)                                          \
  V(Int16, int16, INT16, int16_t)                                              \
  V(Uint32, uint32, UINT32, uint32_t)                                          \
  V(Int32, int32, INT32, int32_t)                                              \
  V(Float32, float32, FLOAT32, float)                                          \
  V(Float64, float64, FLOAT64, double)                                         \
  V(Uint8Clamped, uint8_clamped, UINT8_CLAMPED, uint8_t)                       \
  V(BigUint64, biguint64, BIGUINT64, uint64_t)                                 \
  V(BigInt64, bigint64, BIGINT64, int64_t)
