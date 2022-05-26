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

#include "global.h"

#include "api/context.h"
#include "error-message.h"
#include "es-helper.h"
#include "stack-trace.h"
#include "utils/misc.h"

namespace EscargotShim {
using namespace Escargot;

std::unique_ptr<Flags> Global::s_flags = std::make_unique<Flags>();

Flags* Global::flags() {
  return s_flags.get();
}

void Global::initGlobalObject(ContextWrap* lwContext) {
#if defined(HOST_TIZEN)
// @todo setup device APIs
#endif
  auto context = lwContext->get();

  auto globalObjectData = new GlobalObjectData();
  globalObjectData->setInternalFieldCount(
      GlobalObjectData::kInternalFieldCount);
  globalObjectData->setInternalField(GlobalObjectData::kContextWrapSlot,
                                     lwContext);
  ObjectRefHelper::setExtraData(context->globalObject(), globalObjectData);

  initErrorObject(context);
  initEvalObject(context);
}

void Global::initErrorObject(ContextRef* context) {
  Evaluator::EvaluatorResult r =
      Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
        auto errorObject = state->context()
                               ->globalObject()
                               ->get(state, StringRef::createFromASCII("Error"))
                               ->asObject();

        errorObject->set(state,
                         StringRef::createFromASCII("captureStackTrace"),
                         StackTrace::createCaptureStackTrace(state));

        // TODO: get number from '--stack-trace-limit' options
        errorObject->set(state,
                         StringRef::createFromASCII("stackTraceLimit"),
                         ValueRef::create(20));

        return ValueRef::createUndefined();
      });

  LWNODE_CHECK(r.isSuccessful());
}

static ValueRef* throwExceptionWhenEvalCalled(ExecutionStateRef* state,
                                              ValueRef* thisValue,
                                              size_t argc,
                                              ValueRef** argv,
                                              bool isConstructCall) {
  state->throwException(ExceptionHelper::createErrorObject(
      state->context(), ErrorMessageType::kDisallowCodeGeneration));
  return ValueRef::createUndefined();
}

void Global::initEvalObject(ContextRef* context) {
  if (flags()->isOn(Flag::Type::DisallowCodeGenerationFromStrings)) {
    ObjectRefHelper::addNativeFunction(context,
                                       context->globalObject(),
                                       StringRef::createFromASCII("eval"),
                                       throwExceptionWhenEvalCalled);
  }
}

}  // namespace EscargotShim
