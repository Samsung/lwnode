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
#include "v8-profiler.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

void HeapProfiler::RemoveBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  NESCARGOT_UNIMPLEMENTED("");
}

void HeapProfiler::AddBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  NESCARGOT_UNIMPLEMENTED("");
}

HeapCodeStatistics::HeapCodeStatistics()
    : code_and_metadata_size_(0),
      bytecode_and_metadata_size_(0),
      external_script_source_size_(0) {
  NESCARGOT_UNIMPLEMENTED("");
}

void HeapSnapshot::Serialize(OutputStream* stream,
                             HeapSnapshot::SerializationFormat format) const {
  NESCARGOT_UNIMPLEMENTED("");
}

void HeapSnapshot::Delete() {
  NESCARGOT_UNIMPLEMENTED("");
}
}  // namespace v8
