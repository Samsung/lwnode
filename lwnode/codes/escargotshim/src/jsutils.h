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

#ifndef __JS_UTILS__
#define __JS_UTILS__

#include "escargotbase.h"
#include "escargotshim.h"
#include "escargotutil.h"

namespace EscargotShim {

class JsObjectWrapper : public gc {
 public:
  JsObjectWrapper(JsContextRef context);
  JsValueRef GetJsValueRef();
  DEFINE_GETTER(JsContextRef, context);
  DEFINE_GETTER(JsObjectRef, object);

 protected:
  JsContextRef m_context = {JS_INVALID_REFERENCE};
  JsObjectRef m_object = {JS_INVALID_REFERENCE};
};

class JsKeepAliveWrapper : public JsObjectWrapper {
 public:
  JsKeepAliveWrapper(JsContextRef context);

#define DECLARE_JS_FUNCTION_GETTER(F)                                          \
 public:                                                                       \
  JsValueRef Get##F##Function();                                               \
                                                                               \
 private:                                                                      \
  JsValueRef m_##F##Function = {JS_INVALID_REFERENCE};

#define DEF_IS_TYPE(F) DECLARE_JS_FUNCTION_GETTER(F)
#include "escargotproperyid.inc"
#undef DEF_IS_TYPE

  // TODO: delete escargotshim.js features that escargot public api provides
  DECLARE_JS_FUNCTION_GETTER(cloneObject);
  DECLARE_JS_FUNCTION_GETTER(getEnumerableNamedProperties);
  DECLARE_JS_FUNCTION_GETTER(getEnumerableIndexedProperties);
  DECLARE_JS_FUNCTION_GETTER(createEnumerationIterator);
  DECLARE_JS_FUNCTION_GETTER(createPropertyDescriptor);
  DECLARE_JS_FUNCTION_GETTER(createPropertyDescriptorsEnumerationIterator);
  DECLARE_JS_FUNCTION_GETTER(getNamedOwnKeys);
  DECLARE_JS_FUNCTION_GETTER(getIndexedOwnKeys);
  DECLARE_JS_FUNCTION_GETTER(getStackTrace);
  DECLARE_JS_FUNCTION_GETTER(getSymbolKeyFor);
  DECLARE_JS_FUNCTION_GETTER(getSymbolFor);
  DECLARE_JS_FUNCTION_GETTER(ensureDebug);
  DECLARE_JS_FUNCTION_GETTER(enqueueMicrotask);
  DECLARE_JS_FUNCTION_GETTER(dequeueMicrotask);
  DECLARE_JS_FUNCTION_GETTER(getPropertyAttributes);
  DECLARE_JS_FUNCTION_GETTER(getOwnPropertyNames);
  DECLARE_JS_FUNCTION_GETTER(jsonParse);
  DECLARE_JS_FUNCTION_GETTER(jsonStringify);
};

namespace js {

JsValueRef GetStringConcatFunction(JsContextRef context);

JsValueRef concatArray(JsContextRef context,
                       JsValueRef first,
                       JsValueRef second);

// a. Declare javascript bindings that requires a single object as its parameter

#define DECLARE_JS_FUNCTION_ARG_1_OBJ(F) JsValueRef F(JsValueRef object);
DECLARE_JS_FUNCTION_ARG_1_OBJ(getNamedOwnKeys)
DECLARE_JS_FUNCTION_ARG_1_OBJ(getIndexedOwnKeys)
#undef DECLARE_JS_FUNCTION_ARG_1_OBJ

// b. Declare javascript bindings that requires multiple parameters

JsValueRef createPropertyDescriptor(bool writable,
                                    bool enumerable,
                                    bool configurable,
                                    JsValueRef value,
                                    JsValueRef getter,
                                    JsValueRef setter);

// c. Declare javascript bindings for isAnyType.

#define DECLARE_JS_FUNCTION_IS_TYPE(F) JsValueRef F(JsValueRef object);
#define DEF_IS_TYPE(F) DECLARE_JS_FUNCTION_IS_TYPE(F)
#include "escargotproperyid.inc"
#undef DEF_IS_TYPE
#undef DECLARE_JS_FUNCTION_IS_TYPE

template <typename... Args>
JsErrorCode Call(CachedStringId name, _Out_ JsValueRef& result, Args... args) {
  auto keepalive =
      IsolateShim::GetCurrent()->currentContext()->GetKeepAliveWrapper();
  JsValueRef fn = JS_INVALID_REFERENCE;
  if (GetProperty(keepalive->context(), keepalive->object(), name, fn) !=
      JsNoError) {
    return JsErrorScriptExecution;
  }
  NESCARGOT_ASSERT(fn->isFunctionObject());

  JsValueRef jsArgs[] = {args...};
  if (CallJsFunction(keepalive->context(),
                     keepalive->GetJsValueRef(),
                     fn,
                     jsArgs,
                     sizeof...(args),
                     &result) != JsNoError) {
    return JsErrorScriptExecution;
  }
  return JsNoError;
}
}  // namespace js

}  // namespace EscargotShim

#endif
