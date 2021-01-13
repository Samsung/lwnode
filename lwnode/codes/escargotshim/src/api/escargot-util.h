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

typedef Escargot::VMInstanceRef JsVMInstance;
typedef Escargot::ContextRef JsContext;
typedef Escargot::ExecutionStateRef JsExecutionState;
typedef Escargot::ValueRef JsValue;
typedef Escargot::ValueRef JsNumber;
typedef Escargot::PointerValueRef JsPointerValue;
typedef Escargot::ObjectRef JsObject;
typedef Escargot::ArrayObjectRef JsArrayObject;
typedef Escargot::StringRef JsString;
typedef Escargot::ScriptRef JsScript;
typedef Escargot::SymbolRef JsSymbol;
typedef Escargot::SymbolObjectRef JsSymbolObject;
typedef Escargot::FunctionObjectRef JsFunction;
typedef Escargot::ProxyObjectRef JsProxy;
typedef Escargot::ArrayBufferObjectRef JsArrayBuffer;
typedef Escargot::FunctionObjectRef::NativeFunctionInfo JsNativeFunctionInfo;
typedef Escargot::ValueVectorRef JsValueVector;
typedef Escargot::BooleanObjectRef JsBooleanObject;
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

JsValue* JsUndefined();
JsValue* JsTrue();
JsValue* JsFalse();
JsValue* CreateJsValue(bool value);
JsValue* CreateJsValue(int value);
JsValue* CreateJsValue(uint32_t value);
JsValue* CreateJsValue(float value);
JsValue* CreateJsValue(double value);
JsValue* CreateJsValue(long value);
JsValue* CreateJsValue(unsigned long value);
JsValue* CreateJsValue(long long value);
JsValue* CreateJsValue(unsigned long long value);

}  // namespace EscargotShim
