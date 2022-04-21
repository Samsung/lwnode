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

class PrepareStackTraceScope {
 public:
  explicit PrepareStackTraceScope(IsolateWrap* isolate) : isolate_(isolate) {
    LWNODE_DCHECK(!isolate_->prepareStackTraceRecursion());
    isolate_->setPrepareStackTraceRecursion(true);
  }

  ~PrepareStackTraceScope() { isolate_->setPrepareStackTraceRecursion(false); }

 private:
  IsolateWrap* isolate_;
};

bool StackTrace::getStackTraceLimit(ExecutionStateRef* state,
                                    double& stackTraceLimit) {
  auto errorObject = state->context()->globalObject()->get(
      state, StringRef::createFromASCII("Error"));
  LWNODE_CHECK(errorObject->isObject());

  auto stackTraceLimitValue = errorObject->asObject()->get(
      state, StringRef::createFromASCII("stackTraceLimit"));

  if (!stackTraceLimitValue->isNumber()) {
    return false;
  }

  stackTraceLimit = stackTraceLimitValue->asNumber();
  return true;
}

ValueRef* StackTrace::StackTraceGetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ValueRef* receiver,
    ObjectRef::NativeDataAccessorPropertyData* data) {
  auto accessorData = reinterpret_cast<NativeAccessorProperty*>(data);
  if (accessorData->hasStackValue()) {
    return accessorData->stackValue();
  }

  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();

  if (!accessorData->stackTrace()) {
    auto undefined = ValueRef::createUndefined();
    accessorData->setStackValue(undefined);
    return undefined;
  }

  if (!lwIsolate->prepareStackTraceRecursion() &&
      lwIsolate->HasPrepareStackTraceCallback()) {
    // Avoid calling this function multiple times with 'error.stack' in
    // 'PrepareStackTraceCallback'.
    PrepareStackTraceScope scope(lwIsolate);
    auto formattedStackTrace = lwIsolate->RunPrepareStackTraceCallback(
        state, lwContext, self, accessorData->stackTrace());
    if (!formattedStackTrace->isUndefined()) {
      accessorData->setStackValue(formattedStackTrace);
      return formattedStackTrace;
    }
  }

  StackTrace stackTrace(state, self);
  auto stackTraceString =
      stackTrace.formatStackTraceStringNodeStyle(accessorData->stackTrace());
  accessorData->setStackValue(stackTraceString);
  return stackTraceString;
}

bool StackTrace::StackTraceSetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ValueRef* receiver,
    ObjectRef::NativeDataAccessorPropertyData* data,
    ValueRef* setterInputData) {
  auto accessorData = reinterpret_cast<NativeAccessorProperty*>(data);
  accessorData->setStackValue(setterInputData);
  return true;
}

bool StackTrace::checkFilter(ValueRef* filter,
                             const Evaluator::StackTraceData& traceData) {
  return (filter && traceData.callee.hasValue() &&
          filter == traceData.callee.get());
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
  auto stackString = StringRef::createFromUTF8("stack");
  if (exceptionObject->hasOwnProperty(state, stackString)) {
    exceptionObject->deleteOwnProperty(state, stackString);
  }

  auto callSite = IsolateWrap::GetCurrent()->GetCurrentContext()->callSite();
  auto stackTraceData = state->computeStackTrace();
  auto stackTraceVector = ValueVectorRef::create();

  ValueRef* filterFunction = nullptr;
  if (argc > 1 && argv[1]->isFunctionObject()) {
    filterFunction = argv[1];
  }

  int stacktraceStartIdx = 1;

  double stackTraceLimit = 0;
  if (!getStackTraceLimit(state, stackTraceLimit)) {
    stackTraceVector = nullptr;
  } else {
    size_t maxPrintStackSize =
        std::min(stackTraceLimit, (double)stackTraceData.size());
    for (size_t i = 0; i < maxPrintStackSize; i++) {
      if (StackTrace::checkFilter(filterFunction, stackTraceData[i])) {
        if (stackTraceVector->size() > 0) {
          stackTraceVector->erase(0, stackTraceVector->size());
        }
        filterFunction = nullptr;

        continue;
      }

      stackTraceVector->pushBack(
          callSite->instantiate(state->context(), stackTraceData[i]));
    }
  }

  // FIXME: it seems there are some cases where we need to freeze the
  // stack string here. Investigate further
  StackTrace stackTrace(state, exceptionObject);
  stackTrace.addStackProperty(ArrayObjectRef::create(state, stackTraceVector));

  return ValueRef::createUndefined();
}

void StackTrace::addStackProperty(ArrayObjectRef* stackTrace) {
  // NOTE: either Error or Exception contains stack.
  error_->defineNativeDataAccessorProperty(
      state_,
      StringRef::createFromUTF8("stack"),
      new NativeAccessorProperty(
          true, false, true, StackTraceGetter, StackTraceSetter, stackTrace));
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

StringRef* StackTrace::formatStackTraceStringNodeStyle(
    ArrayObjectRef* stackTrace) {
  std::ostringstream oss;
  auto message = error_->toString(state_)->toStdUTF8String();
  if (message.length() > 0) {
    oss << message;
  }

  auto esContext = state_->context();
  size_t maxPrintStackSize =
      ArrayObjectRefHelper::length(esContext, stackTrace);

  if (maxPrintStackSize > 0) {
    oss << std::endl;
  }

  const std::string separator = "    ";

  for (size_t i = 0; i < maxPrintStackSize; ++i) {
    auto callSiteString = ArrayObjectRefHelper::get(esContext, stackTrace, i)
                              ->toString(state_)
                              ->toStdUTF8String();
    oss << separator << "at " << callSiteString << std::endl;
  }

  auto stringNodeStyle = oss.str();
  return StringRef::createFromUTF8(stringNodeStyle.c_str(),
                                   stringNodeStyle.length());
}

std::string StackTrace::formatStackTraceLine(
    const Evaluator::StackTraceData& line) {
  std::ostringstream oss;

  auto stdFunctionName = line.functionName->toStdUTF8String();
  // TODO: 'anonymous' is not unique. 'stdFunctionName' should be unique.
  // 'anonymous' keyword is related to ScriptCompiler::CompileFunctionInContext.
  if (stdFunctionName == "anonymous") {
    stdFunctionName = "";
  }

  if (stdFunctionName.empty()) {
    oss << line.srcName->toStdUTF8String() << ":" << line.loc.line << ":"
        << line.loc.column;
  } else {
    oss << stdFunctionName << " (" << line.srcName->toStdUTF8String() << ":"
        << line.loc.line << ":" << line.loc.column << ")";
  }

  return oss.str();
}

ArrayObjectRef* StackTrace::genCallSites(
    const GCManagedVector<Evaluator::StackTraceData>& stackTraceData) {
  auto callSite = IsolateWrap::GetCurrent()->GetCurrentContext()->callSite();

  double stackTraceLimit = 0;
  if (!getStackTraceLimit(state_, stackTraceLimit)) {
    return nullptr;
  }

  auto stackTraceVector = ValueVectorRef::create();
  size_t maxPrintStackSize =
      std::min(stackTraceLimit, (double)stackTraceData.size());

  for (size_t i = 0; i < maxPrintStackSize; i++) {
    stackTraceVector->pushBack(
        callSite->instantiate(state_->context(), stackTraceData[i]));
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

ValueRef* CallSite::instantiate(ContextRef* context,
                                EscargotShim::StackTraceData* data) {
  auto callSite = template_->instanceTemplate()->instantiate(context);
  ExtraDataHelper::setExtraData(callSite, data);
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
        auto string = StackTrace::formatStackTraceLine(data->stackTraceData());

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
