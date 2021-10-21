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

#include "es-helper.h"
#include "context.h"
#include "extra-data.h"
#include "isolate.h"
#include "utils/misc.h"
#include "utils/string-util.h"

#include <sstream>

using namespace Escargot;

namespace EscargotShim {

// --- ObjectRefHelper ---

ObjectRef* ObjectRefHelper::create(ContextRef* context) {
  EvalResult r =
      Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
        return ObjectRef::create(state);
      });

  LWNODE_CHECK(r.isSuccessful());

  return r.result->asObject();
}

EvalResult ObjectRefHelper::setProperty(ContextRef* context,
                                        ObjectRef* object,
                                        ValueRef* key,
                                        ValueRef* value) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);
  LWNODE_DCHECK_NOT_NULL(value);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* param1,
         ValueRef* param2) -> ValueRef* {
        // 1. if failed because of its descriptor or strict mode, ObjectRef::set
        // returns false, but evalResult will has no error.
        // 2. if failed because of throwing an exception, evalResult will be an
        // error.
        return ValueRef::create(object->set(state, param1, param2));
      },
      object,
      key,
      value);
}

ValueRef* ObjectRefHelper::getOwnPropertyAttributes(ExecutionStateRef* state,
                                                    ObjectRef* object,
                                                    ValueRef* key) {
  auto val = object->getOwnPropertyDescriptor(state, key);
  if (val->isUndefined()) {
    return ValueRef::createUndefined();
  }

  int attr = ObjectRef::PresentAttribute::NotPresent;

  bool isWritable = val->asObject()
                        ->get(state, StringRef::createFromASCII("writable"))
                        ->asBoolean();
  bool isEnumerable = val->asObject()
                          ->get(state, StringRef::createFromASCII("enumerable"))
                          ->asBoolean();
  bool isConfigurable =
      val->asObject()
          ->get(state, StringRef::createFromASCII("configurable"))
          ->asBoolean();

  if (isWritable) {
    attr = attr | ObjectRef::PresentAttribute::WritablePresent;
  } else {
    attr = attr | ObjectRef::PresentAttribute::NonWritablePresent;
  }

  if (isEnumerable) {
    attr = attr | ObjectRef::PresentAttribute::EnumerablePresent;
  } else {
    attr = attr | ObjectRef::PresentAttribute::NonEnumerablePresent;
  }

  if (isConfigurable) {
    attr = attr | ObjectRef::PresentAttribute::ConfigurablePresent;
  } else {
    attr = attr | ObjectRef::PresentAttribute::NonConfigurablePresent;
  }

  return ValueRef::create(attr);
}

EvalResult ObjectRefHelper::getPropertyAttributes(ContextRef* context,
                                                  ObjectRef* object,
                                                  ValueRef* key,
                                                  bool skipPrototype) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* key,
         bool skipPrototype) -> ValueRef* {
        LWNODE_DCHECK_NOT_NULL(object);
        LWNODE_DCHECK_NOT_NULL(key);

        ValueRef* attr = ValueRef::createUndefined();
        for (ObjectRef* o = object; o;
             o = o->getPrototypeObject(state).value()) {
          attr = getOwnPropertyAttributes(state, o, key);
          if (skipPrototype || !attr->isUndefined()) {
            break;
          }
        }

        return attr;
      },
      object,
      key,
      skipPrototype);

  return r;
}

EvalResult ObjectRefHelper::getProperty(ContextRef* context,
                                        ObjectRef* object,
                                        ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* esState,
         ObjectRef* object,
         ValueRef* key) -> ValueRef* { return object->get(esState, key); },
      object,
      key);
}

EvalResult ObjectRefHelper::getOwnProperty(ContextRef* context,
                                           ObjectRef* object,
                                           ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* esState, ObjectRef* object, ValueRef* key)
          -> ValueRef* { return object->getOwnProperty(esState, key); },
      object,
      key);
}

EvalResult ObjectRefHelper::hasProperty(ContextRef* context,
                                        ObjectRef* object,
                                        ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ObjectRef* object, ValueRef* key)
          -> ValueRef* { return ValueRef::create(object->has(state, key)); },
      object,
      key);
}

EvalResult ObjectRefHelper::hasOwnProperty(ContextRef* context,
                                           ObjectRef* object,
                                           ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* key) -> ValueRef* {
        return ValueRef::create(object->hasOwnProperty(state, key));
      },
      object,
      key);
}

EvalResult ObjectRefHelper::deleteProperty(ContextRef* context,
                                           ObjectRef* object,
                                           ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* key) -> ValueRef* {
        return ValueRef::create(object->deleteProperty(state, key));
      },
      object,
      key);
}

EvalResult ObjectRefHelper::deletePrivateProperty(ContextRef* context,
                                                  SymbolRef* privateValueSymbol,
                                                  ObjectRef* object,
                                                  ValueRef* key) {
  LWNODE_DCHECK_NOT_NULL(object);
  LWNODE_DCHECK_NOT_NULL(key);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         SymbolRef* privateValueSymbol,
         ObjectRef* object,
         ValueRef* key) -> ValueRef* {
        ValueRef* privateValuesObj = object->get(state, privateValueSymbol);

        if (privateValuesObj->isUndefined()) {
          return ValueRef::create(false);
        }

        return ValueRef::create(
            privateValuesObj->asObject()->deleteProperty(state, key));
      },
      privateValueSymbol,
      object,
      key);
}

EvalResult ObjectRefHelper::defineAccessorProperty(
    ContextRef* context,
    ObjectRef* object,
    ValueRef* propertyName,
    const ObjectRef::AccessorPropertyDescriptor& descriptor) {
  LWNODE_DCHECK(propertyName->isSymbol() || propertyName->isString());

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* propertyName,
         ObjectRef::AccessorPropertyDescriptor descriptor) -> ValueRef* {
        return ValueRef::create(
            object->defineAccessorProperty(state, propertyName, descriptor));
      },
      object,
      propertyName,
      descriptor);
}

EvalResult ObjectRefHelper::defineDataProperty(
    ContextRef* context,
    ObjectRef* object,
    ValueRef* propertyName,
    const ObjectRef::DataPropertyDescriptor& descriptor) {
  LWNODE_DCHECK(propertyName->isSymbol() || propertyName->isString());

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* propertyName,
         ObjectRef::DataPropertyDescriptor descriptor) -> ValueRef* {
        return ValueRef::create(
            object->defineDataProperty(state, propertyName, descriptor));
      },
      object,
      propertyName,
      descriptor);
}

ObjectRef* ObjectRefHelper::getPrototype(ContextRef* context,
                                         ObjectRef* object) {
  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ObjectRef* object) -> ValueRef* {
        return object->getPrototype(state);
      },
      object);

  LWNODE_CHECK(r.isSuccessful());
  return r.result->asObject();
}

EvalResult ObjectRefHelper::setPrototype(ContextRef* context,
                                         ObjectRef* object,
                                         ValueRef* proto) {
  auto r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* proto) -> ValueRef* {
        return ValueRef::create(object->setPrototype(state, proto));
      },
      object,
      proto);

  LWNODE_CHECK(r.isSuccessful());
  return r;
}

EvalResult ObjectRefHelper::getPrivate(ContextRef* context,
                                       SymbolRef* privateValueSymbol,
                                       ObjectRef* object,
                                       ValueRef* key) {
  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         SymbolRef* privateValueSymbol,
         ObjectRef* object,
         ValueRef* key) -> ValueRef* {
        ValueRef* hiddenValuesRef =
            object->getOwnProperty(state, privateValueSymbol);

        if (hiddenValuesRef->isUndefined()) {
          return ValueRef::createUndefined();
        }

        return ValueRef::create(hiddenValuesRef->asObject()->get(state, key));
      },
      privateValueSymbol,
      object,
      key);
}

EvalResult ObjectRefHelper::setPrivate(ContextRef* context,
                                       SymbolRef* privateValueSymbol,
                                       ObjectRef* object,
                                       ValueRef* key,
                                       ValueRef* value) {
  LWNODE_CHECK(key->isSymbol());

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ContextRef* context,
         SymbolRef* privateValueSymbol,
         ObjectRef* object,
         ValueRef* param1,
         ValueRef* param2) -> ValueRef* {
        ValueRef* hiddenValuesRef = object->get(state, privateValueSymbol);

        ObjectRef* hiddenValuesObject = nullptr;

        if (hiddenValuesRef->isUndefined()) {
          // 'PRIVATE_SYMBOL_KEY' doesn't exist. create it.
          hiddenValuesObject = ObjectRef::create(state);

          auto result =
              defineDataProperty(
                  context,
                  object,
                  privateValueSymbol,
                  ObjectRef::DataPropertyDescriptor(
                      hiddenValuesObject,
                      static_cast<ObjectRef::PresentAttribute>(
                          ObjectRef::PresentAttribute::WritablePresent |
                          ObjectRef::PresentAttribute::NonEnumerablePresent |
                          ObjectRef::PresentAttribute::NonConfigurablePresent)))
                  .isSuccessful();

          LWNODE_DCHECK(result);
        } else {
          hiddenValuesObject = hiddenValuesRef->asObject();
        }

        hiddenValuesObject->set(state, param1, param2);

        return ValueRef::create(true);
      },
      context,
      privateValueSymbol,
      object,
      key,
      value);
}

bool ObjectRefHelper::hasExtraData(ObjectRef* object) {
  if (object->extraData()) {
    return true;
  }
  return false;
}

void ObjectRefHelper::setExtraData(ObjectRef* object,
                                   ObjectData* data,
                                   bool isForceReplace) {
  if (isForceReplace == false && object->extraData()) {
    LWNODE_DLOG_WARN("extra data already exists. it will be removed.");
  }

  object->setExtraData(data);
}

ObjectData* ObjectRefHelper::getExtraData(ObjectRef* object) {
  void* data = object->extraData();
  return reinterpret_cast<ObjectData*>(data);
}

ObjectRef* ObjectRefHelper::toObject(ContextRef* context, ValueRef* value) {
  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ValueRef* value) -> ValueRef* {
        return value->toObject(state);
      },
      value);

  LWNODE_CHECK(r.isSuccessful());
  return r.result->asObject();
}

StringRef* ObjectRefHelper::getConstructorName(ContextRef* context,
                                               ObjectRef* object) {
  /*
    TODO: Use Escargot API
    This function is buggy in some cases.
    Please check THREADED_TEST(ObjectGetConstructorName)
  */
  auto r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ObjectRef* object) -> ValueRef* {
        OptionalRef<ObjectRef> maybeProto = object->getPrototypeObject(state);
        StringRef* name = StringRef::emptyString();

        if (maybeProto.hasValue()) {
          auto constructor =
              maybeProto.value()
                  ->get(state, StringRef::createFromASCII("constructor"))
                  ->asObject();
          name = constructor->get(state, StringRef::createFromASCII("name"))
                     ->asString();

        } else {
          LWNODE_LOG_WARN("TODO: ConstructorName isn't found.");
        }
        return name;
      },
      object);

  LWNODE_CHECK(r.isSuccessful());

  return r.result->asString();
}

static std::string getCodeLine(const std::string& codeString, int errorLine) {
  if (errorLine < 1 || codeString.empty()) {
    return "";
  }

  std::stringstream sstream(codeString);
  std::string result;
  int curLine = 1;

  for (std::string line; std::getline(sstream, line); ++curLine) {
    if (curLine == errorLine) {
      result = line;
      break;
    }
  }
  return result;
}

std::string EvalResultHelper::getCallStackString(
    const GCManagedVector<Evaluator::StackTraceData>& traceData,
    size_t maxStackSize) {
  std::ostringstream oss;
  const std::string separator = "  ";
  size_t maxPrintStackSize = std::min((int)maxStackSize, (int)traceData.size());

  oss << "Call Stack:" << std::endl;
  for (size_t i = 0; i < maxPrintStackSize; ++i) {
    const auto& iter = traceData[i];
    const auto& resourceName = iter.src->toStdUTF8String();
    const auto& codeString = iter.sourceCode->toStdUTF8String();
    const int errorLine = iter.loc.line;
    const int errorColumn = iter.loc.column;

    auto sourceOnStack = getCodeLine(codeString, errorLine);

    // Trim left spaces
    auto pos = sourceOnStack.find_first_not_of(' ');
    auto errorCodeLine =
        sourceOnStack.substr(pos != std::string::npos ? pos : 0);

    oss << separator << i << ": " << (errorCodeLine == "" ? "?" : errorCodeLine)
        << " ";
    oss << "(" << (resourceName == "" ? "?" : resourceName) << ":" << errorLine
        << ":" << errorColumn << ")" << std::endl;
  }

  return oss.str();
}

std::string EvalResultHelper::getCallStackStringAsNodeStyle(
    const GCManagedVector<Evaluator::StackTraceData>& traceData,
    size_t maxStackSize) {
  std::ostringstream oss;
  const std::string separator = "    ";
  size_t maxPrintStackSize = std::min((int)maxStackSize, (int)traceData.size());

  for (size_t i = 0; i < maxPrintStackSize; ++i) {
    const auto& iter = traceData[i];
    const auto& resourceName = iter.src->toStdUTF8String();
    const auto& functionName = iter.functionName->toStdUTF8String();
    const int errorLine = iter.loc.line;
    const int errorColumn = iter.loc.column;

    oss << separator << "at "
        << (functionName == "" ? "Object.<anonymous>" : functionName) << " "
        << "(" << (resourceName == "" ? "?" : resourceName) << ":" << errorLine
        << ":" << errorColumn << ")" << std::endl;
  }

  return oss.str();
}

std::string EvalResultHelper::getErrorString(
    ContextRef* context, const Evaluator::EvaluatorResult& result) {
  const auto& traceData = result.stackTraceData;
  const auto& reasonString =
      result.resultOrErrorToString(context)->toStdUTF8String();
  const std::string separator = "  ";

  std::ostringstream oss;

  if (traceData.size()) {
    const auto& lastTraceData = traceData[0];
    const auto& resourceName = lastTraceData.src->toStdUTF8String();
    const auto& codeString = lastTraceData.sourceCode->toStdUTF8String();
    const int errorLine = lastTraceData.loc.line;
    const int errorColumn = lastTraceData.loc.column;
    const int marginLine = 5;

    oss << "Name: " << std::endl;
    oss << separator << (resourceName == "" ? "(empty name)" : resourceName)
        << std::endl;
    oss << "Reason: " << std::endl;
    oss << separator << "(" << errorLine << ":" << errorColumn << ") "
        << reasonString << std::endl;

    oss << "Source: " << std::endl;
    std::stringstream sstream(codeString);
    int startLine = std::max(0, errorLine - marginLine);
    int endLine = errorLine + marginLine;
    int curLine = 1;
    for (std::string line; std::getline(sstream, line); ++curLine) {
      if (startLine <= curLine) {
        oss << separator << "L" << curLine << ": " << line;
        if (curLine == errorLine) {
          oss << "\t<--";
        }
        oss << std::endl;

        if (endLine <= curLine) {
          break;
        }
      }
    }

    oss << getCallStackString(traceData);
  }

  return oss.str();
}

Evaluator::EvaluatorResult EvalResultHelper::compileRun(ContextRef* context,
                                                        const char* source,
                                                        bool isModule) {
  auto compileResult = context->scriptParser()->initializeScript(
      StringRef::createFromUTF8(source, strLength(source)),
      StringRef::emptyString(),
      isModule);

  if (!compileResult.isSuccessful()) {
    Evaluator::EvaluatorResult result;
    result.error = ExceptionHelper::createErrorObject(
        context, compileResult.parseErrorCode, compileResult.parseErrorMessage);

    LWNODE_LOG_ERROR(
        "Compile: %s",
        result.resultOrErrorToString(context)->toStdUTF8String().c_str());
    return result;
  }

  auto r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      compileResult.script.get());

  if (r.isSuccessful() == false) {
    LWNODE_DLOG_RAW("Execute:\n  %s (%s:%d)\n%s",
                    TRACE_ARGS2,
                    EvalResultHelper::getErrorString(context, r).c_str());
  }
  return r;
}

void EvalResultHelper::attachBuiltinPrint(ContextRef* context,
                                          ObjectRef* target) {
  static auto builtinPrint = [](ExecutionStateRef* state,
                                ValueRef* thisValue,
                                size_t argc,
                                ValueRef** argv,
                                bool isConstructCall) -> ValueRef* {
    if (argc > 0) {
      std::stringstream ss;

      for (size_t i = 0; i < argc; ++i) {
        if (argv[i]->isSymbol()) {
          ss << argv[i]
                    ->asSymbol()
                    ->symbolDescriptiveString()
                    ->toStdUTF8String();
        } else {
          ss << argv[i]
                    ->toStringWithoutException(state->context())
                    ->toStdUTF8String();
        }
        ss << " ";
      }

      LWNODE_DLOG_RAW("%s", ss.str().c_str());
    }
    return ValueRef::createUndefined();
  };

  static auto builtinPrintAddress = [](ExecutionStateRef* state,
                                       ValueRef* thisValue,
                                       size_t argc,
                                       ValueRef** argv,
                                       bool isConstructCall) -> ValueRef* {
    if (argc > 0) {
      std::stringstream ss;

      for (size_t i = 0; i < argc; ++i) {
        if (argv[i]->isString()) {
          ss << argv[i]
                    ->toStringWithoutException(state->context())
                    ->toStdUTF8String();
        } else {
          ss << CLR_DIM << "(" << argv[i] << ")" << CLR_RESET;
        }
        ss << " ";
      }

      LWNODE_DLOG_RAW("%s", ss.str().c_str());
    }
    return ValueRef::createUndefined();
  };

  static auto builtinPrintCallStack = [](ExecutionStateRef* state,
                                         ValueRef* thisValue,
                                         size_t argc,
                                         ValueRef** argv,
                                         bool isConstructCall) -> ValueRef* {
    size_t maxStackSize = 5;

    if (argc == 1 && argv[0]->isUInt32()) {
      maxStackSize = argv[0]->toUint32(state);
    }

    LWNODE_DLOG_RAW(
        "%s",
        getCallStackString(state->computeStackTraceData(), maxStackSize)
            .c_str());

    return ValueRef::createUndefined();
  };

  Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ObjectRef* target) -> ValueRef* {
        auto esPrint =
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrint,
                                          1,
                                          true,
                                          false));

        esPrint->defineDataProperty(
            state,
            StringRef::createFromASCII("ptr"),
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrintAddress,
                                          1,
                                          true,
                                          false)),
            true,
            true,
            true);

        esPrint->defineDataProperty(
            state,
            StringRef::createFromASCII("stack"),
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrintCallStack,
                                          1,
                                          true,
                                          false)),
            true,
            true,
            true);

        target->defineDataProperty(state,
                                   StringRef::createFromASCII("print"),
                                   esPrint,
                                   true,
                                   true,
                                   true);

        return ValueRef::createUndefined();
      },
      target);
}

ObjectData* ObjectRefHelper::createExtraDataIfNotExist(ObjectRef* object) {
  void* data = object->extraData();
  if (data == nullptr) {
    data = new ObjectData();
    ObjectRefHelper::setExtraData(object, reinterpret_cast<ObjectData*>(data));
  }
  return reinterpret_cast<ObjectData*>(data);
}

void ObjectRefHelper::setInternalFieldCount(ObjectRef* object, int size) {
  auto data = createExtraDataIfNotExist(object);
  LWNODE_DCHECK_NOT_NULL(data);

  data->setInternalFieldCount(size);
}

int ObjectRefHelper::getInternalFieldCount(ObjectRef* object) {
  auto data = getExtraData(object);
  if (data == nullptr) {
    return 0;
  }
  return data->internalFieldCount();
}

void ObjectRefHelper::setInternalField(ObjectRef* object,
                                       int idx,
                                       InternalField* lwValue) {
  auto data = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(data);

  data->setInternalField(idx, lwValue);
}

InternalField* ObjectRefHelper::getInternalField(ObjectRef* object, int idx) {
  auto data = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(data);

  auto field = reinterpret_cast<InternalField*>(data->internalField(idx));
  if (!field) {
    return IsolateWrap::GetCurrent()->undefined_value();
  }

  return field;
}

void ObjectRefHelper::setInternalPointer(ObjectRef* object,
                                         int idx,
                                         void* ptr) {
  auto data = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(data);

  data->setInternalField(idx, ptr);
}

void* ObjectRefHelper::getInternalPointer(ObjectRef* object, int idx) {
  auto extraData = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(extraData);

  auto data = extraData->internalField(idx);

  return data;
}

// --- ObjectTemplateRefHelper ---

void ObjectTemplateRefHelper::setInstanceExtraData(ObjectTemplateRef* otpl,
                                                   ObjectData* data) {
  LWNODE_CHECK_NOT_NULL(data);
  LWNODE_CHECK_NULL(otpl->instanceExtraData());

  otpl->setInstanceExtraData(data);
}

ObjectData* ObjectTemplateRefHelper::getInstanceExtraData(
    ObjectTemplateRef* otpl) {
  auto data = otpl->instanceExtraData();
  return reinterpret_cast<ObjectData*>(data);
}

ObjectData* ObjectTemplateRefHelper::createInstanceExtraDataIfNotExist(
    ObjectTemplateRef* otpl) {
  void* data = otpl->instanceExtraData();
  if (data == nullptr) {
    data = new ObjectData();
    ObjectTemplateRefHelper::setInstanceExtraData(
        otpl, reinterpret_cast<ObjectData*>(data));
  }
  return reinterpret_cast<ObjectData*>(data);
}

void ObjectTemplateRefHelper::setInternalFieldCount(ObjectTemplateRef* otpl,
                                                    int size) {
  auto data = createInstanceExtraDataIfNotExist(otpl);
  LWNODE_DCHECK_NOT_NULL(data);

  data->setInternalFieldCount(size);
}

int ObjectTemplateRefHelper::getInternalFieldCount(ObjectTemplateRef* otpl) {
  auto data = getInstanceExtraData(otpl);
  if (data == nullptr) {
    return 0;
  }
  return data->internalFieldCount();
}

// --- FunctionTemplateRefHelper ---

void FunctionTemplateRefHelper::setInstanceExtraData(FunctionTemplateRef* ftpl,
                                                     FunctionData* data) {
  LWNODE_CALL_TRACE_ID(OBJDATA, "es: %p data: %p", ftpl, data);
  LWNODE_CHECK_NOT_NULL(data);
  LWNODE_CHECK_NULL(ftpl->instanceExtraData());

  ftpl->setInstanceExtraData(data);
}

FunctionData* FunctionTemplateRefHelper::getInstanceExtraData(
    FunctionTemplateRef* ftpl) {
  auto data = ftpl->instanceExtraData();
  LWNODE_CALL_TRACE_ID(OBJDATA, "es: %p data: %p", ftpl, data);
  return reinterpret_cast<FunctionData*>(data);
}

// --- ExceptionHelper ---

ValueWrap* ExceptionHelper::wrapException(ValueRef* exception) {
  return ValueWrap::createValue(exception);
}

ValueRef* ExceptionHelper::unwrapException(void* exception) {
  return reinterpret_cast<ValueWrap*>(exception)->value();
}

ErrorObjectRef* ExceptionHelper::createErrorObject(ContextRef* context,
                                                   ErrorObjectRef::Code code,
                                                   StringRef* errorMessage) {
  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ErrorObjectRef::Code code,
         StringRef* errorMessage) -> ValueRef* {
        auto errorObject = ErrorObjectRef::create(state, code, errorMessage);
        ExceptionHelper::setStackPropertyIfNotExist(state, errorObject);
        return errorObject;
      },
      code,
      errorMessage);

  LWNODE_CHECK(r.isSuccessful());
  return r.result->asErrorObject();
}

void ExceptionHelper::setStackPropertyIfNotExist(ExecutionStateRef* state,
                                                 Escargot::ValueRef* error) {
  if (!error->isObject()) {
    return;
  }

  auto errorObject = error->asObject();
  auto stackString = StringRef::createFromASCII("stack");
  if (errorObject->has(state, stackString)) {
    return;
  }

  auto stack = EvalResultHelper::getCallStackStringAsNodeStyle(
      state->computeStackTraceData(), 1);
  auto message = errorObject->toString(state)->toStdUTF8String();

  if (message.length() > 0) {
    stack = message + "\n" + stack;
  }

  errorObject->set(state,
                   stackString,
                   StringRef::createFromASCII(stack.data(), stack.length()));
}

// --- StringRefHelper ---

bool StringRefHelper::isAsciiString(StringRef* str) {
  auto bufferData = str->stringBufferAccessData();

  if (!bufferData.has8BitContent) {
    return false;
  }

  bool isAscii = true;
  for (size_t i = 0; i < bufferData.length; i++) {
    char16_t c = bufferData.charAt(i);
    if (c > 127) {  // including all 7 bit code
      isAscii = false;
      break;
    }
  }

  return isAscii;
}

}  // namespace EscargotShim
