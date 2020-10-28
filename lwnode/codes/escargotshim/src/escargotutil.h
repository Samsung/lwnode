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

#ifndef __ESCARGOT_UTIL__
#define __ESCARGOT_UTIL__

#include "escargotbase.h"
#include "escargotcontextshim.h"
#include "escargotisolateshim.h"

#define _Out_

namespace v8 {
enum class ArrayBufferCreationMode;
class Isolate;
enum class KeyCollectionMode;
enum PropertyFilter : uint8_t;
enum class IndexFilter;
enum class KeyConversionMode;
}  // namespace v8

namespace EscargotShim {

typedef std::function<JsValueRef(JsExecutionStateRef state)> EvalScriptCallback;
using EvalScriptCallback2 = JsValueRef (*)(JsExecutionStateRef state);
typedef Escargot::FunctionObjectRef::NativeFunctionPointer
    NativeFunctionPointer;
typedef Escargot::FunctionObjectRef::NativeFunctionInfo NativeFunctionInfo;

// FIXME: To cast from a v8::Value to JsValueRef, use v8::Value::asValueRef(),
// as the type casting is type-checked. Use CastTo() to only type-cast
// non v8::Value to JsValueRef. Do proper type-checked type-casting for
// all needed type-castings and remove this CastTo().
template <typename T, typename F>
inline T CastTo(F* p) {
  // FIXME: checking for nullptr should be done before calling CastTo()
  return (p == nullptr)
             ? nullptr
             : reinterpret_cast<T>(
                   const_cast<typename std::remove_const<F>::type*>(p));
}

#define VERIFY_EVAL_RESULT(result, returnValue)                                \
  do {                                                                         \
    if (!result.isSuccessful()) {                                              \
      NESCARGOT_LOG_ERROR("Eval Error at %s (%s:%d)\n",                        \
                          __PRETTY_FUNCTION__,                                 \
                          __FILE__,                                            \
                          __LINE__);                                           \
      EscargotShim::LoggingJSErrorInfo(result);                                \
      return returnValue;                                                      \
    }                                                                          \
  } while (0)

#define VERIFY_EVAL_RESULT_AND_THROW_EXCEPTION(result, returnValue)            \
  do {                                                                         \
    if (!result.isSuccessful()) {                                              \
      auto scriptException = IsolateShim::GetCurrent()->GetScriptException();  \
      if (!scriptException->IsThrownException()) {                             \
        scriptException->SetException(result);                                 \
      }                                                                        \
      return returnValue;                                                      \
    }                                                                          \
  } while (0)

#define VERIFY_JS_ERROR(expr, ...)                                             \
  {                                                                            \
    JsErrorCode _error = (expr);                                               \
    if (_error != JsNoError) {                                                 \
      return _error, ##__VA_ARGS__;                                            \
    }                                                                          \
  }

#define GENERATE_CONTEXT_AND_CHECK_EXIST(context_)                             \
  JsContextRef contextRef = nullptr;                                           \
  UNUSED(contextRef);                                                          \
  if (context_.IsEmpty()) {                                                    \
    auto isolate_ = EscargotShim::IsolateShim::GetCurrent();                   \
    contextRef = isolate_->currentContext()->contextRef();                     \
  } else {                                                                     \
    contextRef = ContextShim::ToContextShim(*context_)->contextRef();          \
  }

class ScriptData : public gc {
 public:
  ScriptData(JsContextRef context, JsScriptRef script = nullptr)
      : context(context), script(script) {}
  JsContextRef context;
  JsScriptRef script;
};

enum class ConsoleAPIType {
  kLog,
  kInfo,
  kError,
  kWarn,
  kDebug,
};

void InitializeEscargotShim();
void FinalizeEscargotShim();

JsVMInstanceRef CreateJsVmInstance(JsPlatformRef platform,
                                   bool isPlatformDeletedWithVMInstance = true);
JsContextRef CreateJsContext(JsVMInstanceRef vmInstance);

JsValueRef GetCachedJsValueStringIfExist(JsValueRef name);
JsValueRef GetCachedJsValueString(CachedStringId id);
JsContextRef GetCurrentJsContextRef();

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

JsErrorCode CreateJsFunction(JsContextRef context,
                             JsNativeFunctionInfo& info,
                             _Out_ JsFunctionRef& object);
JsErrorCode CreateJsFunction(JsNativeFunctionInfo& info,
                             _Out_ JsFunctionRef& object);

JsFunctionTemplateRef CreateJsFunctionTemplate(
    JsAtomicStringRef name,
    size_t argumentCount,
    bool isStrict,
    bool isConstructor,
    Escargot::FunctionTemplateRef::NativeFunctionPointer fn);

JsObjectTemplateRef CreateJsObjectTemplate();

JsErrorCode CreateJsScriptObject(JsContextRef contextRef,
                                 JsStringRef sourceRef,
                                 JsStringRef filenameRef,
                                 JsScriptRef parsedScript,
                                 _Out_ JsObjectRef& scriptObject);

JsValueRef GetCachedJsValueStringIfExist(JsValueRef name);
JsValueRef GetCachedJsValue(CachedStringId id);
JsValueRef GetCachedJsValue(CachedSymbolId id);

JsContextRef GetCurrentJsContextRef();

JsErrorCode CreateJsProxy(JsContextRef context,
                          JsObjectRef target,
                          JsObjectRef handle,
                          _Out_ JsProxyObjectRef& proxy);
JsErrorCode CreateJsProxy(JsObjectRef target,
                          JsObjectRef handle,
                          _Out_ JsProxyObjectRef& proxy);

JsErrorCode CreateJsArrayBufferObject(JsContextRef context,
                                      void* buffer,
                                      size_t byteLength,
                                      v8::ArrayBufferCreationMode mode,
                                      _Out_ JsArrayBufferRef& arrayBuffer);
JsErrorCode CreateJsArrayBufferObject(JsContextRef context,
                                      size_t byteLength,
                                      _Out_ JsArrayBufferRef& arrayBuffer);

JsErrorCode CreateJsArrayObject(JsContextRef context,
                                _Out_ JsArrayObjectRef& arrayObject);
JsErrorCode CreateJsArrayObject(_Out_ JsArrayObjectRef& arrayObject);

JsErrorCode CreateJsBooleanObject(JsContextRef context,
                                  JsValueRef value,
                                  _Out_ JsBooleanObjectRef& booleanObject);
JsErrorCode CreateJsBooleanObject(JsValueRef value,
                                  _Out_ JsBooleanObjectRef& booleanObject);

JsErrorCode CreateJsSymbolObject(JsContextRef context,
                                 JsValueRef value,
                                 _Out_ JsSymbolObjectRef& symbolObject);
JsErrorCode CreateJsSymbolObject(JsValueRef value,
                                 _Out_ JsSymbolObjectRef& symbolObject);

JsErrorCode CreateJsErrorObject(JsContextRef context,
                                JsErrorObjectCode errorCode,
                                JsValueRef errorMessage,
                                _Out_ JsErrorObjectRef& errorObject);

JsErrorCode CreateBuiltinFunction(JsContextRef context,
                                  NativeFunctionInfo& info,
                                  _Out_ JsFunctionRef& function);

JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        JsValueRef value,
                        _Out_ bool& result);
JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        const char* property,
                        JsValueRef value,
                        _Out_ bool& result);
JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        CachedStringId id,
                        JsValueRef value,
                        _Out_ bool& result);

JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        _Out_ JsValueRef& value);
JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        const char* property,
                        _Out_ JsValueRef& value);
JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        CachedStringId id,
                        _Out_ JsValueRef& value);

JsErrorCode HasProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        _Out_ bool& result);

JsErrorCode DeleteProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef property,
                           _Out_ bool& result);

JsErrorCode HasOwnProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef property,
                           _Out_ bool& result);

JsErrorCode SetPrototype(JsContextRef context,
                         JsObjectRef object,
                         JsValueRef proto,
                         _Out_ bool& result);

JsErrorCode GetPrototype(JsContextRef context,
                         JsObjectRef object,
                         _Out_ JsValueRef& proto);

JsErrorCode DefineDataProperty(JsContextRef context,
                               JsObjectRef object,       // NOLINT
                               JsValueRef propertyName,  // NOLINT
                               bool isWritable,          // NOLINT
                               bool isEnumerable,        // NOLINT
                               bool isConfigurable,      // NOLINT
                               JsValueRef propertyValue,
                               NativeFunctionPointer functionPropertyValue,
                               JsValueRef getter,
                               JsValueRef setter,
                               _Out_ bool& result);

JsErrorCode DefineOwnProperty(JsContextRef context,
                              JsObjectRef object,
                              JsValueRef propertyName,
                              JsValueRef descriptor);

JsErrorCode GetOwnPropertyDescriptor(JsContextRef context,
                                     JsObjectRef object,
                                     JsValueRef propertyName,
                                     _Out_ JsValueRef& descriptor);

JsErrorCode GetOwnProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef propertyName,
                           _Out_ JsValueRef& value);

JsErrorCode GetPropertyNames(JsContextRef context,
                             JsObjectRef object,
                             v8::KeyCollectionMode mode,
                             v8::PropertyFilter propertyFilter,
                             v8::IndexFilter indexFilter,
                             v8::KeyConversionMode keyConversion,
                             _Out_ JsValueRef& arrayObjRef);

JsErrorCode GetObjectConstructor(JsContextRef context,
                                 JsObjectRef objectRef,
                                 _Out_ JsValueRef& constructorRef);

JsErrorCode ConstructObject(JsContextRef context,
                            JsValueRef constructor,
                            _Out_ JsObjectRef& object);

JsErrorCode CopyProperty(JsContextRef context,
                         JsObjectRef object,
                         JsObjectRef property);

JsStringRef CreateJsStringFromASCII(const char* str);
JsStringRef CreateJsStringFromASCII(const char* str, size_t len);
JsStringRef CreateJsStringFromUTF8(const char* str, size_t len);
JsStringRef CreateJsStringFromUTF16(const char16_t* str, size_t len);
JsStringRef CreateJsStringFromLatin1(const unsigned char* str, size_t len);
JsStringRef CreateJsEmptyString();
JsStringRef CreateJsStringExternalFromUTF16(const char16_t* str, size_t len);
JsStringRef CreateJsStringExternalFromLatin1(const unsigned char* str,
                                             size_t len);
JsAtomicStringRef CreateJsEmptyAtomicString();

void SetExtraData(JsObjectRef object, void* data);
JsRef GetExtraData(JsObjectRef object);

JsErrorCode CallJsFunction(JsContextRef context,
                           JsValueRef thisValue,
                           JsValueRef fn,
                           JsValueRef* argv,
                           size_t argc,
                           _Out_ JsValueRef* result = JS_INVALID_REFERENCE);

JsErrorCode ParseScript(JsContextRef contextRef,
                        JsStringRef sourceRef,
                        JsStringRef filenameRef,
                        _Out_ JsScriptRef& scriptRef);

void CreateConsoleObject(JsContextRef context);

template <typename... Args, typename F>
EvaluatorResult EvalScript(JsContextRef context, F&& closure, Args... args) {
  return JsEvaluator::execute(context, closure, args...);
}

void LoggingJSErrorInfo(const EvaluatorResult& sbResult);

void printToConsole(const char* format, JsStringRef* stringObj);
void PrintJsValue(JsValueRef value, int depsSize = 0);

JsErrorCode GetPrivate(JsContextRef context,
                       JsObjectRef object,
                       JsValueRef key,
                       _Out_ JsValueRef& result);

JsErrorCode SetPrivate(JsContextRef context,
                       JsObjectRef object,
                       JsValueRef key,
                       JsValueRef value);

bool HasPrivate(JsContextRef context, JsObjectRef object, JsValueRef key);

void addToLocalScope(v8::Isolate* isolate, void* jsObject);
void addToPersistentStorage(v8::Isolate* isolate, void* jsObject);
void removeFromPersistentStorage(void* jsObject);

JsErrorCode RemoveFromHiddenClassChain(JsContextRef context,
                                       JsObjectRef object);

}  // namespace EscargotShim

#endif
