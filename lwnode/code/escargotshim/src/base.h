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

#define VAL(that) reinterpret_cast<ValueWrap*>(that)
#define CVAL(that) reinterpret_cast<const ValueWrap*>(that)

#define __TERMINATION_CHECK(lwIsolate, bailout_value)                          \
  if (lwIsolate->IsExecutionTerminating()) {                                   \
    return bailout_value;                                                      \
  }

#if !defined(NDEBUG)
#define __DLOG_EVAL_ERROR(eval_result)                                         \
  LWNODE_DLOG_ERROR("Evaluate");                                               \
  LWNODE_DLOG_RAW("Execute:\n  %s (%s:%d)\n%s",                                \
                  TRACE_ARGS2,                                                 \
                  EvalResultHelper::getErrorString(                            \
                      lwIsolate->GetCurrentContext()->get(), eval_result)      \
                      .c_str());
#else
#define __DLOG_EVAL_ERROR(eval_result)
#endif

#define API_ENTER(isolate, bailout_value)                                      \
  LWNODE_CALL_TRACE();                                                         \
  IsolateWrap* lwIsolate = IsolateWrap::fromV8(isolate);                       \
  __TERMINATION_CHECK(lwIsolate, bailout_value)

#define API_ENTER_NO_EXCEPTION(isolate)                                        \
  LWNODE_CALL_TRACE();                                                         \
  IsolateWrap* lwIsolate = IsolateWrap::fromV8(isolate);

#define API_ENTER_WITH_CONTEXT(local_context, bailout_value)                   \
  LWNODE_CALL_TRACE();                                                         \
  IsolateWrap* lwIsolate = local_context.IsEmpty()                             \
                               ? IsolateWrap::GetCurrent()                     \
                               : VAL(*local_context)->context()->GetIsolate(); \
  __TERMINATION_CHECK(lwIsolate, bailout_value)

#define API_HANDLE_EXCEPTION(eval_result, lwIsolate, bailout_value)            \
  if (!eval_result.isSuccessful()) {                                           \
    __DLOG_EVAL_ERROR(eval_result);                                            \
    lwIsolate->setStackTrace(eval_result.stackTraceData);                      \
    lwIsolate->ScheduleThrow(eval_result.error.get());                         \
    return bailout_value;                                                      \
  }

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
