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
#include "isolate.h"
#include "utils/misc.h"

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

EvalResult ObjectRefHelper::defineDataProperty(
    ContextRef* context,
    ObjectRef* object,
    ValueRef* propertyName,
    bool isWritable,
    bool isEnumerable,
    bool isConfigurable,
    ValueRef* propertyValue,
    NativeFunctionPointer functionPropertyValue,
    ValueRef* getter,
    ValueRef* setter) {
  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* propertyName,
         bool isWritable,
         bool isEnumerable,
         bool isConfigurable,
         ValueRef* propertyValue,
         NativeFunctionPointer functionPropertyValue,
         ValueRef* getter,
         ValueRef* setter) -> ValueRef* {
        bool result = false;

        // check if accessors (getter or setter) are given
        if (getter != nullptr || setter != nullptr) {
          ObjectRef::PresentAttribute attr = (ObjectRef::PresentAttribute)0;

          if (isEnumerable) {
            attr = (ObjectRef::PresentAttribute)(
                attr | ObjectRef::PresentAttribute::EnumerablePresent);
          }
          if (isConfigurable) {
            attr = (ObjectRef::PresentAttribute)(
                attr | ObjectRef::PresentAttribute::ConfigurablePresent);
          }
          if (isWritable) {
            attr = (ObjectRef::PresentAttribute)(
                attr | ObjectRef::PresentAttribute::WritablePresent);
          }

          result = object->defineAccessorProperty(
              state,
              propertyName,
              ObjectRef::AccessorPropertyDescriptor(getter, setter, attr));

        } else {
          // else: check if a value or a native function to get a value is given
          ValueRef *key = propertyName, *value = propertyValue;
          if (value == nullptr) {
            LWNODE_DCHECK_NOT_NULL(functionPropertyValue);

            auto atomicstr = AtomicStringRef::create(state->context(),
                                                     StringRef::emptyString());

            value = FunctionObjectRef::createBuiltinFunction(
                state,
                FunctionObjectRef::NativeFunctionInfo(
                    atomicstr, functionPropertyValue, 0, false, false));
          }

          result = object->defineDataProperty(
              state, key, value, isWritable, isEnumerable, isConfigurable);
        }
        return ValueRef::create(result);
      },
      object,
      propertyName,
      isWritable,
      isEnumerable,
      isConfigurable,
      propertyValue,
      functionPropertyValue,
      getter,
      setter);
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
  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* param1) -> ValueRef* {
        return ValueRef::create(object->setPrototype(state, param1));
      },
      object,
      proto);
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

          LWNODE_DCHECK(defineDataProperty(context,
                                           object,
                                           s_symbolKeyForHiddenValues,
                                           false,
                                           false,
                                           false,
                                           hiddenValuesObject,
                                           nullptr,
                                           nullptr,
                                           nullptr)
                            .isSuccessful());
        } else {
          hiddenValuesObject = hiddenValuesRef->asObject();
        }

        LWNODE_DCHECK(hiddenValuesObject->set(state, param1, param2));

        return ValueRef::create(true);
      },
      context,
      object,
      key,
      value);
}

void ObjectRefHelper::setExtraData(
    ObjectRef* object,
    void* data,
    Memory::GCAllocatedMemoryFinalizer callback) {
  if (object->extraData()) {
    LWNODE_DLOG_WARN("extra data already exists. it will be removed.");
  }

  // @consider manage extra data with slot id
  object->setExtraData(data);

  if (callback) {
    MemoryUtil::gcRegisterFinalizer(object, callback);
  }
}

}  // namespace EscargotShim
