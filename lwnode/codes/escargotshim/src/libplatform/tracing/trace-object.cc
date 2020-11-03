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

// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "escargotshim-base.h"
#include "include/libplatform/v8-tracing.h"
#include "include/v8-platform.h"

namespace v8 {
namespace platform {
namespace tracing {

void TraceObject::Initialize(
    char phase,
    const uint8_t* category_enabled_flag,
    const char* name,
    const char* scope,
    uint64_t id,
    uint64_t bind_id,
    int num_args,
    const char** arg_names,
    const uint8_t* arg_types,
    const uint64_t* arg_values,
    std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
    unsigned int flags,
    int64_t timestamp,
    int64_t cpu_timestamp) {
  LWNODE_RETURN_VOID;
}

TraceObject::~TraceObject() {
  LWNODE_UNIMPLEMENT;
  delete[] parameter_copy_storage_;
}

void TraceObject::UpdateDuration(int64_t timestamp, int64_t cpu_timestamp) {
  LWNODE_RETURN_VOID;
}

void TraceObject::InitializeForTesting(
    char phase,
    const uint8_t* category_enabled_flag,
    const char* name,
    const char* scope,
    uint64_t id,
    uint64_t bind_id,
    int num_args,
    const char** arg_names,
    const uint8_t* arg_types,
    const uint64_t* arg_values,
    std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
    unsigned int flags,
    int pid,
    int tid,
    int64_t ts,
    int64_t tts,
    uint64_t duration,
    uint64_t cpu_duration) {
  LWNODE_RETURN_VOID;
}

}  // namespace tracing
}  // namespace platform
}  // namespace v8
