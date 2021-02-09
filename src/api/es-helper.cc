/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
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
  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* param1) -> ValueRef* { return object->get(state, param1); },
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

}  // namespace EscargotShim
