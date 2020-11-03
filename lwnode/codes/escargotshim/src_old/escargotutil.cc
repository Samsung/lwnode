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

#include "escargotutil.h"
#include "escargotbase.h"
#include "escargotisolateshim.h"
#include "gc.h"
#include "v8utils.h"

using namespace Escargot;

namespace EscargotShim {

static void InitializeGC();
static void FinalizeGC();

static void PrettyBytes(char* buf, size_t nBuf, size_t bytes) {
  const char* suffix[7] = {
      "B",
      "KB",
      "MB",
      "GB",
      "TB",
      "PB",
      "EB",
  };
  size_t s = 0;
  double c = bytes;
  while (c >= 1024 && s < 7) {
    c /= 1024;
    s++;
  }
  if (c - ((int)c) == 0.f) {
    snprintf(buf, nBuf, "%d %s", (int)c, suffix[s]);
  } else {
    snprintf(buf, nBuf, "%.1f %s", c, suffix[s]);
  }
}

static void PrintGCStats(bool forcePrint = false) {
  char use[20], heap[20];
  PrettyBytes(use, sizeof(use), GC_get_memory_use());
  PrettyBytes(heap, sizeof(heap), GC_get_heap_size());
  if (forcePrint) {
    NESCARGOT_LOG_INFO(
        COLOR_DIM "GC: use:%s, heap:%s\n" COLOR_RESET, use, heap);
  } else {
    NESCARGOT_LOG(1, INFO, "GC: use:%s, heap:%s\n" COLOR_RESET, use, heap);
  }
}

static void InitializeGC() {
  Memory::setGCFrequency(24);
  GC_set_force_unmap_on_gcollect(1);

  GC_set_abort_func([](const char* msg) {
    NESCARGOT_LOG_ERROR("EscargotShim: GC aborted\n");
    NESCARGOT_LOG_ERROR("%s\n", msg);
  });

  GC_set_warn_proc([](char* msg, GC_word arg) {
    NESCARGOT_LOG_ERROR("EscargotShim: GC warning\n");
    NESCARGOT_LOG_ERROR("%s\n", msg);
  });

  GC_set_on_collection_event([](GC_EventType evtType) {
    if (GC_EVENT_RECLAIM_END == evtType) {
      PrintGCStats();
    }
  });

  atexit([]() { PrintGCStats(true); });
}

static void FinalizeGC() {
  GC_register_mark_stack_func([]() {
    // do nothing for skip stack
    // assume there is no gc-object on stack
  });
  GC_gcollect();
  GC_gcollect();
  GC_gcollect_and_unmap();
  GC_register_mark_stack_func(nullptr);
}

void InitializeEscargotShim() {
  Escargot::Globals::initialize();
  InitializeGC();
}

void FinalizeEscargotShim() {
  FinalizeGC();
  Escargot::Globals::finalize();
}

JsVMInstanceRef CreateJsVmInstance(JsPlatformRef platform,
                                   bool isPlatformDeletedWithVMInstance) {
  auto instance = VMInstanceRef::create(platform);

  if (isPlatformDeletedWithVMInstance) {
    instance->setOnVMInstanceDelete(
        [](VMInstanceRef* instance) { delete instance->platform(); });
  }
  return instance;
}

JsContextRef CreateJsContext(JsVMInstanceRef vmInstance) {
  return ContextRef::create(vmInstance);
}
JsValueRef JsUndefined() {
  return IsolateShim::GetCurrent()->Cached()->Undefined();
}

JsValueRef JsTrue() {
  return IsolateShim::GetCurrent()->Cached()->True();
}

JsValueRef JsFalse() {
  return IsolateShim::GetCurrent()->Cached()->False();
}

JsValueRef CreateJsValue(bool value) {
  return ValueRef::create(value);
}

JsValueRef CreateJsValue(int value) {
  return ValueRef::create(value);
}

JsValueRef CreateJsValue(uint32_t value) {
  return ValueRef::create((unsigned long)value);
}

JsValueRef CreateJsValue(float value) {
  return ValueRef::create(value);
}

JsValueRef CreateJsValue(double value) {
  return ValueRef::create(value);
}

JsValueRef CreateJsValue(long value) {
  return ValueRef::create(value);
}
JsValueRef CreateJsValue(unsigned long value) {
  return ValueRef::create(value);
}
JsValueRef CreateJsValue(long long value) {
  return ValueRef::create(value);
}
JsValueRef CreateJsValue(unsigned long long value) {
  return ValueRef::create(value);
}

// JsValueRef CreateJsValue(JsStringRef value) { return ValueRef::create(value);
// }

// JsValueRef CreateJsValue(JsPointerValueRef value) {
//   return ValueRef::create(value);
// }

JsValueRef CreateJsValue(JsPointerValueRef value) {
  return reinterpret_cast<JsValueRef>(value);
}

JsValueRef CreateJsNull() {
  return ValueRef::createNull();
}

JsValueRef CreateJsUndefined() {
  return ValueRef::createUndefined();
}

JsValueRef CreateJsValueString(const char* str) {
  return CreateJsStringFromASCII(str);
}

JsValueRef CreateJsValueSymbol(JsStringRef string) {
  return SymbolRef::create(string);
}

JsValueRef CreateJsValueSymbolFor(JsVMInstanceRef vminstance,
                                  JsStringRef string) {
  return SymbolRef::fromGlobalSymbolRegistry(vminstance, string);
}

JsErrorCode CreateJsObject(JsContextRef context, _Out_ JsObjectRef& object) {
  NESCARGOT_ASSERT(context);
  auto result =
      EvalScript(context, [](JsExecutionStateRef state) -> JsValueRef {
        return CreateJsValue(ObjectRef::create(state));
      });

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  object = result.result->asObject();
  return JsNoError;
}

JsErrorCode CreateJsFunction(JsContextRef context,
                             JsNativeFunctionInfo& info,
                             _Out_ JsFunctionRef& object) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsNativeFunctionInfo info) -> JsValueRef {
        return FunctionObjectRef::create(state, info);
      },
      info);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  object = result.result->asFunctionObject();
  return JsNoError;
}

JsErrorCode CreateJsFunction(JsNativeFunctionInfo& info,
                             _Out_ JsFunctionRef& object) {
  return CreateJsFunction(EscargotShim::GetCurrentJsContextRef(), info, object);
}

JsFunctionTemplateRef CreateJsFunctionTemplate(
    JsAtomicStringRef name,
    size_t argumentCount,
    bool isStrict,
    bool isConstructor,
    Escargot::FunctionTemplateRef::NativeFunctionPointer fn) {
  return FunctionTemplateRef::create(
      name, argumentCount, isStrict, isConstructor, fn);
}

JsObjectTemplateRef CreateJsObjectTemplate() {
  return ObjectTemplateRef::create();
}

JsErrorCode CreateJsScriptObject(JsContextRef contextRef,
                                 JsStringRef sourceRef,
                                 JsStringRef filenameRef,
                                 JsScriptRef parsedScript,
                                 _Out_ JsObjectRef& scriptObject) {
  if (CreateJsObject(contextRef, scriptObject) != JsNoError) {
    return JsErrorScriptExecution;
  }

  JsErrorCode error = RemoveFromHiddenClassChain(contextRef, scriptObject);
  NESCARGOT_ASSERT(error == JsNoError);

  bool result = false;
  if (SetProperty(contextRef,
                  scriptObject,
                  CachedStringId::source,
                  sourceRef,
                  result) != JsNoError) {
    return JsErrorScriptExecution;
  }
  NESCARGOT_ASSERT(result);

  if (SetProperty(contextRef,
                  scriptObject,
                  CachedStringId::filename,
                  filenameRef,
                  result) != JsNoError) {
    return JsErrorScriptExecution;
  }
  NESCARGOT_ASSERT(result);

  SetExtraData(scriptObject, new ScriptData(contextRef, parsedScript));

  return JsNoError;
}

JsErrorCode CreateJsProxy(JsContextRef context,
                          JsObjectRef target,
                          JsObjectRef handle,
                          _Out_ JsProxyObjectRef& proxy) {
  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsObjectRef target,
                    JsObjectRef handle) -> ValueRef* {
                   return ProxyObjectRef::create(state, target, handle);
                 },
                 target,
                 handle);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  proxy = result.result->asObject()->asProxyObject();
  return JsNoError;
}

JsErrorCode CreateJsProxy(JsObjectRef target,
                          JsObjectRef handle,
                          _Out_ JsProxyObjectRef& proxy) {
  return CreateJsProxy(
      EscargotShim::GetCurrentJsContextRef(), target, handle, proxy);
}

JsErrorCode CreateJsArrayBufferObject(JsContextRef context,
                                      void* buffer,
                                      size_t byteLength,
                                      v8::ArrayBufferCreationMode mode,
                                      _Out_ JsArrayBufferRef& arrayBuffer) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state,
         void* buffer,
         size_t byteLength,
         v8::ArrayBufferCreationMode mode) -> JsValueRef {
        auto arrayBuffer = ArrayBufferObjectRef::create(state);
        arrayBuffer->attachBuffer(state, buffer, byteLength);

        v8::ArrayBufferData* externalData = new v8::ArrayBufferData(mode);
        SetExtraData(arrayBuffer, externalData);
        return arrayBuffer;
      },
      buffer,
      byteLength,
      mode);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  arrayBuffer = result.result->asObject()->asArrayBufferObject();
  return JsNoError;
}

JsErrorCode CreateJsArrayBufferObject(JsContextRef context,
                                      size_t byteLength,
                                      _Out_ JsArrayBufferRef& arrayBuffer) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, size_t byteLength) -> JsValueRef {
        auto arrayBuffer = ArrayBufferObjectRef::create(state);
        arrayBuffer->allocateBuffer(state, byteLength);
        return arrayBuffer;
      },
      byteLength);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  arrayBuffer = result.result->asObject()->asArrayBufferObject();
  return JsNoError;
}

JsErrorCode CreateJsArrayObject(JsContextRef context,
                                _Out_ JsArrayObjectRef& arrayObject) {
  auto result =
      EvalScript(context, [](JsExecutionStateRef state) -> JsValueRef {
        return ArrayObjectRef::create(state);
      });

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  arrayObject = result.result->asObject()->asArrayObject();
  return JsNoError;
}

JsErrorCode CreateJsArrayObject(_Out_ JsArrayObjectRef& arrayObject) {
  return CreateJsArrayObject(EscargotShim::GetCurrentJsContextRef(),
                             arrayObject);
}

JsErrorCode CreateJsBooleanObject(JsContextRef context,
                                  JsValueRef value,
                                  _Out_ JsBooleanObjectRef& booleanObject) {
  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state, JsValueRef value) -> JsValueRef {
                   BooleanObjectRef* object = BooleanObjectRef::create(state);

                   object->setPrimitiveValue(state, value);

                   return object;
                 },
                 value);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  booleanObject = result.result->asObject()->asBooleanObject();

  return JsNoError;
}

JsErrorCode CreateJsBooleanObject(JsValueRef value,
                                  _Out_ JsBooleanObjectRef& booleanObject) {
  return CreateJsBooleanObject(
      EscargotShim::GetCurrentJsContextRef(), value, booleanObject);
}

JsErrorCode CreateJsSymbolObject(JsContextRef context,
                                 JsValueRef value,
                                 _Out_ JsSymbolObjectRef& symbolObject) {
  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state, JsValueRef value) -> JsValueRef {
                   SymbolObjectRef* object = SymbolObjectRef::create(state);

                   object->setPrimitiveValue(state, value->asSymbol());

                   return object;
                 },
                 value);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  symbolObject = result.result->asObject()->asSymbolObject();
  return JsNoError;
}

JsErrorCode CreateJsSymbolObject(JsValueRef value,
                                 _Out_ JsSymbolObjectRef& symbolObject) {
  return CreateJsSymbolObject(
      EscargotShim::GetCurrentJsContextRef(), value, symbolObject);
}

JsErrorCode CreateJsErrorObject(JsContextRef context,
                                JsErrorObjectCode errorCode,
                                JsValueRef errorMessage,
                                _Out_ JsErrorObjectRef& errorObject) {
  EvaluatorResult result = EvalScript(
      context,
      [](JsExecutionStateRef state,
         JsValueRef errorMessage,
         JsErrorObjectCode errorCode) -> JsValueRef {
        JsErrorObjectRef errorObjectRef = Escargot::ErrorObjectRef::create(
            state, errorCode, errorMessage->asString());
        return errorObjectRef;
      },
      errorMessage,
      errorCode);

  errorObject = result.result->asObject()->asErrorObject();
  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  return JsNoError;
}

JsErrorCode CreateBuiltinFunction(JsContextRef context,
                                  NativeFunctionInfo& info,
                                  _Out_ JsFunctionRef& function) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, NativeFunctionInfo info) -> JsValueRef {
        return FunctionObjectRef::create(state, info);
      },
      info);
  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  function = result.result->asObject()->asFunctionObject();
  return JsNoError;
}

JsContextRef GetCurrentJsContextRef() {
  return IsolateShim::GetCurrent()->currentContext()->contextRef();
}

JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        JsValueRef value,
                        _Out_ bool& result) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(property != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(value != JS_INVALID_REFERENCE);

  auto evalResult = EvalScript(context,
                               [](JsExecutionStateRef state,
                                  JsObjectRef object,
                                  JsValueRef property,
                                  JsValueRef value) -> JsValueRef {
                                 bool setResult =
                                     object->set(state, property, value);
                                 return CreateJsValue(setResult);
                               },
                               object,
                               property,
                               value);

  VERIFY_EVAL_RESULT(evalResult, JsErrorScriptExecution);
  result = evalResult.result->asBoolean();
  return JsNoError;
}

JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        const char* property,
                        JsValueRef value,
                        _Out_ bool& result) {
  return SetProperty(
      context, object, CreateJsStringFromASCII(property), value, result);
}

JsErrorCode SetProperty(JsContextRef context,
                        JsObjectRef object,
                        CachedStringId id,
                        JsValueRef value,
                        _Out_ bool& result) {
  auto propertyName =
      CreateJsValue(IsolateShim::GetCurrent()->Cached()->Strings(id));
  return SetProperty(context, object, propertyName, value, result);
}

JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        _Out_ JsValueRef& value) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(property != JS_INVALID_REFERENCE);

  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsObjectRef object, JsValueRef property)
          -> JsValueRef { return object->get(state, property); },
      object,
      property);
  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  value = result.result;
  return JsNoError;
}

JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        const char* property,
                        _Out_ JsValueRef& value) {
  return GetProperty(context, object, CreateJsStringFromASCII(property), value);
}

JsErrorCode GetProperty(JsContextRef context,
                        JsObjectRef object,
                        CachedStringId id,
                        _Out_ JsValueRef& value) {
  auto propertyName =
      CreateJsValue(IsolateShim::GetCurrent()->Cached()->Strings(id));
  return GetProperty(context, object, propertyName, value);
}

JsErrorCode HasProperty(JsContextRef context,
                        JsObjectRef object,
                        JsValueRef property,
                        _Out_ bool& result) {
  JsValueRef value;

  VERIFY_JS_ERROR(GetProperty(context, object, property, value));
  result = value->isUndefined() ? false : true;
  return JsNoError;
}

JsErrorCode DeleteProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef property,
                           _Out_ bool& result) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(property != JS_INVALID_REFERENCE);

  // TODO: This implementation is buggy. (e.g it doesn't work for the test,
  // StringObjectDelete.)

  auto evalResult =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef property) -> JsValueRef {
                   bool result = object->deleteOwnProperty(state, property);
                   if (!result) {
                     auto ref = object->getPrototypeObject(state);
                     while (ref.hasValue()) {
                       if (ref.get()->deleteOwnProperty(state, property)) {
                         return CreateJsValue(true);
                       }
                       ref = ref.get()->getPrototypeObject(state);
                     }
                     result = false;
                   }
                   return CreateJsValue(result);
                 },
                 object,
                 property);

  VERIFY_EVAL_RESULT(evalResult, JsErrorScriptExecution);
  result = evalResult.result->asBoolean();
  return JsNoError;
}

JsErrorCode HasOwnProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef property,
                           _Out_ bool& result) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(property != JS_INVALID_REFERENCE);

  auto evalResult = EvalScript(
      context,
      [](JsExecutionStateRef state,
         JsObjectRef object,
         JsValueRef property) -> JsValueRef {
        return CreateJsValue(object->hasOwnProperty(state, property));
      },
      object,
      property);
  VERIFY_EVAL_RESULT(evalResult, JsErrorScriptExecution);
  result = evalResult.result->asBoolean();
  return JsNoError;
}

JsErrorCode SetPrototype(JsContextRef context,
                         JsObjectRef object,
                         JsValueRef proto,
                         _Out_ bool& result) {
  auto evalResult =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef proto) -> JsValueRef {
                   return CreateJsValue(object->setPrototype(state, proto));
                 },
                 object,
                 proto);

  VERIFY_EVAL_RESULT(evalResult, JsErrorScriptExecution);
  result = evalResult.result->asBoolean();
  return JsNoError;
}

JsErrorCode GetPrototype(JsContextRef context,
                         JsObjectRef object,
                         _Out_ JsValueRef& proto) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsObjectRef object) -> JsValueRef {
        return object->getPrototype(state);
      },
      object);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);

  proto = result.result;
  return JsNoError;
}

JsErrorCode GetPropertyNames(JsContextRef context,
                             JsObjectRef object,
                             v8::KeyCollectionMode mode,
                             v8::PropertyFilter propertyFilter,
                             v8::IndexFilter indexFilter,
                             v8::KeyConversionMode keyConversion,
                             _Out_ JsValueRef& arrayObjRef) {
  auto func = [](JsExecutionStateRef state,
                 JsObjectRef object,
                 v8::KeyCollectionMode mode,
                 v8::PropertyFilter propertyFilter,
                 v8::IndexFilter indexFilter,
                 v8::KeyConversionMode keyConversion) -> JsValueRef {
    GCVector<JsValueRef> propertyNamesSoFar;
    GCUnorderedSet<JsValueRef> propertyNamesSet;

    for (JsObjectRef p = object; p; p = p->getPrototypeObject(state).value()) {
      JsObjectRef curObject = p;
      curObject->enumerateObjectOwnProperies(
          state,
          [curObject,
           &propertyNamesSoFar,
           &propertyNamesSet,
           mode,
           propertyFilter,
           indexFilter,
           keyConversion](JsExecutionStateRef state,
                          JsValueRef propertyName,
                          bool isWritable,
                          bool isEnumerable,
                          bool isConfigurable) -> bool {
            bool onlyWritable =
                propertyFilter & v8::PropertyFilter::ONLY_WRITABLE;
            bool onlyEnumerable =
                propertyFilter & v8::PropertyFilter::ONLY_ENUMERABLE;
            bool onlyConfigurable =
                propertyFilter & v8::PropertyFilter::ONLY_CONFIGURABLE;
            bool skipStrings =
                propertyFilter & v8::PropertyFilter::SKIP_STRINGS;
            bool skipSymbols =
                propertyFilter & v8::PropertyFilter::SKIP_SYMBOLS;

            if (!isWritable && onlyWritable) {
              return false;
            }
            if (!isEnumerable && onlyEnumerable) {
              return false;
            }
            if (!isConfigurable && onlyConfigurable) {
              return false;
            }

            if (propertyName->isString() && skipStrings) {
              return false;
            }
            if (propertyName->isSymbol() && skipSymbols) {
              return false;
            }

            auto success = propertyNamesSet.insert(propertyName);
            if (!success.second) {
              return false;
            }

            propertyNamesSoFar.push_back(propertyName);
            return true;
          });

      if (mode == v8::KeyCollectionMode::kOwnOnly) {
        break;
      }
    }

    ValueVectorRef* names = ValueVectorRef::create();
    for (auto name = propertyNamesSoFar.begin();
         name != propertyNamesSoFar.end();
         ++name) {
      names->pushBack(*name);
    }

    return ArrayObjectRef::create(state, names);
  };

  auto result = EvalScript(
      context, func, object, mode, propertyFilter, indexFilter, keyConversion);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);

  arrayObjRef = result.result;
  return JsNoError;
}

JsErrorCode GetObjectConstructor(JsContextRef context,
                                 JsObjectRef objectRef,
                                 _Out_ JsValueRef& constructorRef) {
  if (GetProperty(
          context, objectRef, CachedStringId::constructor, constructorRef) !=
      JsNoError) {
    return JsErrorScriptExecution;
  }
  return JsNoError;
}

JsErrorCode ConstructObject(JsContextRef context,
                            JsValueRef constructor,
                            _Out_ JsObjectRef& object) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsValueRef constructor) -> JsValueRef {
        JsValueRef args[] = {};
        auto result = constructor->construct(state, 0, args);
        return result;
      },
      constructor);
  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  object = result.result->asObject();
  return JsNoError;
}

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
                               _Out_ bool& result) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(propertyName != JS_INVALID_REFERENCE);

  auto evalResult = EvalScript(
      context,
      [](JsExecutionStateRef state,
         JsObjectRef object,
         JsValueRef propertyName,  // NOLINT
         bool isWritable,
         bool isEnumerable,
         bool isConfigurable,
         JsValueRef propertyValue,
         NativeFunctionPointer functionPropertyValue,
         JsValueRef getter,
         JsValueRef setter) -> JsValueRef {
        bool result = false;
        // check if accessors (getter or setter) are given
        if (getter != JS_INVALID_REFERENCE || setter != JS_INVALID_REFERENCE) {
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
          if (value == JS_INVALID_REFERENCE) {
            NESCARGOT_ASSERT(functionPropertyValue != JS_INVALID_REFERENCE);

            auto atomicstr = AtomicStringRef::create(state->context(),
                                                     StringRef::emptyString());

            value = CreateJsValue(FunctionObjectRef::createBuiltinFunction(
                state,
                FunctionObjectRef::NativeFunctionInfo(
                    atomicstr, functionPropertyValue, 0, false, false)));
          }

          result = object->defineDataProperty(
              state, key, value, isWritable, isEnumerable, isConfigurable);
        }
        return CreateJsValue(result);
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

  VERIFY_EVAL_RESULT(evalResult, JsErrorScriptExecution);
  result = evalResult.result->asBoolean();
  return JsNoError;
}

JsErrorCode DefineOwnProperty(JsContextRef context,
                              JsObjectRef object,
                              JsValueRef propertyName,
                              JsValueRef descriptor) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(propertyName != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(descriptor != JS_INVALID_REFERENCE);

  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef propertyName,
                    JsValueRef descriptor) -> JsValueRef {
                   object->defineOwnProperty(state, propertyName, descriptor);
                   return ValueRef::createUndefined();
                 },
                 object,
                 propertyName,
                 descriptor);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  return JsNoError;
}

JsErrorCode GetOwnPropertyDescriptor(JsContextRef context,
                                     JsObjectRef object,
                                     JsValueRef propertyName,
                                     _Out_ JsValueRef& descriptor) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(propertyName != JS_INVALID_REFERENCE);

  auto result =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef propertyName) -> JsValueRef {
                   return object->getOwnPropertyDescriptor(state, propertyName);
                 },
                 object,
                 propertyName);

  VERIFY_EVAL_RESULT_AND_THROW_EXCEPTION(result, JsErrorScriptExecution);
  descriptor = result.result;
  return JsNoError;
}

JsErrorCode GetOwnProperty(JsContextRef context,
                           JsObjectRef object,
                           JsValueRef propertyName,
                           _Out_ JsValueRef& value) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsObjectRef object, JsValueRef propertyName)
          -> JsValueRef { return object->getOwnProperty(state, propertyName); },
      object,
      propertyName);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  value = result.result;
  return JsNoError;
}

JsValueRef GetCachedJsValueStringIfExist(JsValueRef name) {
  JsStringRef cachedString = IsolateShim::GetCurrent()->Cached()->Strings(name);
  if (cachedString != JS_INVALID_REFERENCE) {
    return cachedString;
  }

  return name;
}

JsStringRef CreateJsStringFromASCII(const char* str) {
  return StringRef::createFromASCII(str, strlen(str));
}

JsValueRef GetCachedJsValue(CachedStringId id) {
  return IsolateShim::GetCurrent()->Cached()->Strings(id);
}

JsValueRef GetCachedJsValue(CachedSymbolId id) {
  return IsolateShim::GetCurrent()->Cached()->Symbols(id);
}

JsStringRef CreateJsString(const char* str, size_t len) {
  return StringRef::createFromUTF8(str, len);
}

JsStringRef CreateJsStringFromASCII(const char* str, size_t len) {
  return StringRef::createFromASCII(str, len);
}

JsStringRef CreateJsStringFromUTF8(const char* str, size_t len) {
  return StringRef::createFromUTF8(str, len);
}

JsStringRef CreateJsStringFromUTF16(const char16_t* str, size_t len) {
  return StringRef::createFromUTF16(str, len);
}

JsStringRef CreateJsStringFromLatin1(const unsigned char* str, size_t len) {
  return StringRef::createFromLatin1(str, len);
}

JsStringRef CreateJsEmptyString() {
  return StringRef::emptyString();
}

JsStringRef CreateJsStringExternalFromUTF16(const char16_t* str, size_t len) {
  return StringRef::createExternalFromUTF16(str, len);
}

JsStringRef CreateJsStringExternalFromLatin1(const unsigned char* str,
                                             size_t len) {
  return StringRef::createExternalFromLatin1(str, len);
}

JsAtomicStringRef CreateJsEmptyAtomicString() {
  return AtomicStringRef::emptyAtomicString();
}

void SetExtraData(JsObjectRef object, void* data) {
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  object->setExtraData(data);
}

JsRef GetExtraData(JsObjectRef object) {
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  return object->extraData();
}

JsErrorCode CallJsFunction(JsContextRef context,
                           JsValueRef thisValue,
                           JsValueRef fn,
                           JsValueRef* argv,
                           size_t argc,
                           _Out_ JsValueRef* result) {
  if (fn->isCallable() == false) {
    return JsErrorScriptExecution;
  }

  auto sbresult =
      EvalScript(context,
                 [](JsExecutionStateRef state,
                    JsValueRef thisValue,
                    JsValueRef fn,
                    JsValueRef* argv,
                    size_t argc) -> JsValueRef {
                   return fn->asObject()->call(state, thisValue, argc, argv);
                 },
                 thisValue,
                 fn,
                 argv,
                 argc);

  if (sbresult.error.hasValue()) {
    NESCARGOT_LOG_ERROR("Failed to run function object!\n");
    LoggingJSErrorInfo(sbresult);
    return JsErrorScriptExecution;

  } else {
    if (result) {
      *result = sbresult.result;
    }
  }

  return JsNoError;
}

JsErrorCode ParseScript(JsContextRef contextRef,
                        JsStringRef sourceRef,
                        JsStringRef filenameRef,
                        _Out_ JsScriptRef& scriptRef) {
  auto sbresult = EvalScript(
      contextRef,
      [](JsExecutionStateRef state,
         JsStringRef sourceRef,
         JsStringRef filenameRef,
         JsScriptRef* scriptRef) -> JsValueRef {
        *scriptRef = state->context()
                         ->scriptParser()
                         ->initializeScript(sourceRef, filenameRef)
                         .fetchScriptThrowsExceptionIfParseError(state);
        return JsUndefined();
      },
      sourceRef,
      filenameRef,
      &scriptRef);

  if (!sbresult.isSuccessful()) {
    NESCARGOT_LOG_ERROR(
        "Script Parser Error: %s\n",
        sbresult.resultOrErrorToString(contextRef)->toStdUTF8String().c_str());
    Evaluator::StackTraceData data;
    data.src = filenameRef;
    data.sourceCode = sbresult.resultOrErrorToString(contextRef);
    // TODO: Input accurate error location
    data.loc.column = 1;
    data.loc.index = 0;
    data.loc.line = 0;
    EscargotShim::IsolateShim::GetCurrent()->GetScriptException()->SetException(
        sbresult, &data);
    return JsErrorScriptCompile;
  }
  return JsNoError;
}

void printToConsole(const char* format, JsStringRef stringObj) {
  auto strData = stringObj->stringBufferAccessData();
  if (strData.has8BitContent) {
    NESCARGOT_CONSOLE_LOG(format, strData.length, strData.buffer);
  } else {
    std::string str = stringObj->toStdUTF8String();
    NESCARGOT_CONSOLE_LOG(format, str.length(), str.data());
  }
}

void PrintJsValue(JsValueRef value, int depsSize) {
  auto contextRef = IsolateShim::GetCurrent()->currentContext()->contextRef();
  EvalScript(
      contextRef,
      [](JsExecutionStateRef state,
         JsValueRef value,
         int depsSize) -> JsValueRef {
        if (value->isFunctionObject()) {
          NESCARGOT_CONSOLE_LOG("[Function");
          auto object = value->asFunctionObject();
          auto nameValue = CreateJsValueString("name");
          if (object->hasOwnProperty(state, nameValue)) {
            auto nameObject = object->get(state, nameValue);

            if (nameObject->isString()) {
              printToConsole(": %.*s", nameObject->asString());
            } else {
              NESCARGOT_CONSOLE_LOG(": unknown");
            }
          }
          NESCARGOT_CONSOLE_LOG("]");

          auto prototypeValue = CreateJsValueString("prototype");
          if (object->hasOwnProperty(state, prototypeValue)) {
            auto prototype = object->get(state, prototypeValue);
            if (prototype->isObject()) {
              auto keys = prototype->asObject()->ownPropertyKeys(state);
              NESCARGOT_CONSOLE_LOG(" prototype: { ");
              for (size_t i = 0; i < keys->size(); i++) {
                auto key = keys->at(i);
                if (key->isString()) {
                  printToConsole("%.*s, ", key->asString());
                }
              }
              NESCARGOT_CONSOLE_LOG("}");
            }
          }

          auto keys = object->ownPropertyKeys(state);
          if (keys->size() != 0) {
            NESCARGOT_CONSOLE_LOG(" {\n");
            for (size_t i = 0; i < keys->size(); i++) {
              auto key = keys->at(i);
              if (key->isString() && object->hasOwnProperty(state, key)) {
                JsValueRef objectValue = object->getOwnProperty(state, key);

                std::string indent = "";
                for (int i = 0; i < depsSize; i++) {
                  indent += " ";
                }
                indent += "%.*s: ";
                printToConsole(indent.data(), key->asString());

                if (depsSize < 1) {
                  PrintJsValue(objectValue, depsSize + 1);
                } else {
                  NESCARGOT_CONSOLE_LOG("...");
                }
                NESCARGOT_CONSOLE_LOG(",\n");
              }
            }
            NESCARGOT_CONSOLE_LOG("%*s}", depsSize, "");
          } else {
            NESCARGOT_CONSOLE_LOG("\n");
          }
        } else if (value->isObject()) {
          auto object = value->asObject();
          auto keys = object->ownPropertyKeys(state);

          NESCARGOT_CONSOLE_LOG("[Object] {\n");
          auto prototype = object->getPrototypeObject(state);
          if (prototype.hasValue()) {
            auto prototypeObject = prototype.get();
            auto prototypeKeys = prototypeObject->ownPropertyKeys(state);
            NESCARGOT_CONSOLE_LOG("%*s__proto__: {", depsSize, "");
            for (size_t i = 0; i < prototypeKeys->size(); i++) {
              auto prototypeKey = prototypeKeys->at(i);
              if (prototypeKey->isString() &&
                  prototypeObject->hasOwnProperty(state, prototypeKey)) {
                printToConsole("%.*s, ", prototypeKey->asString());
              }
            }
            NESCARGOT_CONSOLE_LOG("}\n");
          }

          for (size_t i = 0; i < keys->size(); i++) {
            auto key = keys->at(i);
            if (key->isString() && object->hasOwnProperty(state, key)) {
              JsValueRef objectValue = object->getOwnProperty(state, key);

              std::string indent = "";
              for (int i = 0; i < depsSize; i++) {
                indent += " ";
              }
              indent += "%.*s: ";
              printToConsole(indent.data(), key->asString());

              if (depsSize < 1) {
                PrintJsValue(objectValue, depsSize + 1);
              } else {
                NESCARGOT_CONSOLE_LOG("...");
              }

              NESCARGOT_CONSOLE_LOG(",\n");
            }
          }
          NESCARGOT_CONSOLE_LOG("%*s}", depsSize, "");
        } else {
          printToConsole("%.*s", value->toString(state));
        }
        if (depsSize == 0) {
          NESCARGOT_CONSOLE_LOG("\n");
        }
        return JsUndefined();
      },
      value,
      depsSize);
}  // namespace EscargotShim

static void PrintConsole(ConsoleAPIType type, size_t argc, JsValueRef* argv) {
  std::string tag;
  if (type == ConsoleAPIType::kLog) {
    tag = "[console.log]";
  }
  NESCARGOT_CONSOLE_LOG(COLOR_BLUE "%s " COLOR_RESET, tag.c_str());

  for (size_t i = 0; i < argc; i++) {
    PrintJsValue(argv[i]);
  }
}

static ValueRef* _logConsoleFunction(JsExecutionStateRef state,
                                     JsValueRef thisValue,
                                     size_t argc,
                                     JsValueRef* argv,
                                     bool isNewExpression) {
  PrintConsole(ConsoleAPIType::kLog, argc, argv);
  return ValueRef::createUndefined();
}

void CreateConsoleObject(JsContextRef context) {
  auto globalObject = context->globalObject();
  JsObjectRef console = JS_INVALID_REFERENCE;
  CreateJsObject(context, console);

#define CONSOLE_APIS(F) F(log)

#define DECLARE_CONSOLE_APIS(name)                                             \
  JsFunctionRef func = JS_INVALID_REFERENCE;                                   \
  NativeFunctionInfo info(AtomicStringRef::create(context, #name),             \
                          _##name##ConsoleFunction,                            \
                          1,                                                   \
                          true,                                                \
                          false);                                              \
  CreateBuiltinFunction(context, info, func);                                  \
  bool result = false;                                                         \
  DefineDataProperty(context,                                                  \
                     console,                                                  \
                     GetCachedJsValue(CachedStringId::name),                   \
                     true,                                                     \
                     true,                                                     \
                     true,                                                     \
                     func,                                                     \
                     JS_INVALID_REFERENCE,                                     \
                     JS_INVALID_REFERENCE,                                     \
                     JS_INVALID_REFERENCE,                                     \
                     result);                                                  \
  NESCARGOT_ASSERT(result);
  CONSOLE_APIS(DECLARE_CONSOLE_APIS)
#undef DECLARE_CONSOLE_APIS
#undef CONSOLE_APIS

  DefineDataProperty(context,
                     globalObject,
                     GetCachedJsValue(CachedStringId::console),
                     true,
                     false,
                     true,
                     console,
                     JS_INVALID_REFERENCE,
                     JS_INVALID_REFERENCE,
                     JS_INVALID_REFERENCE,
                     result);
  NESCARGOT_ASSERT(result);
}

void LoggingJSErrorInfo(const EvaluatorResult& sbResult) {
#if defined(DEBUG)
  JsContextRef context =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  NESCARGOT_LOG_ERROR(
      "[Internal JS]Uncaught %s\n",
      sbResult.resultOrErrorToString(context)->toStdUTF8String().data());
  for (size_t i = 0; i < sbResult.stackTraceData.size(); i++) {
    NESCARGOT_LOG_ERROR(
        "at %s(%d:%d)\n",
        sbResult.stackTraceData[i].src->toStdUTF8String().data(),
        (int)sbResult.stackTraceData[i].loc.line,
        (int)sbResult.stackTraceData[i].loc.column);

    Escargot::StringRef* src = sbResult.stackTraceData[i].sourceCode;
    if (src->length()) {
      const size_t preLineMax = 40;
      const size_t afterLineMax = 40;

      size_t preLineSoFar = 0;
      size_t afterLineSoFar = 0;

      size_t start = sbResult.stackTraceData[i].loc.index;
      int64_t idx = (int64_t)start;
      while (start - idx < preLineMax) {
        if (idx == 0) {
          break;
        }
        if (src->charAt((size_t)idx) == '\r' ||
            src->charAt((size_t)idx) == '\n') {
          idx++;
          break;
        }
        idx--;
      }
      preLineSoFar = idx;

      idx = start;
      while (idx - start < afterLineMax) {
        if ((size_t)idx == src->length() - 1) {
          break;
        }
        if (src->charAt((size_t)idx) == '\r' ||
            src->charAt((size_t)idx) == '\n') {
          break;
        }
        idx++;
      }
      afterLineSoFar = idx;

      if (preLineSoFar <= afterLineSoFar && preLineSoFar <= src->length() &&
          afterLineSoFar <= src->length()) {
        auto subSrc = src->substring(preLineSoFar, afterLineSoFar);
        NESCARGOT_LOG_INFO("%s\n", subSrc->toStdUTF8String().data());
        std::string sourceCodePosition;
        for (size_t i = preLineSoFar; i < start; i++) {
          sourceCodePosition += " ";
        }
        sourceCodePosition += "^\n";
        NESCARGOT_LOG_INFO("%s", sourceCodePosition.data());
      }
    }
  }
  NESCARGOT_ASSERT(false);
#endif
}

bool HasPrivate(JsContextRef context, JsObjectRef object, JsValueRef key) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(key != JS_INVALID_REFERENCE);

  JsValueRef hiddenValuesRef;
  if (GetProperty(context,
                  object,
                  GetCachedJsValue(CachedSymbolId::__hiddenvalues__),
                  hiddenValuesRef) != JsNoError) {
    return false;
  }

  if (hiddenValuesRef->isUndefined()) {
    return false;
  }

  JsValueRef result;
  if (GetProperty(context, hiddenValuesRef->asObject(), key, result) !=
      JsNoError) {
    return false;
  }

  if (result->isUndefined()) {
    return false;
  }

  return true;
}

JsErrorCode GetPrivate(JsContextRef context,
                       JsObjectRef object,
                       JsValueRef key,
                       JsValueRef& result) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(key != JS_INVALID_REFERENCE);

  JsValueRef hiddenValuesRef;

  VERIFY_JS_ERROR(
      GetProperty(context,
                  object,
                  GetCachedJsValue(CachedSymbolId::__hiddenvalues__),
                  hiddenValuesRef));

  if (hiddenValuesRef->isUndefined()) {
    result = JsUndefined();
    return JsNoError;
  }

  VERIFY_JS_ERROR(
      GetProperty(context, hiddenValuesRef->asObject(), key, result));

  return JsNoError;
}

JsErrorCode SetPrivate(JsContextRef context,
                       JsObjectRef object,
                       JsValueRef key,
                       JsValueRef value) {
  NESCARGOT_ASSERT(context != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(object != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(key != JS_INVALID_REFERENCE);
  NESCARGOT_ASSERT(value != JS_INVALID_REFERENCE);

  JsValueRef hiddenValuesRef = JS_INVALID_REFERENCE;
  JsObjectRef hiddenValuesTable = JS_INVALID_REFERENCE;

  JsValueRef hiddenValuesIdRef =
      GetCachedJsValue(CachedSymbolId::__hiddenvalues__);

  VERIFY_JS_ERROR(
      GetProperty(context, object, hiddenValuesIdRef, hiddenValuesRef));

  // if '__hiddenvalues__' is not defined on object, define it
  if (hiddenValuesRef->isUndefined()) {
    VERIFY_JS_ERROR(CreateJsObject(context, hiddenValuesTable));
    bool result = false;
    VERIFY_JS_ERROR(DefineDataProperty(context,
                                       object,
                                       hiddenValuesIdRef,
                                       false,
                                       false,
                                       false,
                                       hiddenValuesTable,
                                       JS_INVALID_REFERENCE,
                                       JS_INVALID_REFERENCE,
                                       JS_INVALID_REFERENCE,
                                       result));
    if (result == false) {
      return JsErrorScriptExecution;
    }
  } else {
    hiddenValuesTable = hiddenValuesRef->asObject();
  }

  bool result = false;
  VERIFY_JS_ERROR(SetProperty(context, hiddenValuesTable, key, value, result));

  return result ? JsNoError : JsErrorScriptExecution;
}

JsErrorCode RemoveFromHiddenClassChain(JsContextRef context,
                                       JsObjectRef object) {
  auto result = EvalScript(
      context,
      [](JsExecutionStateRef state, JsObjectRef object) -> JsValueRef {
        object->removeFromHiddenClassChain(state);
        return JsUndefined();
      },
      object);

  VERIFY_EVAL_RESULT(result, JsErrorScriptExecution);
  return JsNoError;
}

void addToPersistentStorage(v8::Isolate* isolate, void* jsObject) {
  // FIXME: cleanup the header and then isolate should not be null
  if (isolate == nullptr) {
    IsolateShim::GetCurrent()->addToPersistentStorage(jsObject);
    return;
  }

  if (isolate && jsObject) {
    IsolateShim::ToIsolateShim(isolate)->addToPersistentStorage(jsObject);
  }
}

// FIXME: get an isolate as a parameter
void removeFromPersistentStorage(void* jsObject) {
  if (IsolateShim::GetCurrent() && jsObject) {
    IsolateShim::GetCurrent()->removeFromPersistentStorage(jsObject);
  }
}

void addToLocalScope(v8::Isolate* isolate, void* jsObject) {
  NESCARGOT_ASSERT(isolate == v8::HandleScope::current()->GetIsolate());
  v8::HandleScope::current()->addToLocalScope(jsObject);
}

JsValueRef asJsValueRef(const v8::Value* value) {
  NESCARGOT_ASSERT(value);
  JsValueRef val = reinterpret_cast<JsValueRef>(
      const_cast<typename std::remove_const<v8::Value>::type*>(value));
  NESCARGOT_ASSERT(val->isObject() || val->isBoolean() || val->isNumber() ||
                   val->isNull() || val->isUndefined() || val->isUInt32() ||
                   val->isString() || val->isSymbol());
  return val;
}

JsObjectTemplateRef asJsObjectTemplateRef(const v8::Data* data) {
  JsObjectTemplateRef val = reinterpret_cast<JsObjectTemplateRef>(
      const_cast<typename std::remove_const<v8::Data>::type*>(data));
  NESCARGOT_ASSERT(val->isObjectTemplate());
  return val;
}

JsFunctionTemplateRef asJsFunctionTemplateRef(const v8::Data* data) {
  JsFunctionTemplateRef val = reinterpret_cast<JsFunctionTemplateRef>(
      const_cast<typename std::remove_const<v8::Data>::type*>(data));
  NESCARGOT_ASSERT(val->isFunctionTemplate());
  return val;
}

}  // namespace EscargotShim
