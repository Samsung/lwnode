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

#if defined(HOST_TIZEN)
#include "TizenDeviceAPILoaderForEscargot.h"
#endif

#include "escargotisolateshim.h"
#include "jsutils.h"

using namespace Escargot;

namespace EscargotShim {

ContextShim* ContextShim::ToContextShim(v8::Context* context) {
  return reinterpret_cast<ContextShim*>(context);
}

v8::Context* ContextShim::asContext() {
  return reinterpret_cast<v8::Context*>(this);
}

static JsValueRef GCNativeFunction(JsExecutionStateRef state,
                                   JsValueRef thisValue,
                                   size_t argc,
                                   JsValueRef* argv,
                                   bool isNewExpression) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
  return JsUndefined();
}

ContextShim::ContextShim(EscargotShim::IsolateShim* isolateShim,
                         bool exposeGC,
                         JsObjectRef globalObjectTemplateInstance)
    : m_globalObjectTemplateInstance(globalObjectTemplateInstance) {
  NESCARGOT_ASSERT(isolateShim);
  m_context = CreateJsContext(isolateShim->vmInstanceRef());
  NESCARGOT_ASSERT(m_context);
  m_isolateShim = isolateShim;

  CreateConsoleObject(m_context);
  initializeBuiltIns();
  executeEscargotShimJS();
  checkConfigGlobalObjectTemplate();

  if (exposeGC) {
    JsFunctionRef gcFunction = JS_INVALID_REFERENCE;
    NativeFunctionInfo nativeFunctionInfo(
        AtomicStringRef::emptyAtomicString(), GCNativeFunction, 0, true, false);
    JsErrorCode error = JsNoError;
    error = CreateJsFunction(m_context, nativeFunctionInfo, gcFunction);
    NESCARGOT_ASSERT(error == JsNoError);
    bool result = false;
    error = SetProperty(m_context,
                        m_context->globalObject(),
                        CachedStringId::gc,
                        gcFunction,
                        result);
    NESCARGOT_ASSERT(error == JsNoError);
    NESCARGOT_ASSERT(result);
  }

#if defined(HOST_TIZEN)
  m_tizenDeviceManager = DeviceAPI::initialize(m_context);
#endif
}

void* ContextShim::GetAlignedPointerFromEmbedderData(int index) {
  if (index >= 0 && static_cast<std::vector<void*>::size_type>(index) <
                        m_embedderData.size()) {
    return m_embedderData.at(index);
  }
  return nullptr;
}

void ContextShim::SetAlignedPointerInEmbedderData(int index, void* value) {
  if (index < 0) {
    return;
  }

  try {
    auto minSize = static_cast<std::vector<void*>::size_type>(index) + 1;
    if (m_embedderData.size() < minSize) {
      m_embedderData.resize(minSize);
    }
    m_embedderData[index] = value;
  } catch (const std::exception&) {
  }
}

bool ContextShim::executeEscargotShimJS() {
  JsValueRef url = CreateJsValueString("escargot_shim.js");
  auto scriptBuffer = IsolateShim::GetEscargotShimJsArrayBuffer(m_context)
                          ->asObject()
                          ->asArrayBufferObject();
  auto rawStringData = (const char*)(scriptBuffer->rawBuffer());
  auto scriptString =
      CreateJsStringFromASCII(rawStringData, scriptBuffer->byteLength());

  auto sbresult = EvalScript(
      m_context,
      [](JsExecutionStateRef state,
         JsStringRef scriptString,
         JsValueRef url) -> JsValueRef {
        auto result = state->context()->scriptParser()->initializeScript(
            scriptString, url->asString());
        if (!result.isSuccessful()) {
          state->throwException(ErrorObjectRef::create(
              state, result.parseErrorCode, result.parseErrorMessage));
        }
        return result.script.get()->execute(state);
      },
      scriptString,
      url);

  if (sbresult.error.hasValue()) {
    NESCARGOT_LOG_ERROR(
        "parseError (escargot_shim.js): %s \n",
        sbresult.resultOrErrorToString(m_context)->toStdUTF8String().c_str());

    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
    LoggingJSErrorInfo(sbresult);
    return false;
  }

  // m_keepAliveWrapper should be kept
  JsValueRef arguments[] = {
      m_keepAliveWrapper->GetJsValueRef(),
  };

  if (CallJsFunction(m_context,
                     m_context->globalObject(),
                     sbresult.result,
                     arguments,
                     _countof(arguments)) != JsNoError) {
    return false;
  }

  return true;
}

static JsValueRef GetPrototypeOfGlobalProxyFunction(JsExecutionStateRef state,
                                                    JsValueRef thisValue,
                                                    size_t argc,
                                                    JsValueRef* argv,
                                                    bool isNewExpression) {
  // NOTE: V8 Global is actually proxy where the actual global is it's
  // prototype.
  return state->context()->globalObject();
}

bool ContextShim::initializeBuiltIns() {
  m_keepAliveWrapper = new JsKeepAliveWrapper(m_context);

  bool result = false;
  if (DefineDataProperty(m_context,
                         m_context->globalObject(),
                         GetCachedJsValue(CachedSymbolId::__keepalive__),
                         false,
                         false,
                         false,
                         m_keepAliveWrapper->GetJsValueRef(),
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         result) != JsNoError ||
      result == false) {
    return false;
  }

  // Initialize Proxy Of Global

  JsObjectRef proxyHandler = JS_INVALID_REFERENCE;
  if (CreateJsObject(m_context, proxyHandler) != JsNoError) {
    return false;
  }

  if (DefineDataProperty(m_context,
                         proxyHandler,
                         GetCachedJsValue(CachedStringId::getPrototypeOf),
                         true,
                         true,
                         true,
                         JS_INVALID_REFERENCE,
                         GetPrototypeOfGlobalProxyFunction,
                         JS_INVALID_REFERENCE,
                         JS_INVALID_REFERENCE,
                         result) != JsNoError ||
      result == false) {
    return false;
  }

  JsProxyObjectRef proxy = JS_INVALID_REFERENCE;
  if (CreateJsProxy(
          m_context, m_context->globalObject(), proxyHandler, proxy) !=
      JsNoError) {
    return false;
  }
  m_proxyOfGlobal = proxy;
  return true;
}

bool ContextShim::checkConfigGlobalObjectTemplate() {
  if (m_globalObjectTemplateInstance != JS_INVALID_REFERENCE) {
    // Only need to config once. Discard globalObjectTemplateInstance
    JsValueRef newProto = m_globalObjectTemplateInstance;
    // JsRelease(m_globalObjectTemplateInstance, nullptr);
    m_globalObjectTemplateInstance = JS_INVALID_REFERENCE;

    JsValueRef glob, oldProto;
    bool result;
    glob = m_context->globalObject();
    GetPrototype(m_context, glob->asObject(), oldProto);

    SetPrototype(m_context, newProto->asObject(), oldProto, result);
    SetPrototype(m_context, glob->asObject(), newProto, result);

    m_proxyOfGlobal->setExtraData(newProto->asObject()->extraData());
  }

  return true;
}

JsKeepAliveWrapper* ContextShim::GetKeepAliveWrapper() {
  return m_keepAliveWrapper;
}

JsObjectRef ContextShim::GetProxyOfGlobal() {
  return m_proxyOfGlobal;
}

}  // namespace EscargotShim
