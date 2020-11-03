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

#include "libplatform/tracing/trace-buffer.h"
#include "escargotshim-base.h"

namespace v8 {
namespace platform {
namespace tracing {

TraceBufferRingBuffer::TraceBufferRingBuffer(size_t max_chunks,
                                             TraceWriter* trace_writer)
    : max_chunks_(max_chunks) {
  LWNODE_UNIMPLEMENT;
}

TraceObject* TraceBufferRingBuffer::AddTraceEvent(uint64_t* handle) {
  LWNODE_RETURN_NULLPTR;
}

TraceObject* TraceBufferRingBuffer::GetEventByHandle(uint64_t handle) {
  LWNODE_RETURN_NULLPTR;
}

bool TraceBufferRingBuffer::Flush() {
  LWNODE_RETURN_FALSE;
}

uint64_t TraceBufferRingBuffer::MakeHandle(size_t chunk_index,
                                           uint32_t chunk_seq,
                                           size_t event_index) const {
  LWNODE_RETURN_0;
}

void TraceBufferRingBuffer::ExtractHandle(uint64_t handle,
                                          size_t* chunk_index,
                                          uint32_t* chunk_seq,
                                          size_t* event_index) const {
  LWNODE_RETURN_VOID;
}

size_t TraceBufferRingBuffer::NextChunkIndex(size_t index) const {
  LWNODE_RETURN_0;
}

TraceBufferChunk::TraceBufferChunk(uint32_t seq) : seq_(seq) {}

void TraceBufferChunk::Reset(uint32_t new_seq) {
  LWNODE_RETURN_VOID;
}

TraceObject* TraceBufferChunk::AddTraceEvent(size_t* event_index) {
  LWNODE_RETURN_NULLPTR;
}

TraceBuffer* TraceBuffer::CreateTraceBufferRingBuffer(
    size_t max_chunks, TraceWriter* trace_writer) {
  LWNODE_RETURN_NULLPTR;
}

}  // namespace tracing
}  // namespace platform
}  // namespace v8
