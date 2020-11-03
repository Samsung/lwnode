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

#include "escargotutil.h"

#include "v8.h"

#include "libplatform/v8-tracing.h"

namespace v8 {
namespace platform {
namespace tracing {

static const size_t kMaxCategoryGroups = 200;
unsigned char g_category_group_enabled[kMaxCategoryGroups] = {0};

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
  NESCARGOT_UNIMPLEMENTED("");
}

TraceObject::~TraceObject() {
  // Intentionally left empty to suppress warning C4722.
}

void TraceConfig::AddIncludedCategory(char const*) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

TraceObject* TraceBufferChunk::AddTraceEvent(size_t*) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
  // TODO: check who frees a TraceObject
  return nullptr;
}

void TraceBufferChunk::Reset(uint32_t) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

TraceBufferChunk::TraceBufferChunk(uint32_t) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

void TracingController::StopTracing() {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

void TracingController::StartTracing(TraceConfig*) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

void TracingController::Initialize(TraceBuffer*) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

const uint8_t* TracingController::GetCategoryGroupEnabled(
    const char* category_group) {
  NESCARGOT_UNIMPLEMENTED("");
  return &g_category_group_enabled[0];
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
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

void TracingController::UpdateTraceEventDuration(
    const uint8_t* category_enabled_flag, const char* name, uint64_t handle) {
  NESCARGOT_UNIMPLEMENTED("");
}

void TracingController::RemoveTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  NESCARGOT_UNIMPLEMENTED("");
}

int64_t TracingController::CurrentCpuTimestampMicroseconds() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

void TracingController::AddTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  NESCARGOT_UNIMPLEMENTED("");
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
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

int64_t TracingController::CurrentTimestampMicroseconds() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

}  // namespace tracing
}  // namespace platform
}  // namespace v8
