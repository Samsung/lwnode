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

#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

SnapshotCreator::SnapshotCreator(Isolate* isolate,
                                 const intptr_t* external_references,
                                 StartupData* existing_blob) {
  NESCARGOT_UNIMPLEMENTED("");
}

SnapshotCreator::SnapshotCreator(const intptr_t* external_references,
                                 StartupData* existing_blob) {
  NESCARGOT_UNIMPLEMENTED("");
}
SnapshotCreator::~SnapshotCreator() {
  NESCARGOT_UNIMPLEMENTED("");
}

Isolate* SnapshotCreator::GetIsolate() {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

}  // namespace v8
