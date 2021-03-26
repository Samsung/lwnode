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

typedef Evaluator::EvaluatorResult EvalResult;
typedef FunctionObjectRef::NativeFunctionPointer NativeFunctionPointer;
typedef FunctionObjectRef::NativeFunctionInfo NativeFunctionInfo;

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

  static EvalResult hasProperty(ContextRef* context,
                                ObjectRef* object,
                                ValueRef* key);

  static EvalResult hasOwnProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

  static EvalResult getPrivate(ContextRef* context,
                               ObjectRef* object,
                               ValueRef* key);
  static EvalResult setPrivate(ContextRef* context,
                               ObjectRef* object,
                               ValueRef* key,
                               ValueRef* value);

  static EvalResult deleteProperty(ContextRef* context,
                                   ObjectRef* object,
                                   ValueRef* key);

  static EvalResult defineDataProperty(
      ContextRef* context,
      ObjectRef* object,
      ValueRef* propertyName,
      bool isWritable,
      bool isEnumerable,
      bool isConfigurable,
      ValueRef* propertyValue,
      NativeFunctionPointer functionPropertyValue,
      ValueRef* getter,
      ValueRef* setter);

  static void setExtraData(ObjectRef* object,
                           ObjectData* data,
                           Memory::GCAllocatedMemoryFinalizer callback);

  static ObjectData* getExtraData(ObjectRef* object);

 private:
  static SymbolRef* s_symbolKeyForHiddenValues;
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

}  // namespace EscargotShim
