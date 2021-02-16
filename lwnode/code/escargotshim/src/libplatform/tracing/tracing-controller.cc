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

// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unimplemented.h"
#include "include/libplatform/v8-tracing.h"

namespace v8 {
namespace platform {
namespace tracing {

TracingController::TracingController() {
  LWNODE_UNIMPLEMENT;
}

TracingController::~TracingController() {
  LWNODE_UNIMPLEMENT;
  StopTracing();
}

void TracingController::Initialize(TraceBuffer* trace_buffer) {
  LWNODE_RETURN_VOID;
}

int64_t TracingController::CurrentTimestampMicroseconds() {
  LWNODE_RETURN_0;
}

int64_t TracingController::CurrentCpuTimestampMicroseconds() {
  LWNODE_RETURN_0;
}

uint64_t TracingController::AddTraceEvent(
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
    unsigned int flags) {
  LWNODE_RETURN_0;
}

uint64_t TracingController::AddTraceEventWithTimestamp(
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
    int64_t timestamp) {
  LWNODE_RETURN_0;
}

void TracingController::UpdateTraceEventDuration(
    const uint8_t* category_enabled_flag, const char* name, uint64_t handle) {
  LWNODE_RETURN_VOID;
}

const char* TracingController::GetCategoryGroupName(
    const uint8_t* category_group_enabled) {
  LWNODE_RETURN_NULLPTR;
}

void TracingController::StartTracing(TraceConfig* trace_config) {
  LWNODE_RETURN_VOID;
}

void TracingController::StopTracing() {
  LWNODE_RETURN_VOID;
}

void TracingController::UpdateCategoryGroupEnabledFlag(size_t category_index) {
  LWNODE_RETURN_VOID;
}

void TracingController::UpdateCategoryGroupEnabledFlags() {
  LWNODE_RETURN_VOID;
}

const uint8_t* TracingController::GetCategoryGroupEnabled(
    const char* category_group) {
  LWNODE_RETURN_NULLPTR;
}

void TracingController::AddTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  LWNODE_RETURN_VOID;
}

void TracingController::RemoveTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  LWNODE_RETURN_VOID;
}

}  // namespace tracing
}  // namespace platform
}  // namespace v8
