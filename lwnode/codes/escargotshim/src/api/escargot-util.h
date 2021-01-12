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

#pragma once

#include <EscargotPublic.h>

#define _Out_

typedef Escargot::VMInstanceRef* JsVMInstanceRef;
typedef Escargot::ContextRef* JsContextRef;
typedef Escargot::ExecutionStateRef* JsExecutionStateRef;
typedef Escargot::ValueRef* JsValueRef;
typedef Escargot::ValueRef* JsNumberRef;
typedef Escargot::PointerValueRef* JsPointerValueRef;
typedef Escargot::ObjectRef* JsObjectRef;
typedef Escargot::ArrayObjectRef* JsArrayObjectRef;
typedef Escargot::StringRef* JsStringRef;
typedef Escargot::ScriptRef* JsScriptRef;
typedef Escargot::SymbolRef* JsSymbolRef;
typedef Escargot::SymbolObjectRef* JsSymbolObjectRef;
typedef Escargot::FunctionObjectRef* JsFunctionRef;
typedef Escargot::ProxyObjectRef* JsProxyRef;
typedef Escargot::ArrayBufferObjectRef* JsArrayBufferRef;
typedef Escargot::FunctionObjectRef::NativeFunctionInfo JsNativeFunctionInfo;
typedef Escargot::ValueVectorRef* JsValueVectorRef;
typedef Escargot::BooleanObjectRef* JsBooleanObjectRef;
typedef Escargot::ErrorObjectRef::Code JsErrorObjectCode;

namespace EscargotShim {

enum JsErrorCode {
  JsNoError = 0,
  JsErrorCategoryScript = 0x30000,
  JsErrorScriptException,
  JsErrorScriptCompile,
  JsErrorScriptExecution,
  JsErrorCategoryFatal = 0x40000,
  JsErrorFatal,
};

JsContextRef CreateJsContext(JsVMInstanceRef vmInstance);

JsValueRef JsUndefined();
JsValueRef JsTrue();
JsValueRef JsFalse();
JsValueRef CreateJsValue(bool value);
JsValueRef CreateJsValue(int value);
JsValueRef CreateJsValue(uint32_t value);
JsValueRef CreateJsValue(float value);
JsValueRef CreateJsValue(double value);
JsValueRef CreateJsValue(long value);
JsValueRef CreateJsValue(unsigned long value);
JsValueRef CreateJsValue(long long value);
JsValueRef CreateJsValue(unsigned long long value);
JsValueRef CreateJsValue(JsPointerValueRef value);
JsValueRef CreateJsNull();
JsValueRef CreateJsUndefined();
JsValueRef CreateJsValueString(const char* str);
JsValueRef CreateJsValueSymbol(JsStringRef string);
JsValueRef CreateJsValueSymbolFor(JsVMInstanceRef vminstance,
                                  JsStringRef string);
JsErrorCode CreateJsObject(JsContextRef context, _Out_ JsObjectRef& object);
JsErrorCode CreateJsObject(JsContextRef context, _Out_ JsObjectRef& object);

}  // namespace EscargotShim
