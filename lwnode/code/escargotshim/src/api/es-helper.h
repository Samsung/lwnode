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

#pragma once

#include <EscargotPublic.h>
#include "extra-data.h"
#include "utils/gc-util.h"

using namespace Escargot;

namespace EscargotShim {

class IsolateWrap;
class ContextWrap;
class ObjectData;
class FunctionData;
class ValueWrap;
enum class ErrorMessageType;

struct EvalResult : public Evaluator::EvaluatorResult {
  EvalResult();
  EvalResult(const EvalResult& src);
  EvalResult(EvalResult&& src);

  // Evaluator::EvaluatorResult
  EvalResult(const Evaluator::EvaluatorResult& src)
      : Evaluator::EvaluatorResult(src) {}
  EvalResult(Evaluator::EvaluatorResult&& src)
      : Evaluator::EvaluatorResult(std::move(src)) {}

  EvalResult& check() {
    LWNODE_CHECK(isSuccessful());
    return *this;
  }
};

typedef FunctionObjectRef::NativeFunctionPointer NativeFunctionPointer;
typedef FunctionObjectRef::NativeFunctionInfo NativeFunctionInfo;
typedef ValueWrap InternalField;

class ObjectRefHelper {
 public:
  static ObjectRef* create(ContextRef* context);

  static ObjectRef* getPrototype(ContextRef* context, ObjectRef* object);
  static EvalResult setPrototype(ContextRef* context,
                                 ObjectRef* object,
                                 ValueRef* proto);

  static EvalResult getProperty(ContextRef* context,
                                ObjectRef* object,
                                ValueRef* key);
  static EvalResult setProperty(ContextRef* context,
                                ObjectRef* object,
                                ValueRef* key,
                                ValueRef* value);

  static EvalResult getOwnProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

  static EvalResult getPropertyAttributes(
      ContextRef* context,
      ObjectRef* object,
      ValueRef* key,
      bool skipTraversingPrototypeChain = false);

  static EvalResult hasProperty(ContextRef* context,
                                ObjectRef* object,
                                ValueRef* key);

  static EvalResult hasOwnProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

  static EvalResult getOwnIndexedProperty(ContextRef* context,
                                          ObjectRef* object,
                                          uint32_t index);

  static EvalResult getPrivate(ContextRef* context,
                               SymbolRef* privateValueSymbol,
                               ObjectRef* object,
                               ValueRef* key);
  static EvalResult setPrivate(ContextRef* context,
                               SymbolRef* privateValueSymbol,
                               ObjectRef* object,
                               ValueRef* key,
                               ValueRef* value);

  static EvalResult deleteProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

  static EvalResult deletePrivateProperty(ContextRef* context,
                                          SymbolRef* privateValueSymbol,
                                          ObjectRef* object,
                                          ValueRef* key);

  static EvalResult defineDataProperty(
      ContextRef* context,
      ObjectRef* object,
      ValueRef* propertyName,
      const ObjectRef::DataPropertyDescriptor& descriptor);

  static EvalResult defineAccessorProperty(
      ContextRef* context,
      ObjectRef* object,
      ValueRef* propertyName,
      const ObjectRef::AccessorPropertyDescriptor& descriptor);

  static void setExtraData(ObjectRef* object,
                           ObjectData* data,
                           bool isForceReplace = false);
  static bool hasExtraData(ObjectRef* object);
  static ObjectData* getExtraData(ObjectRef* object);

  static int getInternalFieldCount(ObjectRef* object);
  static void setInternalField(ObjectRef* object,
                               int idx,
                               InternalField* lwValue);
  static InternalField* getInternalField(ObjectRef* object, int idx);

  static void setInternalPointer(ObjectRef* object, int idx, void* ptr);
  static void* getInternalPointer(ObjectRef* object, int idx);

  static ObjectRef* toObject(ContextRef* context, ValueRef* value);
  static StringRef* getConstructorName(ContextRef* context, ObjectRef* object);

  static void addNativeFunction(ContextRef* context,
                                ObjectRef* object,
                                StringRef* name,
                                NativeFunctionPointer function);

 private:
  static ValueRef* getOwnPropertyAttributes(ExecutionStateRef* state,
                                            ObjectRef* object,
                                            ValueRef* key);
};

class ArrayObjectRefHelper {
 public:
  static ArrayObjectRef* create(ContextRef* context, const uint64_t length);
  static ArrayObjectRef* create(ContextRef* context, ValueVectorRef* elements);
  static uint64_t length(ContextRef* context, ArrayObjectRef* object);
  static ValueRef* get(ContextRef* context,
                       ArrayObjectRef* object,
                       ValueRef::ValueIndex index);
  static void set(ContextRef* context,
                  ArrayObjectRef* object,
                  ValueRef::ValueIndex index,
                  ValueRef* value);
};

class ObjectTemplateData;
class FunctionTemplateData;
class FunctionData;
class ExtraDataHelper {
 public:
  static ExtraData* getExtraData(ObjectRef* object) {
    return (ExtraData*)object->extraData();
  }

  static FunctionData* getFunctionExtraData(ObjectRef* functionObject) {
    auto extraData = (ExtraData*)functionObject->extraData();
    if (extraData) {
      return extraData->asFunctionData();
    }

    return nullptr;
  }

  static ObjectTemplateData* getObjectTemplateExtraData(
      ObjectRef* objectTemplate) {
    auto extraData = (ExtraData*)objectTemplate->extraData();
    if (extraData) {
      return extraData->asObjectTemplateData();
    }

    return nullptr;
  }

  static ObjectTemplateData* getObjectTemplateExtraData(
      ObjectTemplateRef* objectTemplate) {
    auto extraData = (ExtraData*)objectTemplate->instanceExtraData();
    if (extraData) {
      return extraData->asObjectTemplateData();
    }

    return nullptr;
  }

  static FunctionTemplateData* getFunctionTemplateExtraData(
      FunctionTemplateRef* functionTemplate) {
    auto extraData = (ExtraData*)functionTemplate->instanceExtraData();
    if (extraData) {
      return extraData->asFunctionTemplateData();
    }

    return nullptr;
  }

  static ExceptionObjectData* getExceptionObjectExtraData(
      ObjectRef* functionObject) {
    auto extraData = (ExtraData*)functionObject->extraData();
    if (extraData) {
      return extraData->asExceptionObjectData();
    }

    return nullptr;
  }

  // Only allow the following (Object, extraData) pairs when adding extraData
  static void setExtraData(ObjectTemplateRef* otpl, ObjectTemplateData* data);
  static void setExtraData(FunctionTemplateRef* ftpl,
                           FunctionTemplateData* data);
  static void setExtraData(FunctionObjectRef* functionObject,
                           FunctionData* data,
                           bool force = false);
  static void setExtraData(ObjectRef* exceptionObject,
                           ExceptionObjectData* data);
  static void setExtraData(ObjectRef* callSite, StackTraceData* data);
  static void setExtraData(ObjectRef* object, ObjectData* data);
};

class ObjectTemplateRefHelper {
 public:
  static void setInternalFieldCount(ObjectTemplateRef* otpl, int size);
  static int getInternalFieldCount(ObjectTemplateRef* otpl);

 private:
};

class ArrayBufferHelper {
 public:
  enum ArrayType {
    kExternalInt8Array = 1,
    kExternalUint8Array,
    kExternalInt16Array,
    kExternalUint16Array,
    kExternalInt32Array,
    kExternalUint32Array,
    kExternalFloat32Array,
    kExternalFloat64Array,
    kExternalUint8ClampedArray,
    kExternalBigInt64Array,
    kExternalBigUint64Array,
  };

  template <class T>
  static ArrayBufferViewRef* createView(ContextRef* context,
                                        ArrayBufferObjectRef* abo,
                                        size_t byteOffset,
                                        size_t arrayLength,
                                        ArrayType type) {
    EvalResult r = Evaluator::execute(
        context,
        [](ExecutionStateRef* state) -> ValueRef* { return T::create(state); });

    LWNODE_CHECK(r.isSuccessful());

    size_t byteSize = 0;

    switch (type) {
      case kExternalInt8Array:
      case kExternalUint8Array:
      case kExternalUint8ClampedArray:
        byteSize = 1;
        break;
      case kExternalInt16Array:
      case kExternalUint16Array:
        byteSize = 2;
        break;
      case kExternalInt32Array:
      case kExternalUint32Array:
      case kExternalFloat32Array:
        byteSize = 4;
        break;
      case kExternalFloat64Array:
      case kExternalBigInt64Array:
      case kExternalBigUint64Array:
        byteSize = 8;
        break;
      default:
        break;
    }

    LWNODE_CHECK(byteSize != 0);

    auto arrayBufferView = r.result->asArrayBufferView();

    arrayBufferView->setBuffer(
        abo, byteOffset, byteSize * arrayLength, arrayLength);

    return arrayBufferView;
  }
};

class EvalResultHelper {
 public:
  static void attachBuiltinPrint(ContextRef* context, ObjectRef* target);
  static Evaluator::EvaluatorResult compileRun(ContextRef* context,
                                               const char* source,
                                               bool isModule = false);
  static std::string getErrorString(
      ContextRef* context, const Evaluator::EvaluatorResult& eval_result);
  static std::string getCallStackString(
      const GCManagedVector<Evaluator::StackTraceData>& traceData,
      size_t maxStackSize = 5);
  static std::string getCallStackStringAsNodeStyle(
      const GCManagedVector<Evaluator::StackTraceData>& traceData,
      size_t maxStackSize = 5);
};

class ExceptionHelper {
 public:
  static ValueWrap* wrapException(ValueRef* exception);
  static ValueRef* unwrapException(void* exception);
  static ErrorObjectRef* createErrorObject(ContextRef* context,
                                           ErrorObjectRef::Code code,
                                           StringRef* errorMessage);
  static ErrorObjectRef* createErrorObject(ContextRef* context,
                                           ErrorMessageType type);
  static void addStackPropertyCallback(ExecutionStateRef* state,
                                       Escargot::ValueRef* error);
};

class StringRefHelper {
 public:
  template <size_t N>
  static bool equalsWithASCIIString(StringRef* esString, const char (&str)[N]) {
    return esString->equalsWithASCIIString(str, N - 1);
  }

  static bool isAsciiString(StringRef* str);
  static bool isOneByteString(StringRef* str);
};

}  // namespace EscargotShim
