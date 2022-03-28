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

#include "stack-trace.h"
#include "api.h"
#include "base.h"
#include "context.h"
#include "isolate.h"

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

size_t StackTrace::getStackTraceLimit(ExecutionStateRef* state) {
  auto errorObject = state->context()->globalObject()->get(
      state, StringRef::createFromASCII("Error"));
  LWNODE_CHECK(errorObject->isObject());

  auto stackTraceLimitValue = errorObject->asObject()->get(
      state, StringRef::createFromASCII("stackTraceLimit"));
  LWNODE_CHECK(stackTraceLimitValue->isNumber());

  return stackTraceLimitValue->asNumber();
}

ValueRef* StackTrace::StackTraceGetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ValueRef* receiver,
    ObjectRef::NativeDataAccessorPropertyData* data) {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();
  auto accessorData = reinterpret_cast<NativeAccessorProperty*>(data);

  if (lwIsolate->HasPrepareStackTraceCallback()) {
    auto sites =
        ArrayObjectRef::create(state, accessorData->stackTraceVector());

    auto formattedStackTrace =
        lwIsolate->RunPrepareStackTraceCallback(state, lwContext, self, sites);
    if (formattedStackTrace) {
      return formattedStackTrace;
    }
  }

  return ValueRef::createUndefined();
}

bool StackTrace::StackTraceSetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ValueRef* receiver,
    ObjectRef::NativeDataAccessorPropertyData* data,
    ValueRef* setterInputData) {
  LWNODE_RETURN_FALSE;
}

ValueRef* StackTrace::captureStackTraceCallback(ExecutionStateRef* state,
                                                ValueRef* thisValue,
                                                size_t argc,
                                                ValueRef** argv,
                                                bool isConstructCall) {
  if (argc < 1 || !argv[0]->isObject()) {
    return ValueRef::createUndefined();
  }

  auto exceptionObject = argv[0]->asObject();
  auto callSite = IsolateWrap::GetCurrent()->GetCurrentContext()->callSite();
  auto stackTrace = state->computeStackTrace();
  auto stackTraceVector = ValueVectorRef::create();

  std::string filterFunctionName;
  if (argc > 1 && argv[1]->isFunctionObject()) {
    auto name = argv[1]->asFunctionObject()->get(
        state, StringRef::createFromASCII("name"));
    if (name->isString()) {
      filterFunctionName = name->asString()->toStdUTF8String();
    }
  }

  // TODO: handle `constructorOpt` option :
  // compare the address of constructorOpt function not name.
  int stacktraceStartIdx = 1;
  size_t stackTraceLimit = getStackTraceLimit(state) + stacktraceStartIdx;

  for (size_t i = stacktraceStartIdx;
       i < stackTraceLimit && i < stackTrace.size();
       i++) {
    if (!filterFunctionName.empty() &&
        filterFunctionName == stackTrace[i].functionName->toStdUTF8String()) {
      break;
    }

    stackTraceVector->pushBack(
        callSite->instantiate(state->context(), stackTrace[i]));
  }

  addStackProperty(state, exceptionObject, stackTraceVector);

  return ValueRef::createUndefined();
}

void StackTrace::addStackProperty(ExecutionStateRef* state,
                                  ObjectRef* object,
                                  ValueVectorRef* stackTraceVector) {
  // NOTE: either Error or Exception contains stack.
  object->defineNativeDataAccessorProperty(
      state,
      StringRef::createFromUTF8("stack"),
      new NativeAccessorProperty(false,
                                 false,
                                 false,
                                 StackTraceGetter,
                                 StackTraceSetter,
                                 stackTraceVector));
}

ValueRef* StackTrace::createCaptureStackTrace(
    Escargot::ExecutionStateRef* state) {
  FunctionObjectRef::NativeFunctionInfo info(
      AtomicStringRef::create(state->context(), "captureStackTrace"),
      captureStackTraceCallback,
      1,
      true,
      false);

  return FunctionObjectRef::create(state, info);
}

ValueRef* StackTrace::createPrepareStackTrace(
    Escargot::ExecutionStateRef* state) {
  FunctionObjectRef::NativeFunctionInfo info(
      AtomicStringRef::create(state->context(), "prepareStackTrace"),
      prepareStackTraceCallback,
      2,
      true,
      false);

  return FunctionObjectRef::create(state, info);
}

ValueRef* StackTrace::prepareStackTraceCallback(ExecutionStateRef* state,
                                                ValueRef* thisValue,
                                                size_t argc,
                                                ValueRef** argv,
                                                bool isConstructCall) {
  // TODO: JS registers prepareStackTrace() to reformat StackTrace.
  LWNODE_UNIMPLEMENT;
  return StringRef::emptyString();
}

StringRef* StackTrace::formatStackTraceStringNodeStyle(ObjectRef* errorObject,
                                                       size_t maxStackSize) {
  std::ostringstream oss;
  auto message = errorObject->toString(state_)->toStdUTF8String();
  if (message.length() > 0) {
    oss << message << "\n";
  }

  auto traceData = state_->computeStackTrace();
  size_t maxPrintStackSize = std::min((int)maxStackSize, (int)traceData.size());

  const std::string separator = "    ";
  for (size_t i = 0; i < maxPrintStackSize; ++i) {
    oss << separator << "at " << formatStackTraceLine(traceData[i])
        << std::endl;
  }

  auto stringNodeStyle = oss.str();
  return StringRef::createFromUTF8(stringNodeStyle.c_str(),
                                   stringNodeStyle.length());
}

std::string StackTrace::formatStackTraceLine(
    const Evaluator::StackTraceData& line) {
  std::ostringstream oss;

  const auto& iter = line;
  const auto& resourceName = iter.srcName->toStdUTF8String();
  const auto& functionName = iter.functionName->toStdUTF8String();
  const int errorLine = iter.loc.line;
  const int errorColumn = iter.loc.column;

  oss << (functionName == "" ? "Object.<anonymous>" : functionName) << " "
      << "(" << (resourceName == "" ? "?" : resourceName) << ":" << errorLine
      << ":" << errorColumn << ")";

  return oss.str();
}

ArrayObjectRef* StackTrace::genCallSites() {
  auto stackTrace = state_->computeStackTrace();
  auto stackTraceVector = ValueVectorRef::create();
  auto callSite = IsolateWrap::GetCurrent()->GetCurrentContext()->callSite();

  for (size_t i = 0; i < stackTrace.size(); i++) {
    stackTraceVector->pushBack(
        callSite->instantiate(state_->context(), stackTrace[i]));
  }

  return ArrayObjectRef::create(state_, stackTraceVector);
}

CallSite::CallSite(ContextRef* context) : context_(context) {
  template_ = FunctionTemplateRef::create(
      AtomicStringRef::create(context, "CallSite"),
      0,
      true,
      true,
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        return ValueRef::createUndefined();
      });
  injectSitePrototype();
}

ValueRef* CallSite::instantiate(ContextRef* context,
                                const Evaluator::StackTraceData& data) {
  auto callSite = template_->instanceTemplate()->instantiate(context);
  ExtraDataHelper::setExtraData(callSite, new StackTraceData(data));
  return callSite;
};

void CallSite::injectSitePrototype() {
  setCallSitePrototype(
      "getFunctionName",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        return data->functionName();
      });

  setCallSitePrototype(
      "getFileName",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        return data->src();
      });

  setCallSitePrototype(
      "getLineNumber",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        return ValueRef::create(data->loc().line);
      });

  setCallSitePrototype(
      "getColumnNumber",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        return ValueRef::create(data->loc().column);
      });

  setCallSitePrototype(
      "isEval",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        return ValueRef::create(data->isEval());
      });

  setCallSitePrototype(
      "toString",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        std::ostringstream stream;
        stream << data->functionName()->toStdUTF8String() << " ("
               << data->src()->toStdUTF8String() << ":" << data->loc().line
               << ":" << data->loc().column << ")";
        auto string = stream.str();
        return StringRef::createFromUTF8(string.data(), string.length());
      });

  setCallSitePrototype(
      "getFunction",
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         bool isNewExpression) -> ValueRef* {
        auto data = ObjectRefHelper::getExtraData(thisValue->asObject())
                        ->asStackTraceData();
        auto esFunction = data->callee();
        if (esFunction.hasValue()) {
          return esFunction.get();
        }
        return ValueRef::createUndefined();
      });
}

void CallSite::setCallSitePrototype(
    const std::string& name,
    Escargot::FunctionObjectRef::NativeFunctionPointer fn) {
  EvalResult r = Evaluator::execute(
      context_,
      [](ExecutionStateRef* state,
         const std::string* name,
         Escargot::FunctionObjectRef::NativeFunctionPointer fn) -> ValueRef* {
        return FunctionObjectRef::create(
            state,
            FunctionObjectRef::NativeFunctionInfo(
                AtomicStringRef::create(
                    state->context(), name->c_str(), name->length()),
                fn,
                0,
                true,
                false));
      },
      &name,
      fn);
  LWNODE_CHECK(r.isSuccessful());

  template_->prototypeTemplate()->set(
      StringRef::createFromUTF8(name.c_str(), name.length()),
      r.result,
      false,
      false,
      true);
}

}  // namespace EscargotShim
