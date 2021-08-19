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

#include "extra-data.h"
#include "base.h"
#include "context.h"
#include "es-helper.h"
#include "isolate.h"
#include "utils/misc.h"
#include "v8.h"

using namespace Escargot;

namespace EscargotShim {

static inline bool checkOutofBounds(ObjectData* data, int idx) {
  return idx >= data->internalFieldCount() || idx < 0;
}

static std::string toObjectDataString(const ObjectData* data,
                                      int index,
                                      const void* field) {
  std::ostringstream oss;

  oss << " fields (" << data << ")";
  oss << " [" << index << "]";

  if (field && CVAL(field)->isValid()) {
    oss << " " << CVAL(field)->getHandleInfoString();
  } else {
    oss << " " << field;
  }
  return oss.str();
}

int ObjectData::internalFieldCount() {
  if (m_internalFields == nullptr) {
    return 0;
  }
  return m_internalFields->size();
}

void ObjectData::setInternalFieldCount(int size) {
  LWNODE_CALL_TRACE_ID(OBJDATA, "%d", size);

  // TODO: throw internal error
  if (size <= 0) {
    LWNODE_DLOG_ERROR("InternalField: The size is negative");
    return;
  }

  m_internalFields = new GCContainer<void*>(size);
  for (int i = 0; i < size; i++) {
    setInternalField(i, nullptr);
  }
}

void ObjectData::setInternalField(int idx, void* lwValue) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(m_internalFields);
  if (checkOutofBounds(this, idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return;
  }

  LWNODE_CALL_TRACE_ID(
      OBJDATA, "%s", toObjectDataString(this, idx, lwValue).c_str());

  m_internalFields->set(idx, lwValue);
}

void* ObjectData::internalField(int idx) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(m_internalFields);
  if (checkOutofBounds(this, idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return nullptr;
  }

  void* field = m_internalFields->get(idx);
  if (field) {
    LWNODE_CALL_TRACE_ID(
        OBJDATA, "%s", toObjectDataString(this, idx, field).c_str());
  }

  return field;
}

ObjectData* ObjectData::clone() {
  auto newData = new ObjectData();

  LWNODE_CALL_TRACE_ID(OBJDATA, "%p clone: %p", this, newData);

  // copy internalField
  auto count = internalFieldCount();
  if (count > 0) {
    newData->setInternalFieldCount(count);
  }
  for (int i = 0; i < count; i++) {
    LWNODE_DCHECK(internalField(i) == nullptr);
    newData->setInternalField(i, internalField(i));
  }

  return newData;
}

void ObjectData::setInstanceTemplate(Escargot::FunctionTemplateRef* tpl) {
  instanceTemplate_ = tpl;
}

Escargot::FunctionTemplateRef* ObjectData::instanceTemplate() {
  return instanceTemplate_;
}

bool FunctionData::checkSignature(Escargot::ExecutionStateRef* state,
                                  ValueRef* thisValue) {
  if (m_signature == nullptr) {
    return true;
  }
  auto esContext = state->context();
  auto receiver = CVAL(m_signature)->ftpl()->instantiate(esContext);
  auto r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState,
         ValueRef* thisValue,
         ObjectRef* receiver) -> ValueRef* {
        return ValueRef::create(thisValue->instanceOf(esState, receiver));
      },
      thisValue,
      receiver);
  LWNODE_CHECK(r.isSuccessful());
  return r.result->asBoolean();
}

GCVector<ExceptionObjectData::StackTraceData*>* ExceptionObjectData::stackTrace(
    ObjectRef* exceptionObject) {
  auto exceptionObjectData = static_cast<ExceptionObjectData*>(
      ObjectRefHelper::getExtraData(exceptionObject));
  return exceptionObjectData->stackTrace();
}

}  // namespace EscargotShim
