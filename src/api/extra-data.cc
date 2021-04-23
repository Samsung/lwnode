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

#include "v8.h"
#include "extra-data.h"
#include "utils/misc.h"
#include "isolate.h"

using namespace Escargot;

namespace EscargotShim {

static inline bool checkOutofBounds(ObjectData* data, int idx) {
  return idx >= data->internalFieldCount() || idx < 0;
}

int ObjectData::internalFieldCount() {
  if (m_internalFields == nullptr) {
    return 0;
  }
  return m_internalFields->size();
}

void ObjectData::setInternalFieldCount(int size) {
  // TODO: throw internal error
  LWNODE_CHECK_NULL(m_internalFields);
  if (size <= 0) {
    LWNODE_DLOG_ERROR("InternalField: The size is negative");
    return;
  }
  m_internalFields = new GCContainer<void*>(size);
  for (int i = 0; i < size; i++) {
    m_internalFields->set(i, IsolateWrap::GetCurrent()->undefined());
  }
}

void ObjectData::setInternalField(int idx, void* lwValue) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(m_internalFields);
  if (checkOutofBounds(this, idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return;
  }
  m_internalFields->set(idx, lwValue);
}

void* ObjectData::internalField(int idx) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(m_internalFields);
  if (checkOutofBounds(this, idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return nullptr;
  }
  return m_internalFields->get(idx);
}

}  // namespace EscargotShim
