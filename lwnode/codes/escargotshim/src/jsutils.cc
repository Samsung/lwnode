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

#include "jsutils.h"
#include "escargotisolateshim.h"
#include "escargotutil.h"

namespace EscargotShim {

// JsObjectWrapper

JsObjectWrapper::JsObjectWrapper(JsContextRef context) {
  m_context = context;
  CreateJsObject(context, m_object);
}

JsValueRef JsObjectWrapper::GetJsValueRef() {
  return m_object;
}

JsKeepAliveWrapper::JsKeepAliveWrapper(JsContextRef context)
    : JsObjectWrapper(context) {}

#define IMPLEMENT_JS_FUNCTION_GETTER(F)                                        \
  JsValueRef JsKeepAliveWrapper::Get##F##Function() {                          \
    JsValueRef fn = JS_INVALID_REFERENCE;                                      \
    auto result = GetProperty(m_context, m_object, CachedStringId::F, fn);     \
    if (result != JsNoError) {                                                 \
      NESCARGOT_ASSERT(false);                                                 \
    }                                                                          \
    NESCARGOT_ASSERT(fn->isFunctionObject());                                  \
    return fn;                                                                 \
  }

#define DEF_IS_TYPE(F) IMPLEMENT_JS_FUNCTION_GETTER(F)
#include "escargotproperyid.inc"
#undef DEF_IS_TYPE

IMPLEMENT_JS_FUNCTION_GETTER(cloneObject);
IMPLEMENT_JS_FUNCTION_GETTER(getEnumerableNamedProperties);
IMPLEMENT_JS_FUNCTION_GETTER(getEnumerableIndexedProperties);
IMPLEMENT_JS_FUNCTION_GETTER(createEnumerationIterator);
IMPLEMENT_JS_FUNCTION_GETTER(createPropertyDescriptor);
IMPLEMENT_JS_FUNCTION_GETTER(createPropertyDescriptorsEnumerationIterator);
IMPLEMENT_JS_FUNCTION_GETTER(getNamedOwnKeys);
IMPLEMENT_JS_FUNCTION_GETTER(getIndexedOwnKeys);
IMPLEMENT_JS_FUNCTION_GETTER(getStackTrace);
IMPLEMENT_JS_FUNCTION_GETTER(getSymbolKeyFor);
IMPLEMENT_JS_FUNCTION_GETTER(getSymbolFor);
IMPLEMENT_JS_FUNCTION_GETTER(ensureDebug);
IMPLEMENT_JS_FUNCTION_GETTER(enqueueMicrotask);
IMPLEMENT_JS_FUNCTION_GETTER(dequeueMicrotask);
IMPLEMENT_JS_FUNCTION_GETTER(getPropertyAttributes);
IMPLEMENT_JS_FUNCTION_GETTER(getOwnPropertyNames);
IMPLEMENT_JS_FUNCTION_GETTER(jsonParse);
IMPLEMENT_JS_FUNCTION_GETTER(jsonStringify);

namespace js {

JsValueRef concatArray(JsContextRef context,
                       JsValueRef first,
                       JsValueRef second) {
  JsValueRef fn = JS_INVALID_REFERENCE;
  if (GetProperty(context,
                  context->globalObject()->arrayPrototype(),
                  CachedStringId::concat,
                  fn) != JsNoError) {
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }

  NESCARGOT_ASSERT(first->asObject()->isArrayObject());
  NESCARGOT_ASSERT(second->asObject()->isArrayObject());

  JsValueRef arguments[] = {second};
  JsValueRef result;

  if (CallJsFunction(
          context, first, fn, arguments, _countof(arguments), &result) !=
      JsNoError) {
    // TODO : change return type to JsErrorCode
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }
  return result;
}

JsValueRef GetStringConcatFunction(JsContextRef context) {
  JsValueRef fn = JS_INVALID_REFERENCE;
  if (GetProperty(context,
                  context->globalObject()->stringPrototype(),
                  CachedStringId::concat,
                  fn) != JsNoError) {
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }
  return fn;
}

// a. Implement javascript bindings that requires a single object as its
// parameter

#define IMPLEMENT_JS_FUNCTION_ARG_1_OBJ(F)                                     \
  JsValueRef F(JsValueRef object) {                                            \
    ContextShim* contextShim = IsolateShim::GetCurrent()->currentContext();    \
    auto keepalive = contextShim->GetKeepAliveWrapper();                       \
    auto fn = keepalive->Get##F##Function();                                   \
    JsValueRef arguments[] = {                                                 \
        object,                                                                \
    };                                                                         \
    JsValueRef result;                                                         \
    if (CallJsFunction(contextShim->contextRef(),                              \
                       keepalive->GetJsValueRef(),                             \
                       fn,                                                     \
                       arguments,                                              \
                       _countof(arguments),                                    \
                       &result) != JsNoError) {                                \
      NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();                                   \
    }                                                                          \
    return result;                                                             \
  }

IMPLEMENT_JS_FUNCTION_ARG_1_OBJ(getNamedOwnKeys);
IMPLEMENT_JS_FUNCTION_ARG_1_OBJ(getIndexedOwnKeys);

// b. Implement javascript bindings that requires multiple parameters

JsValueRef createPropertyDescriptor(bool writable,
                                    bool enumerable,
                                    bool configurable,
                                    JsValueRef value,
                                    JsValueRef getter,
                                    JsValueRef setter) {
  ContextShim* contextShim = IsolateShim::GetCurrent()->currentContext();
  auto keepalive = contextShim->GetKeepAliveWrapper();
  auto fn = keepalive->GetcreatePropertyDescriptorFunction();
  JsValueRef arguments[] = {CreateJsValue(writable),
                            CreateJsValue(enumerable),
                            CreateJsValue(configurable),
                            value,
                            getter,
                            setter};

  JsValueRef result;

  if (CallJsFunction(contextShim->contextRef(),
                     keepalive->GetJsValueRef(),
                     fn,
                     arguments,
                     _countof(arguments),
                     &result) != JsNoError) {
    // TODO : change return type to JsErrorCode
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }

  return result;
}

// c. Implement javascript bindings for isAnyType.

#define IMPLEMENT_CALL_JS_FUNCTION_IS_TYPE(F)                                  \
  JsValueRef F(JsValueRef object) {                                            \
    ContextShim* contextShim = IsolateShim::GetCurrent()->currentContext();    \
    auto keepalive = contextShim->GetKeepAliveWrapper();                       \
    JsValueRef arguments[] = {object};                                         \
    JsValueRef result;                                                         \
    if (CallJsFunction(contextShim->contextRef(),                              \
                       keepalive->GetJsValueRef(),                             \
                       keepalive->Get##F##Function(),                          \
                       arguments,                                              \
                       _countof(arguments),                                    \
                       &result) != JsNoError) {                                \
      NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();                                   \
    }                                                                          \
    return result;                                                             \
  }

#define DEF_IS_TYPE(F) IMPLEMENT_CALL_JS_FUNCTION_IS_TYPE(F)
#include "escargotproperyid.inc"
#undef DEF_IS_TYPE
#undef IMPLEMENT_CALL_JS_FUNCTION_IS_TYPE

}  // namespace js

}  // namespace EscargotShim
