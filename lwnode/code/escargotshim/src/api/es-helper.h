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
#include "utils/gc.h"

using namespace Escargot;

namespace EscargotShim {

class IsolateWrap;
class ContextWrap;
class ObjectData;
class FunctionData;
class ValueWrap;

typedef Evaluator::EvaluatorResult EvalResult;
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

  static EvalResult getPropertyAttributes(ContextRef* context,
                                          ObjectRef* object,
                                          ValueRef* key,
                                          bool skipPrototype = false);

  static EvalResult hasProperty(ContextRef* context,
                                ObjectRef* object,
                                ValueRef* key);

  static EvalResult hasOwnProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

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

  static void setInternalFieldCount(ObjectRef* object, int size);
  static int getInternalFieldCount(ObjectRef* object);
  static void setInternalField(ObjectRef* object,
                               int idx,
                               InternalField* lwValue);
  static InternalField* getInternalField(ObjectRef* object, int idx);

  static void setInternalPointer(ObjectRef* object, int idx, void* ptr);
  static void* getInternalPointer(ObjectRef* object, int idx);

  static ObjectRef* toObject(ContextRef* context, ValueRef* value);
  static StringRef* getConstructorName(ContextRef* context, ObjectRef* object);

 private:
  static ObjectData* createExtraDataIfNotExist(ObjectRef* object);

  static ValueRef* getOwnPropertyAttributes(ExecutionStateRef* state,
                                            ObjectRef* object,
                                            ValueRef* key);
};

class ObjectTemplateRefHelper {
 public:
  static void setInstanceExtraData(ObjectTemplateRef* otpl, ObjectData* data);
  static ObjectData* getInstanceExtraData(ObjectTemplateRef* otpl);
  static void setInternalFieldCount(ObjectTemplateRef* otpl, int size);
  static int getInternalFieldCount(ObjectTemplateRef* otpl);

 private:
  static ObjectData* createInstanceExtraDataIfNotExist(ObjectTemplateRef* otpl);
};

class FunctionTemplateRefHelper {
 public:
  static void setInstanceExtraData(FunctionTemplateRef* ftpl,
                                   FunctionData* data);
  static FunctionData* getInstanceExtraData(FunctionTemplateRef* ftpl);
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
};

class ExceptionHelper {
 public:
  static ValueWrap* wrapException(ValueRef* exception);
  static ValueRef* unwrapException(void* exception);
  static ErrorObjectRef* createErrorObject(ContextRef* context,
                                           ErrorObjectRef::Code code,
                                           StringRef* errorMessage);
};

class StringRefHelper {
 public:
  template <size_t N>
  static bool equalsWithASCIIString(StringRef* esString, const char (&str)[N]) {
    return esString->equalsWithASCIIString(str, N - 1);
  }

  static bool isAsciiString(StringRef* str);
};

}  // namespace EscargotShim
