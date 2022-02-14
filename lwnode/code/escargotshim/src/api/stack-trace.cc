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

class NativeDataAccessorPropertyDataForStackTrace
    : public ObjectRef::NativeDataAccessorPropertyData {
 public:
  NativeDataAccessorPropertyDataForStackTrace(
      bool isWritable,
      bool isEnumerable,
      bool isConfigurable,
      ObjectRef::NativeDataAccessorPropertyGetter getter,
      ObjectRef::NativeDataAccessorPropertySetter setter,
      ValueVectorRef* stackTraceVector)
      : NativeDataAccessorPropertyData(
            isWritable, isEnumerable, isConfigurable, getter, setter),
        stackTraceVector_(stackTraceVector) {}

  ValueVectorRef* stackTraceVector() { return stackTraceVector_; }

  void* operator new(size_t size) { return GC_MALLOC(size); }

 private:
  ValueVectorRef* stackTraceVector_;
};

static size_t getStackTraceLimit(ExecutionStateRef* state) {
  auto errorObject = state->context()->globalObject()->get(
      state, StringRef::createFromASCII("Error"));
  LWNODE_CHECK(errorObject->isObject());

  auto stackTraceLimitValue = errorObject->asObject()->get(
      state, StringRef::createFromASCII("stackTraceLimit"));
  LWNODE_CHECK(stackTraceLimitValue->isNumber());

  return stackTraceLimitValue->asNumber();
}

static ValueRef* StackTraceGetter(
    ExecutionStateRef* state,
    ObjectRef* self,
    ValueRef* receiver,
    ObjectRef::NativeDataAccessorPropertyData* data) {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto lwContext = lwIsolate->GetCurrentContext();
  auto accessorData =
      reinterpret_cast<NativeDataAccessorPropertyDataForStackTrace*>(data);

  if (lwIsolate->HasPrepareStackTraceCallback()) {
    auto sites =
        ArrayObjectRef::create(state, accessorData->stackTraceVector());
    v8::MaybeLocal<v8::Value> callbackResult =
        lwIsolate->PrepareStackTraceCallback()(
            v8::Utils::NewLocal<Context>(lwIsolate->toV8(), lwContext),
            v8::Utils::NewLocal<Value>(lwIsolate->toV8(), self),
            v8::Utils::NewLocal<Array>(lwIsolate->toV8(), sites));

    if (!callbackResult.IsEmpty()) {
      Local<Value> callbackResultLocal;
      if (callbackResult.ToLocal(&callbackResultLocal)) {
        return CVAL(*callbackResultLocal)->value();
      }
    }
  }

  return ValueRef::createUndefined();
}

static bool StackTraceSetter(ExecutionStateRef* state,
                             ObjectRef* self,
                             ValueRef* receiver,
                             ObjectRef::NativeDataAccessorPropertyData* data,
                             ValueRef* setterInputData) {
  LWNODE_RETURN_FALSE;
}

static ValueRef* captureStackTraceCallback(ExecutionStateRef* state,
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

  exceptionObject->defineNativeDataAccessorProperty(
      state,
      StringRef::createFromUTF8("stack"),
      new NativeDataAccessorPropertyDataForStackTrace(false,
                                                      false,
                                                      false,
                                                      StackTraceGetter,
                                                      StackTraceSetter,
                                                      stackTraceVector));

  return ValueRef::createUndefined();
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

static void setCallSitePrototype(
    ContextRef* context,
    ObjectTemplateRef* otpl,
    const char* name,
    Escargot::FunctionObjectRef::NativeFunctionPointer fn) {
  auto length = strlen(name);

  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         const char* name,
         size_t length,
         Escargot::FunctionObjectRef::NativeFunctionPointer fn) -> ValueRef* {
        return FunctionObjectRef::create(
            state,
            FunctionObjectRef::NativeFunctionInfo(
                AtomicStringRef::create(state->context(), name, length),
                fn,
                0,
                true,
                false));
      },
      name,
      length,
      fn);
  LWNODE_CHECK(r.isSuccessful());

  otpl->set(
      StringRef::createFromUTF8(name, length), r.result, false, false, true);
}

static void injectSitePrototype(ContextRef* context, ObjectTemplateRef* otpl) {
  setCallSitePrototype(
      context,
      otpl,
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
      context,
      otpl,
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
      context,
      otpl,
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
      context,
      otpl,
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
      context,
      otpl,
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
      context,
      otpl,
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
      context,
      otpl,
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

CallSite::CallSite(ContextRef* context) {
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
  injectSitePrototype(context, template_->prototypeTemplate());
}

ValueRef* CallSite::instantiate(ContextRef* context,
                                const Evaluator::StackTraceData& data) {
  auto callSite = template_->instanceTemplate()->instantiate(context);
  ExtraDataHelper::setExtraData(callSite, new StackTraceData(data));
  return callSite;
};

}  // namespace EscargotShim
