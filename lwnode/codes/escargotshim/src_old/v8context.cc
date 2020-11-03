// Copyright Microsoft. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "escargotutil.h"
#include "v8.h"

// #include "v8-debug.h"

#include <assert.h>

using namespace EscargotShim;

namespace v8 {

Context::Scope::Scope(Handle<Context> context) : m_context(context) {
  m_context->Enter();
}

Context::Scope::~Scope() {
  m_context->Exit();
}

Local<Object> Context::Global() {
  // V8 Global is actually proxy where the actual global is it's prototype
  // No need to create handle here,  the context will keep it alive

  // TODO: return global object if needed
  auto contextShim = EscargotShim::ContextShim::ToContextShim(this);
  auto proxyofglobal = contextShim->GetProxyOfGlobal();
  return Local<Object>::New(contextShim->isolateShim()->asIsolate(),
                            CreateJsValue(proxyofglobal));
  // return Local<Object>(ContextShim::GetJsContextRef(this)->globalObject());
}

extern bool g_exposeGC;

Local<Context> Context::New(
    Isolate* isolate,
    ExtensionConfiguration* extensions,
    MaybeLocal<ObjectTemplate> global_template,
    MaybeLocal<Value> global_object,
    DeserializeInternalFieldsCallback internal_fields_deserializer,
    MicrotaskQueue* microtask_queue) {
  // TODO: Impl extensions and global_object
  if (extensions || !global_object.IsEmpty()) {
    NESCARGOT_UNIMPLEMENTED("");
  }

  Local<Object> globalObjectFromTemplate;
  if (!global_template.IsEmpty()) {
    globalObjectFromTemplate = global_template.ToLocalChecked()
                                   ->NewInstance(isolate->GetCurrentContext())
                                   .ToLocalChecked();
    if (globalObjectFromTemplate.IsEmpty()) {
      return Local<Context>();
    }
  }

  EscargotShim::IsolateShim* isolateShim =
      EscargotShim::IsolateShim::ToIsolateShim(isolate);
  JsObjectRef globalObject = nullptr;
  if (!globalObjectFromTemplate.IsEmpty()) {
    globalObject = globalObjectFromTemplate->asJsValueRef()->asObject();
  }
  EscargotShim::ContextShim* contextShim =
      isolateShim->newContextShim(g_exposeGC, globalObject);

  return Local<Context>::New(isolate, contextShim);
}

void Context::Enter() {
  ContextShim* context = ContextShim::ToContextShim(this);
  IsolateShim* isolateShim = context->isolateShim();
  NESCARGOT_ASSERT(isolateShim);
  isolateShim->pushContext(context);
}

void Context::Exit() {
  ContextShim* context = ContextShim::ToContextShim(this);
  IsolateShim* isolateShim = context->isolateShim();
  NESCARGOT_ASSERT(isolateShim);
  isolateShim->popContext();
}

Isolate* Context::GetIsolate() {
  ContextShim* context = ContextShim::ToContextShim(this);
  IsolateShim* isolateShim = context->isolateShim();
  NESCARGOT_ASSERT(isolateShim);
  return isolateShim->asIsolate();
}

void* Context::GetAlignedPointerFromEmbedderData(int index) {
  ContextShim* contextShim = ContextShim::ToContextShim(this);
  return contextShim->GetAlignedPointerFromEmbedderData(index);
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  ContextShim* contextShim = ContextShim::ToContextShim(this);
  return contextShim->SetAlignedPointerInEmbedderData(index, value);
}

void Context::SetEmbedderData(int index, Local<Value> value) {
  SetAlignedPointerInEmbedderData(index, *value);
}

Local<Value> Context::GetEmbedderData(int index) {
  ContextShim* contextShim = ContextShim::ToContextShim(this);
  Isolate* isolate = contextShim->isolateShim()->asIsolate();
  return Local<Value>::New(isolate, GetAlignedPointerFromEmbedderData(index));
}

void Context::SetSecurityToken(Handle<Value> token) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

Handle<Value> Context::GetSecurityToken() {
  return Handle<Value>();
}

uint32_t Context::GetNumberOfEmbedderDataFields() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

void Context::AllowCodeGenerationFromStrings(bool allow) {
  NESCARGOT_UNIMPLEMENTED("");
}

Local<Object> Context::GetExtrasBindingObject() {
  // NOTE: Escargot does not have an extra binding object
  JsContextRef context = ContextShim::ToContextShim(this)->contextRef();
  NESCARGOT_ASSERT(context != nullptr);

  JsObjectRef newObject = JS_INVALID_REFERENCE;
  if (CreateJsObject(context, newObject) != JsNoError) {
    return Local<Object>();
  }

  return Local<Object>(newObject);
}

MaybeLocal<Context> Context::FromSnapshot(
    Isolate* isolate,
    size_t context_snapshot_index,
    DeserializeInternalFieldsCallback embedder_fields_deserializer,
    ExtensionConfiguration* extensions,
    MaybeLocal<Value> global_object,
    MicrotaskQueue* microtask_queue) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Context>();
}

}  // namespace v8
