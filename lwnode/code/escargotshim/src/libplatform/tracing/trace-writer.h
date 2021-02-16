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

#ifndef V8_LIBPLATFORM_TRACING_TRACE_WRITER_H_
#define V8_LIBPLATFORM_TRACING_TRACE_WRITER_H_

#include "include/libplatform/v8-tracing.h"

namespace v8 {
namespace platform {
namespace tracing {

class JSONTraceWriter : public TraceWriter {
 public:
  explicit JSONTraceWriter(std::ostream& stream);
  JSONTraceWriter(std::ostream& stream, const std::string& tag);
  ~JSONTraceWriter() override;
  void AppendTraceEvent(TraceObject* trace_event) override;
  void Flush() override;

 private:
  void AppendArgValue(uint8_t type, TraceObject::ArgValue value);
  void AppendArgValue(v8::ConvertableToTraceFormat*);

  std::ostream& stream_;
  bool append_comma_ = false;
};

}  // namespace tracing
}  // namespace platform
}  // namespace v8

#endif  // V8_LIBPLATFORM_TRACING_TRACE_WRITER_H_
