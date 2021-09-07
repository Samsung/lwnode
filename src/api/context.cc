/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "context.h"
#include "base.h"
#include "es-helper.h"
#include "extra-data.h"
#include "isolate.h"
#include "stack-trace.h"

using namespace Escargot;

namespace EscargotShim {

static void evalJavaScript(ContextRef* context,
                           const char* name,
                           const char* buffer,
                           size_t bufferSize) {
  ScriptParserRef::InitializeScriptResult scriptResult =
      context->scriptParser()->initializeScript(
          StringRef::createExternalFromASCII(buffer, bufferSize),
          StringRef::createFromUTF8(name,
                                    strnlen(name, v8::String::kMaxLength)));
  LWNODE_CHECK_MSG(scriptResult.isSuccessful(),
                   "Cannot parser %s: %s",
                   name,
                   scriptResult.parseErrorMessage->toStdUTF8String().data());

  auto r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      scriptResult.script.get());
  LWNODE_CHECK_MSG(r.isSuccessful(), "Cannot execute %s", name);
}

static bool createGlobals(ContextRef* context) {
#if defined(HOST_TIZEN)
// @todo setup device APIs
#endif
  // Create captureStackTrace and stackTraceLimit
  EvalResult r =
      Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
        auto errorObject = state->context()
                               ->globalObject()
                               ->get(state, StringRef::createFromASCII("Error"))
                               ->asObject();

        errorObject->set(state,
                         StringRef::createFromASCII("captureStackTrace"),
                         StackTrace::createCaptureStackTrace(state));
        errorObject->set(
            state,
            StringRef::createFromASCII("stackTraceLimit"),
            ValueRef::create(
                20));  // TODO: get number from '--stack-trace-limit' options
        return ValueRef::createUndefined();
      });
  LWNODE_CHECK(r.isSuccessful());
  return true;
}

// ContextWrap

ContextWrap::ContextWrap(IsolateWrap* isolate) {
  isolate_ = isolate;

  context_ = ContextRef::create(isolate->vmInstance());

  callSite_ = new CallSite(context_);

  auto globalObjectData = new GlobalObjectData();
  globalObjectData->setInternalFieldCount(
      GlobalObjectData::kInternalFieldCount);
  globalObjectData->setInternalField(GlobalObjectData::kContextWrapSlot, this);
  ObjectRefHelper::setExtraData(context_->globalObject(), globalObjectData);
  createGlobals(context_);

  val_ = context_;
  type_ = Type::Context;
}

ContextWrap* ContextWrap::New(IsolateWrap* isolate) {
  LWNODE_CHECK_NOT_NULL(isolate);
  return new ContextWrap(isolate);
}

ContextWrap* ContextWrap::fromEscargot(Escargot::ContextRef* esContext) {
  auto globalObjectData =
      ObjectRefHelper::getExtraData(esContext->globalObject())
          ->asGlobalObjectData();

  LWNODE_CHECK(globalObjectData->internalFieldCount() ==
               GlobalObjectData::kInternalFieldCount);

  auto lwCreationContext = reinterpret_cast<ContextWrap*>(
      globalObjectData->internalField(GlobalObjectData::kContextWrapSlot));
  return lwCreationContext;
}

void ContextWrap::Enter() {
  isolate_->Enter();
  isolate_->pushContext(this);
}

void ContextWrap::Exit() {
  isolate_->popContext(this);
}

IsolateWrap* ContextWrap::GetIsolate() {
  return isolate_;
}

ObjectRef* ContextWrap::GetExtrasBindingObject() {
  if (bindingObject_ == nullptr) {
    // note: https://nodejs.org/api/tracing.html#tracing_trace_events
    // use a v8 feature, which requries functions, `isTraceCategoryEnabled` and
    // `trace`, exposed.
    auto nativeFunction = [](ExecutionStateRef* state,
                             ValueRef* thisValue,
                             size_t argc,
                             ValueRef** argv,
                             OptionalRef<ObjectRef> newTarget) -> ValueRef* {
      return ValueRef::createUndefined();
    };

    auto functionTemplate = FunctionTemplateRef::create(
        AtomicStringRef::emptyAtomicString(), 0, false, false, nativeFunction);

    auto objectTemplate = ObjectTemplateRef::create();
    auto function = functionTemplate->instantiate(context_);
    auto name1 = StringRef::createFromASCII("isTraceCategoryEnabled");
    auto name2 = StringRef::createFromASCII("trace");
    objectTemplate->set(name1, function, false, true, false);
    objectTemplate->set(name2, function, false, true, false);
    bindingObject_ = objectTemplate->instantiate(context_);
  }
  return bindingObject_;
}

void ContextWrap::setEmbedderData(int index, void* value) {
  if (embedder_data_ == nullptr) {
    embedder_data_ = new EmbedderDataMap();
  }
  LWNODE_DLOG_INFO("set: EmbedderData: idx %d", index);
  embedder_data_->insert(std::make_pair(index, value));
}

void* ContextWrap::getEmbedderData(int index) {
  if (!embedder_data_) {
    return nullptr;
  }

  auto iter = embedder_data_->find(index);
  if (iter != embedder_data_->end()) {
    return VAL(iter->second);
  }
  return nullptr;
}

void ContextWrap::SetEmbedderData(int index, ValueWrap* value) {
  setEmbedderData(index, reinterpret_cast<void*>(value));
}

ValueWrap* ContextWrap::GetEmbedderData(int index) {
  return VAL(getEmbedderData(index));
}

uint32_t ContextWrap::GetNumberOfEmbedderDataFields() {
  int maxIndex = -1;
  for (auto itr = embedder_data_->begin(); itr != embedder_data_->end();
       ++itr) {
    maxIndex = std::max(maxIndex, itr->first);
  }

  return maxIndex + 1;
}

void ContextWrap::SetAlignedPointerInEmbedderData(int index, void* value) {
  setEmbedderData(index, value);
}

void* ContextWrap::GetAlignedPointerFromEmbedderData(int index) {
  return getEmbedderData(index);
}

void ContextWrap::SetSecurityToken(ValueRef* token) {
  security_token_ = token;
}

ValueRef* ContextWrap::GetSecurityToken() {
  if (!security_token_) {
    return ValueRef::createUndefined();
  }
  return security_token_;
}

void ContextWrap::UseDefaultSecurityToken() {
  security_token_ = context_->globalObject();
}

}  // namespace EscargotShim

// namespace EscargotShim
