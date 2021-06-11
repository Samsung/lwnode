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
#include "utils/string.h"

#include <sstream>

using namespace Escargot;

namespace EscargotShim {

#define PRIVATE_SYMBOL_KEY "__hiddenvalues__"
SymbolRef* ObjectRefHelper::s_symbolKeyForHiddenValues = nullptr;

ObjectRef* ObjectRefHelper::create(ContextRef* context) {
  EvalResult r =
      Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
        return ObjectRef::create(state);
      });

  LWNODE_CHECK(r.isSuccessful());

  if (s_symbolKeyForHiddenValues == nullptr) {
    s_symbolKeyForHiddenValues =
        SymbolRef::create(StringRef::createFromASCII(PRIVATE_SYMBOL_KEY));
  }

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
         ValueRef* param1) -> ValueRef* {
        return ValueRef::create(object->setPrototype(state, param1));
      },
      object,
      proto);

  LWNODE_CHECK(r.isSuccessful());
  return r;
}

EvalResult ObjectRefHelper::getPrivate(ContextRef* context,
                                       ObjectRef* object,
                                       ValueRef* key) {
  LWNODE_CHECK_NOT_NULL(s_symbolKeyForHiddenValues);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* param1) -> ValueRef* {
        ValueRef* hiddenValuesRef =
            object->get(state, s_symbolKeyForHiddenValues);

        if (hiddenValuesRef->isUndefined()) {
          return ValueRef::createUndefined();
        }

        ObjectRef* hiddenValuesObject = hiddenValuesRef->asObject();

        return ValueRef::create(hiddenValuesObject->get(state, param1));
      },
      object,
      key);
}

EvalResult ObjectRefHelper::setPrivate(ContextRef* context,
                                       ObjectRef* object,
                                       ValueRef* key,
                                       ValueRef* value) {
  LWNODE_CHECK_NOT_NULL(s_symbolKeyForHiddenValues);
  LWNODE_CHECK(key->isSymbol());

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ContextRef* context,
         ObjectRef* object,
         ValueRef* param1,
         ValueRef* param2) -> ValueRef* {
        ValueRef* hiddenValuesRef =
            object->get(state, s_symbolKeyForHiddenValues);

        ObjectRef* hiddenValuesObject = nullptr;

        if (hiddenValuesRef->isUndefined()) {
          // 'PRIVATE_SYMBOL_KEY' doesn't exist. create it.
          hiddenValuesObject = ObjectRef::create(state);

          auto result =
              defineDataProperty(
                  context,
                  object,
                  s_symbolKeyForHiddenValues,
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

void ObjectRefHelper::setExtraData(
    ObjectRef* object,
    ObjectData* data,
    Memory::GCAllocatedMemoryFinalizer callback) {
  if (object->extraData()) {
    LWNODE_DLOG_WARN("extra data already exists. it will be removed.");
  }

  object->setExtraData(data);

  if (callback) {
    MemoryUtil::gcRegisterFinalizer(object, callback);
  }
}

ObjectData* ObjectRefHelper::getExtraData(ObjectRef* object) {
  void* data = object->extraData();
  return reinterpret_cast<ObjectData*>(data);
}

std::string getCodeLine(const std::string& codeString, int errorLine) {
  LWNODE_CHECK(errorLine >= 1);

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

    size_t maxPrintStackSize = std::min(5, (int)traceData.size());

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

      oss << separator << i << ": " << errorCodeLine << " ";
      oss << "(" << (resourceName == "" ? "?" : resourceName) << ":"
          << errorLine << ":" << errorColumn << ")" << std::endl;
    }
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
        "%s", result.resultOrErrorToString(context)->toStdUTF8String().c_str());
    return result;
  }

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      compileResult.script.get());
}

void EvalResultHelper::attachBuiltinPrint(ContextRef* context) {
  static auto builtinPrint = [](ExecutionStateRef* state,
                                ValueRef* thisValue,
                                size_t argc,
                                ValueRef** argv,
                                bool isConstructCall) -> ValueRef* {
    if (argc > 0) {
      if (argv[0]->isSymbol()) {
        puts(argv[0]
                 ->asSymbol()
                 ->symbolDescriptiveString()
                 ->toStdUTF8String()
                 .c_str());
      } else {
        puts(argv[0]->toString(state)->toStdUTF8String().c_str());
      }
    } else {
      puts("undefined");
    }
    return ValueRef::createUndefined();
  };

  Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
    ContextRef* context = state->context();

    FunctionObjectRef::NativeFunctionInfo info(
        AtomicStringRef::create(context, "print"),
        builtinPrint,
        1,
        true,
        false);

    context->globalObject()->defineDataProperty(
        state,
        StringRef::createFromASCII("print"),
        FunctionObjectRef::create(state, info),
        true,
        true,
        true);

    return ValueRef::createUndefined();
  });
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

  return reinterpret_cast<InternalField*>(data->internalField(idx));
}

void ObjectRefHelper::setInternalPointer(ObjectRef* object,
                                         int idx,
                                         void* ptr) {
  auto data = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(data);

  data->setInternalField(idx, ptr);
}

void* ObjectRefHelper::getInternalPointer(ObjectRef* object, int idx) {
  auto data = getExtraData(object);
  LWNODE_CHECK_NOT_NULL(data);

  return data->internalField(idx);
}

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

void FunctionTemplateRefHelper::setInstanceExtraData(FunctionTemplateRef* ftpl,
                                                     FunctionData* data) {
  LWNODE_CHECK_NOT_NULL(data);
  LWNODE_CHECK_NULL(ftpl->instanceExtraData());

  ftpl->setInstanceExtraData(data);
}

FunctionData* FunctionTemplateRefHelper::getInstanceExtraData(
    FunctionTemplateRef* ftpl) {
  auto data = ftpl->instanceExtraData();
  return reinterpret_cast<FunctionData*>(data);
}

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
        return ErrorObjectRef::create(state, code, errorMessage);
      },
      code,
      errorMessage);

  LWNODE_CHECK(r.isSuccessful());
  return r.result->asErrorObject();
}

}  // namespace EscargotShim
