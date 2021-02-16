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

#include "libplatform/tracing/trace-writer.h"
#include "unimplemented.h"
#include "v8.h"

namespace v8 {
namespace platform {
namespace tracing {

JSONTraceWriter::JSONTraceWriter(std::ostream& stream)
    : JSONTraceWriter(stream, "") {}

JSONTraceWriter::JSONTraceWriter(std::ostream& stream, const std::string& tag)
    : stream_(stream) {
  LWNODE_UNIMPLEMENT;
}

JSONTraceWriter::~JSONTraceWriter() {
  LWNODE_UNIMPLEMENT;
}

void JSONTraceWriter::AppendTraceEvent(TraceObject* trace_event) {
  LWNODE_RETURN_VOID;
}

void JSONTraceWriter::Flush() {
  LWNODE_RETURN_VOID;
}

void JSONTraceWriter::AppendArgValue(uint8_t type,
                                     TraceObject::ArgValue value) {
  LWNODE_RETURN_VOID;
}

void JSONTraceWriter::AppendArgValue(v8::ConvertableToTraceFormat*) {
  LWNODE_RETURN_VOID;
}

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
  LWNODE_RETURN_NULLPTR;
}
}  // namespace tracing
}  // namespace platform
}  // namespace v8
