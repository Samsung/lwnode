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

#include "libplatform/tracing/trace-writer.h"

namespace v8 {
namespace platform {
namespace tracing {

JSONTraceWriter::JSONTraceWriter(std::ostream& stream)
    : JSONTraceWriter(stream, "") {}

JSONTraceWriter::JSONTraceWriter(std::ostream& stream, const std::string& tag)
    : stream_(stream) {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

JSONTraceWriter::~JSONTraceWriter() {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

void JSONTraceWriter::AppendTraceEvent(TraceObject* trace_event) {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

void JSONTraceWriter::Flush() {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

void JSONTraceWriter::AppendArgValue(uint8_t type,
                                     TraceObject::ArgValue value) {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

void JSONTraceWriter::AppendArgValue(v8::ConvertableToTraceFormat*) {
  NESCARGOT_UNIMPLEMENTED("JSONTraceWriter");
}

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
  NESCARGOT_UNIMPLEMENTED("TraceWriter");
  // TODO: check who frees a JSONTraceWriter
  return nullptr;
}
}  // namespace tracing
}  // namespace platform
}  // namespace v8
