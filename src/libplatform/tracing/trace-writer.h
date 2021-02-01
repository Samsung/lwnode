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
