
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

#ifndef ESCARGOTSHIM_H_
#define ESCARGOTSHIM_H_

#include "EscargotPublic.h"

enum JsErrorCode {
  JsNoError = 0,
  JsErrorCategoryScript = 0x30000,
  JsErrorScriptException,
  JsErrorScriptCompile,
  JsErrorScriptExecution,
  JsErrorCategoryFatal = 0x40000,
  JsErrorFatal,
};

typedef Escargot::VMInstanceRef* JsVMInstanceRef;
typedef Escargot::ContextRef* JsContextRef;
typedef Escargot::PlatformRef* JsPlatformRef;
typedef Escargot::ExecutionStateRef* JsExecutionStateRef;
typedef Escargot::ValueRef* JsValueRef;
typedef Escargot::ValueRef* JsNumberRef;
typedef Escargot::PointerValueRef* JsPointerValueRef;
typedef Escargot::ObjectRef* JsObjectRef;
typedef Escargot::ArrayObjectRef* JsArrayObjectRef;
typedef Escargot::StringRef* JsStringRef;
typedef Escargot::AtomicStringRef* JsAtomicStringRef;
typedef Escargot::ScriptRef* JsScriptRef;
typedef Escargot::SymbolRef* JsSymbolRef;
typedef Escargot::SymbolObjectRef* JsSymbolObjectRef;
typedef Escargot::FunctionObjectRef* JsFunctionRef;
typedef Escargot::ProxyObjectRef* JsProxyObjectRef;
typedef Escargot::ArrayBufferObjectRef* JsArrayBufferRef;
typedef Escargot::FunctionObjectRef::NativeFunctionInfo JsNativeFunctionInfo;
typedef Escargot::ValueVectorRef* JsValueVectorRef;
typedef Escargot::BooleanObjectRef* JsBooleanObjectRef;
typedef Escargot::MapObjectRef* JsMapObjectRef;
typedef Escargot::ErrorObjectRef* JsErrorObjectRef;
typedef Escargot::ErrorObjectRef::Code JsErrorObjectCode;
typedef Escargot::RangeErrorObjectRef* JsRangeErrorObjectRef;
typedef Escargot::ReferenceErrorObjectRef* JsReferenceErrorObjectRef;
typedef Escargot::SyntaxErrorObjectRef* JsSyntaxErrorObjectRef;
typedef Escargot::TypeErrorObjectRef* JsTypeErrorObjectRef;

typedef Escargot::TemplateRef* JsTemplateRef;
typedef Escargot::ObjectTemplateRef* JsObjectTemplateRef;
typedef Escargot::FunctionTemplateRef* JsFunctionTemplateRef;
typedef Escargot::ObjectRef::NativeDataAccessorPropertySetter JsNativeDataAccessorPropertySetter;
typedef Escargot::ObjectRef::NativeDataAccessorPropertyData JsNativeDataAccessorPropertyData;
typedef Escargot::ObjectTemplateNamedPropertyHandlerData JsObjectTemplateNamedPropertyHandlerData;
typedef Escargot::TemplatePropertyNameRef JsTemplatePropertyNameRef;
typedef Escargot::TemplatePropertyAttribute JsTemplatePropertyAttribute;
typedef Escargot::TemplateNamedPropertyHandlerEnumerationCallbackResultVector JsTemplateNamedPropertyHandlerEnumerationCallbackResultVector;
typedef Escargot::ObjectPropertyDescriptorRef JsObjectPropertyDescriptorRef;

typedef Escargot::Evaluator JsEvaluator;

typedef void* JsRef;

namespace v8 {
  class Isolate;
  class Data;
  class Value;
  class Template;
}

namespace EscargotShim {
class IsolateShim;
class ContextShim;
class ContextScope;
void addToPersistentStorage(v8::Isolate* isolate, void* jsObject);
// FIXME: Need to pass an IsolateShim* as a parameter
void removeFromPersistentStorage(void* jsObject);

void addToLocalScope(v8::Isolate* isolate, void* jsObject);
JsValueRef asJsValueRef(const v8::Value* value);
}  // namespace EscargotShim

#endif  // ESCARGOTSHIM_H_
