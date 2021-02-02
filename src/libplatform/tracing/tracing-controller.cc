/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
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
