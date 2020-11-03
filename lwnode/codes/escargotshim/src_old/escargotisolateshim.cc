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

#include <stdint.h>
#include "escargot_natives.h"
#include "escargotutil.h"

#include "v8utils.h"

using namespace Escargot;

namespace EscargotShim {

class EscargotShimPlatform : public Escargot::PlatformRef {
 public:
  void* onArrayBufferObjectDataBufferMalloc(JsContextRef whereObjectMade,
                                            ArrayBufferObjectRef* obj,
                                            size_t sizeInByte) override {
    // Only the size of the buffer is known
    void* buffer = calloc(sizeInByte, 1);
    NESCARGOT_ASSERT(buffer);
    return buffer;
  }

  void onArrayBufferObjectDataBufferFree(JsContextRef whereObjectMade,
                                         ArrayBufferObjectRef* obj,
                                         void* buffer) override {
    if (!buffer) {
      return;
    }

    v8::ArrayBufferData* data =
        v8::ExternalData::GetExternalData<v8::ArrayBufferData>(obj);
    if (!data ||
        (data && data->m_mode == v8::ArrayBufferCreationMode::kInternalized)) {
      free(buffer);
    }
  }

  virtual void didPromiseJobEnqueued(ContextRef* relatedContext,
                                     PromiseObjectRef* obj) {}
  virtual LoadModuleResult onLoadModule(ContextRef* relatedContext,
                                        ScriptRef* whereRequestFrom,
                                        StringRef* moduleSrc) {
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
    LoadModuleResult result(nullptr);
    return result;
  }
  virtual void didLoadModule(ContextRef* relatedContext,
                             OptionalRef<ScriptRef> whereRequestFrom,
                             ScriptRef* loadedModule) {}

  virtual void hostImportModuleDynamically(ContextRef* relatedContext,
                                           ScriptRef* referrer,
                                           StringRef* src,
                                           PromiseObjectRef* promise) override {
    LoadModuleResult loadedModuleResult =
        onLoadModule(relatedContext, referrer, src);

    Evaluator::EvaluatorResult executionResult = Evaluator::execute(
        relatedContext,
        [](ExecutionStateRef* state,
           LoadModuleResult loadedModuleResult,
           PromiseObjectRef* promise) -> ValueRef* {
          if (loadedModuleResult.script) {
            if (loadedModuleResult.script.value()->isExecuted()) {
              if (loadedModuleResult.script.value()
                      ->wasThereErrorOnModuleEvaluation()) {
                state->throwException(
                    loadedModuleResult.script.value()->moduleEvaluationError());
              }
            } else {
              loadedModuleResult.script.value()->execute(state);
            }
          } else {
            state->throwException(
                ErrorObjectRef::create(state,
                                       loadedModuleResult.errorCode,
                                       loadedModuleResult.errorMessage));
          }
          return loadedModuleResult.script.value()->moduleNamespace(state);
        },
        loadedModuleResult,
        promise);

    Evaluator::execute(relatedContext,
                       [](ExecutionStateRef* state,
                          bool isSuccessful,
                          ValueRef* value,
                          PromiseObjectRef* promise) -> ValueRef* {
                         if (isSuccessful) {
                           promise->fulfill(state, value);
                         } else {
                           promise->reject(state, value);
                         }
                         return ValueRef::createUndefined();
                       },
                       executionResult.isSuccessful(),
                       executionResult.isSuccessful()
                           ? executionResult.result
                           : executionResult.error.value(),
                       promise);
  }
};

IsolateShim* IsolateShim::ToIsolateShim(v8::Isolate* isolate) {
  return reinterpret_cast<IsolateShim*>(isolate);
}

v8::Isolate* IsolateShim::asIsolate() {
  return reinterpret_cast<v8::Isolate*>(this);
}

IsolateShim* IsolateShim::Allocate() {
  return New();
}

// This is separate so that tests can provide a different |isolate|.
void IsolateShim::Initialize(IsolateShim* isolate, const CreateParams& params) {
  // TODO: Init isolate with params here
  isolate->set_array_buffer_allocator(params.array_buffer_allocator);
}

IsolateShim* IsolateShim::New() {
  IsolateShim* isolateShim = new IsolateShim();
  return isolateShim;
}

IsolateShim* IsolateShim::New(const CreateParams& params) {
  IsolateShim* isolateShim = New();
  Initialize(isolateShim, params);
  return isolateShim;
}

// FIXME: Create a mapping {(threadId, Isolate)}
THREAD_LOCAL IsolateShim* IsolateShim::s_currentIsolate = JS_INVALID_REFERENCE;

// FIXME: temporary use static platform instance since it must live until
// VmInstance is destroyed. Use dynamic-allocation once the escargot bug related
// is fixed.
static EscargotShimPlatform s_platform;

IsolateShim::IsolateShim() {
  m_instance = CreateJsVmInstance(&s_platform, false);
  m_atomicValue = new AtomicValue();
  m_scriptException = new ScriptException();
}

IsolateShim::~IsolateShim() {
  m_instance.reset(nullptr);
}

IsolateShim* IsolateShim::GetCurrent() {
  return s_currentIsolate;
}

// Initializes the current thread to run this Isolate.
// Not thread-safe. Multiple threads should not Enter/Exit the same isolate
// at the same time, this should be prevented using external locking.
void IsolateShim::Enter() {
  // NOTE: Escargot does not support multi-threads, hence it does not support
  // thread-specific data. Therefore, only the same thread that created
  // this Isolate calls this Enter() and Exit(). To prevent Enter() and Exit()
  // pair mismatch, we make sure the same number of Enter() and Exit()
  // are called.
  m_localThreadData.push_back(new LocalThreadData(this));
  s_currentIsolate = m_localThreadData.back()->m_isolateShim;
}

// Exits the current thread. The previosuly entered Isolate is restored
// for the thread.
// Not thread-safe. Multiple threads should not Enter/Exit the same isolate
// at the same time, this should be prevented using external locking.
void IsolateShim::Exit() {
  NESCARGOT_ASSERT(!m_localThreadData.empty());
  m_localThreadData.pop_back();

  if (m_localThreadData.empty()) {
    s_currentIsolate = nullptr;
  } else {
    s_currentIsolate = m_localThreadData.back()->m_isolateShim;
  }
}

void IsolateShim::Dispose() {
  clearPointerPersistentMap();
  clearContextMap();
  s_currentIsolate = nullptr;
  m_localThreadData.clear();
  // TODO:  atomic value should have the same life time with Context
  delete m_atomicValue;
  // delete m_scriptException;
}

ContextShim* IsolateShim::newContextShim(
    bool exposeGC, JsObjectRef globalObjectTemplateInstance) {
  auto contextShim =
      new ContextShim(this, exposeGC, globalObjectTemplateInstance);

  m_contextMap.insert(std::make_pair(contextShim->contextRef(), contextShim));

  return contextShim;
}

ContextShim* IsolateShim::currentContext() {
  if (m_contextStack.empty()) {
    return nullptr;
  }
  return m_contextStack.back();
}

ContextShim* IsolateShim::GetContextShimFromJsContext(JsContextRef context) {
  auto iter = m_contextMap.find(context);
  if (iter == m_contextMap.end()) {
    return nullptr;
  } else {
    return iter->second;
  }
}

void IsolateShim::pushContext(ContextShim* context) {
  NESCARGOT_ASSERT(context);
  m_contextStack.push_back(context);
}

void IsolateShim::popContext() {
  NESCARGOT_ASSERT(!m_contextStack.empty());
  m_contextStack.pop_back();
}

bool IsolateShim::AddMessageListener(void* that) {
  m_messageListeners.push_back(that);
  return true;
}

void IsolateShim::RemoveMessageListeners(void* that) {
  m_messageListeners.erase(
      std::remove(m_messageListeners.begin(), m_messageListeners.end(), that),
      m_messageListeners.end());
}

void IsolateShim::pushLocalScopeData() {
  m_localScopeDataStack.push_back(new LocalScopeData());
}

void IsolateShim::popLocalScopeData() {
  NESCARGOT_ASSERT(!m_localScopeDataStack.empty());
  LocalScopeData* currentScopeData = m_localScopeDataStack.back();
  m_localScopeDataStack.pop_back();
  currentScopeData->clear();
}

IsolateShim::LocalScopeData* IsolateShim::currentLocalScopeData() {
  if (m_localScopeDataStack.empty()) {
    return nullptr;
  }
  return m_localScopeDataStack.back();
}

void IsolateShim::addToLocalScopeData(void* jsObject) {
  LocalScopeData* currentScopeData = currentLocalScopeData();
  NESCARGOT_ASSERT(currentScopeData);
  currentScopeData->addGcObjectRef(jsObject);
}

void IsolateShim::addToPersistentStorage(void* jsObject) {
  auto iter = m_persistentMap.find(jsObject);
  if (iter == m_persistentMap.end()) {
    m_persistentMap.insert(std::make_pair(jsObject, 1));
  } else {
    iter->second++;
  }
}

void IsolateShim::removeFromPersistentStorage(void* jsObject) {
  auto iter = m_persistentMap.find(jsObject);
  if (iter != m_persistentMap.end()) {
    if (iter->second == 1) {
      m_persistentMap.erase(iter);
    } else {
      iter->second--;
    }
  }
}

void IsolateShim::clearPointerPersistentMap() {
  m_persistentMap.clear();
}

void IsolateShim::clearContextMap() {
  m_contextMap.clear();
}

// ScriptException

void ScriptException::SetTryCatchStackTop(v8::TryCatch* section) {
  if (section == JS_INVALID_REFERENCE) {
    ClearException();
  }
  m_tryCatchStackTop = section;
}

void ScriptException::SetException(const EvaluatorResult& result,
                                   const Evaluator::StackTraceData* data) {
  if (!result.error.hasValue()) {
    return;
  }

  if (data == nullptr) {
    if (result.stackTraceData.size() == 0) {
      NESCARGOT_LOG_ERROR(
          "Cannot set Exception: There is no stack trace data!\n");
      return;
    }
    data = &result.stackTraceData[0];
  }

  auto contextRef = GetCurrentJsContextRef();
  if (m_exception == JS_INVALID_REFERENCE) {
    if (CreateJsObject(contextRef, m_exception) != JsNoError) {
      return;
    }
    RemoveFromHiddenClassChain(contextRef, m_exception);
  }

  int index = (int)data->loc.index;
  if (index < 0) {
    index = 0;
  }
  int column = (int)data->loc.column;
  if (column < 0) {
    column = 0;
  }
  int line = (int)data->loc.line;
  if (line < 0) {
    line = 0;
  }

  auto url = data->src->toStdUTF8String();
  JsErrorCode error = JsNoError;
  bool setResult = false;
  error = EscargotShim::SetProperty(
      contextRef,
      m_exception,
      CachedStringId::url,
      CreateJsStringFromASCII(url.data(), url.length()),
      setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);
  error = EscargotShim::SetProperty(contextRef,
                                    m_exception,
                                    CachedStringId::line,
                                    CreateJsValue(line),
                                    setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);
  error = EscargotShim::SetProperty(contextRef,
                                    m_exception,
                                    CachedStringId::column,
                                    CreateJsValue(column - 1),
                                    setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);

  error = EscargotShim::SetProperty(
      contextRef,
      m_exception,
      CachedStringId::exception,
      CreateJsValue(result.resultOrErrorToString(contextRef)),
      setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);

  auto messageString =
      result.resultOrErrorToString(contextRef)->toStdUTF8String();
  auto prefix = std::string("Error: ");
  auto messageStart = messageString.find(prefix);
  if (messageStart != std::string::npos &&
      messageString.length() >= prefix.length()) {
    messageString = messageString.substr(messageStart + prefix.length(),
                                         messageString.length() - messageStart);
  }
  error = EscargotShim::SetProperty(
      contextRef,
      m_exception,
      CachedStringId::message,
      CreateJsValue(CreateJsStringFromASCII(messageString.data())),
      setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);

  std::string stack(
      result.resultOrErrorToString(contextRef)->toStdUTF8String());
  stack += '\n';

  auto source = data->sourceCode->toStdUTF8String();

  int startSourceIdx = index - column + 1;
  if (startSourceIdx < 0) {
    startSourceIdx = 0;
  }
  size_t endSourceIdx = source.find_first_of("\n", startSourceIdx);
  if (endSourceIdx != std::string::npos) {
    source = source.substr(startSourceIdx, endSourceIdx - startSourceIdx);
  } else {
    source = "unknown";
  }

  error = EscargotShim::SetProperty(
      contextRef,
      m_exception,
      CachedStringId::source,
      CreateJsStringFromASCII(source.data(), source.length()),
      setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);

  error = EscargotShim::SetProperty(contextRef,
                                    m_exception,
                                    CachedStringId::length,
                                    CreateJsValue(1),
                                    setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);

  stack += source;
  stack += "\n\n";

  for (size_t i = 0; i < result.stackTraceData.size(); i++) {
    char buffer[256];
    std::string functionName;
    if (result.stackTraceData[i].isFunction) {
      functionName = result.stackTraceData[i].functionName->toStdUTF8String();
    }
    snprintf(buffer,
             sizeof(buffer),
             "\tat %s (%s:%d:%d)\n",
             functionName.data(),
             result.stackTraceData[i].src->toStdUTF8String().data(),
             (int)result.stackTraceData[i].loc.line,
             (int)result.stackTraceData[i].loc.column);
    stack += buffer;
  }
  error = EscargotShim::SetProperty(
      contextRef,
      m_exception,
      CachedStringId::stack,
      CreateJsStringFromASCII(stack.data(), stack.size()),
      setResult);
  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(setResult);
}

void ScriptException::ThrowExceptionToStateIfHasError(
    JsExecutionStateRef state) {
  if (m_exception == JS_INVALID_REFERENCE || m_isThrownException) {
    return;
  }
  JsStringRef errorMessage = JS_INVALID_REFERENCE;
  auto messageString =
      IsolateShim::GetCurrent()->Cached()->Strings(CachedStringId::message);
  if (m_exception->hasOwnProperty(state, messageString)) {
    auto errorMessageValue = m_exception->get(state, messageString);
    messageString = errorMessageValue->asString();
  }
  if (errorMessage == JS_INVALID_REFERENCE) {
    errorMessage = CreateJsEmptyString();
  }
  m_exception = JS_INVALID_REFERENCE;
  m_isThrownException = true;
  auto error =
      ErrorObjectRef::create(state, ErrorObjectRef::Code::None, errorMessage);
  state->throwException(error);
}

// AtomicValue

AtomicValue::AtomicValue() {
  m_Undefined = CreateJsUndefined();
  m_Null = CreateJsNull();
  m_True = CreateJsValue(true);
  m_False = CreateJsValue(false);
  m_strings = new StringRegistry();
  m_symbols = new SymbolRegistry();

// register atomic strings
#define DEF(N) m_strings->set(CachedStringId::N, CreateJsStringFromASCII(#N));
#include "escargotproperyid.inc"
#undef DEF

#define DEFSYMBOL(N)                                                           \
  m_symbols->set(CachedSymbolId::N,                                            \
                 SymbolRef::create(CreateJsStringFromASCII(#N)));
#include "escargotproperyid.inc"
#undef DEFSYMBOL
}

void AtomicValue::Dispose() {}

JsStringRef AtomicValue::Strings(JsValueRef value) {
  NESCARGOT_ASSERT(m_strings);

  JsStringRef str = JS_INVALID_REFERENCE;

  if (value->isString()) {
    str = value->asString();
  } else {
    return str;
  }

  JsStringRef cachedStr = JS_INVALID_REFERENCE;
  // NOTE: consider using a map to search
  m_strings->iterate([&str, &cachedStr](CachedStringId const& key,
                                        JsStringRef const& value) -> bool {
    if (str->equals(value)) {
      cachedStr = value;
      return false;
    }
    return true;
  });

  if (cachedStr == JS_INVALID_REFERENCE) {
    return JS_INVALID_REFERENCE;
  }
  return cachedStr;
}

JsValueRef IsolateShim::GetEscargotShimJsArrayBuffer(JsContextRef context) {
  JsArrayBufferRef arrayBuffer = JS_INVALID_REFERENCE;
  auto error = CreateJsArrayBufferObject(
      context,
      static_cast<void*>(const_cast<uint8_t*>(raw_escargot_shim_value)),
      sizeof(raw_escargot_shim_value),
      v8::ArrayBufferCreationMode::kExternalized,
      arrayBuffer);
  NESCARGOT_ASSERT(error == JsNoError);
  return arrayBuffer;
}

}  // namespace EscargotShim
