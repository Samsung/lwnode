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

bool InternalFieldData::isValidIndex(int idx) {
  return 0 <= idx && idx < internalFieldCount();
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

ObjectData::ObjectData(FunctionObjectRef* functionObject)
    : TemplateData(), functionObject_(functionObject) {}

int InternalFieldData::internalFieldCount() {
  if (internalFields_ == nullptr) {
    return 0;
  }
  return internalFields_->size();
}

void InternalFieldData::setInternalFieldCount(int size) {
  LWNODE_CALL_TRACE_ID(OBJDATA, "%d", size);

  // TODO: throw internal error
  if (size <= 0) {
    LWNODE_DLOG_ERROR("InternalField: Invalid field count: %d\n", size);
    return;
  }

  internalFields_ = new GCContainer<void*>(size);
  for (int i = 0; i < size; i++) {
    setInternalField(i, nullptr);
  }
}

void InternalFieldData::setInternalField(int idx, void* lwValue) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(internalFields_);
  if (!isValidIndex(idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return;
  }

  // TODO: Update type
  // LWNODE_CALL_TRACE_ID(
  //     OBJDATA, "%s", toObjectDataString(this, idx, lwValue).c_str());

  internalFields_->set(idx, lwValue);
}

void* InternalFieldData::internalField(int idx) {
  // TODO: throw internal error
  LWNODE_CHECK_NOT_NULL(internalFields_);
  if (!isValidIndex(idx)) {
    LWNODE_DLOG_ERROR("InternalField: Internal field out of bounds");
    return nullptr;
  }

  void* field = internalFields_->get(idx);
  if (field) {
    // TODO: Update type
    // LWNODE_CALL_TRACE_ID(
    //     OBJDATA, "%s", toObjectDataString(this, idx, field).c_str());
  }

  return field;
}

ObjectData::ObjectData(ObjectTemplateRef* objectTemplate)
    : TemplateData(ExtraDataHelper::getExtraData(objectTemplate)
                       ->asObjectTemplateData()
                       ->functionTemplate()),
      objectTemplate_(objectTemplate) {}

ObjectData* ObjectTemplateData::createObjectData(
    ObjectTemplateRef* objectTemplate) {
  auto newData = new ObjectData(objectTemplate);

  LWNODE_CALL_TRACE_ID_LOG(EXTRADATA,
                           "ObjectTemplateData(%p)::createObjectData: %p",
                           objectTemplate,
                           newData);

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

v8::Isolate* FunctionData::isolate() {
  if (functionTemplate_) {
    auto functionTemplateData = ExtraDataHelper::getExtraData(functionTemplate_)
                                    ->asFunctionTemplateData();
    return functionTemplateData->isolate();
  }

  return nullptr;
}

v8::FunctionCallback FunctionData::callback() {
  if (functionTemplate_) {
    auto functionTemplateData = ExtraDataHelper::getExtraData(functionTemplate_)
                                    ->asFunctionTemplateData();
    return functionTemplateData->callback();
  }

  return v8::FunctionCallback();
}

v8::Value* FunctionData::callbackData() {
  if (functionTemplate_) {
    auto functionTemplateData = ExtraDataHelper::getExtraData(functionTemplate_)
                                    ->asFunctionTemplateData();
    return functionTemplateData->callbackData();
  }

  return nullptr;
}

v8::Signature* FunctionData::signature() {
  if (functionTemplate_) {
    auto functionTemplateData = ExtraDataHelper::getExtraData(functionTemplate_)
                                    ->asFunctionTemplateData();
    return functionTemplateData->signature();
  }

  return nullptr;
}

//  A receiver matches a given signature if the receiver (or any of its
//  hidden prototypes) was created from the signature's FunctionTemplate, or
//  from a FunctionTemplate that inherits directly or indirectly from the
//  signature's FunctionTemplate.
bool FunctionData::checkSignature(Escargot::ExecutionStateRef* state,
                                  ObjectRef* receiver) {
  auto calleeSignature = signature();
  if (calleeSignature == nullptr) {
    return true;
  }

  auto esContext = state->context();

  // e.g., 1.x();
  // 1 is not created by FunctionTemplate
  auto extraData = ExtraDataHelper::getExtraData(receiver);
  if (extraData == nullptr) {
    // receiver is not created by functionTemplate.
    return false;
  }

  FunctionTemplateRef* functionTemplate = nullptr;
  if (extraData->isObjectData()) {
    functionTemplate = extraData->asObjectData()->functionTemplate();
  } else if (extraData->isObjectTemplateData()) {
    functionTemplate = extraData->asObjectTemplateData()->functionTemplate();
  } else {
    LWNODE_CHECK(false);
  }

  auto calleeFunctionTemplate = CVAL(calleeSignature)->ftpl();
  for (auto p = functionTemplate; p; p = p->parent().value()) {
    if (p == calleeFunctionTemplate) {
      return true;
    }
  }

  return false;
}

static bool isValidStackFrame(const Evaluator::StackTraceData& traceData) {
  const int errorLine = traceData.loc.line;
  const int errorColumn = traceData.loc.column;
  // simple filter for { [native function] } :-1:-1)
  return (errorLine > 0 && errorColumn > 0);
}

ExceptionObjectData::ExceptionObjectData(
    GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData,
    bool isThisExceptionUndefined)
    : isThisExceptionUndefined_(isThisExceptionUndefined) {
  constexpr const char* kNativeFunction = "[native function]";

  for (size_t i = 0; i < stackTraceData.size(); i++) {
    const auto& traceData = stackTraceData[i];
    if (isValidStackFrame(traceData) == false) {
      LWNODE_DLOG_INFO("filtered: stack frame #%zu", i);
      continue;
    }
    stackTraces_.push_back(new StackTraceData(traceData));
  }
}

GCVector<StackTraceData*>* ExceptionObjectData::stackTrace(
    ObjectRef* exceptionObject) {
  auto exceptionObjectData =
      ExtraDataHelper::getExtraData(exceptionObject)->asExceptionObjectData();

  return exceptionObjectData->stackTrace();
}

}  // namespace EscargotShim
