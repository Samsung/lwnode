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

#include "api.h"
#include "base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {

// --- S c r i p t s ---

// Internally, UnboundScript is a SharedFunctionInfo, and Script is a
// JSFunction.

ScriptCompiler::CachedData::CachedData(const uint8_t* data_,
                                       int length_,
                                       BufferPolicy buffer_policy_)
    : data(data_),
      length(length_),
      rejected(false),
      buffer_policy(buffer_policy_) {}

ScriptCompiler::CachedData::~CachedData() {}

bool ScriptCompiler::ExternalSourceStream::SetBookmark() {
  LWNODE_RETURN_FALSE;
}

void ScriptCompiler::ExternalSourceStream::ResetToBookmark() {
  LWNODE_RETURN_VOID;
}

// ScriptCompiler::StreamedSource::StreamedSource(ExternalSourceStream* stream,
//                                                Encoding encoding)
//     : StreamedSource(std::unique_ptr<ExternalSourceStream>(stream), encoding)
//     {}

// ScriptCompiler::StreamedSource::StreamedSource(
//     std::unique_ptr<ExternalSourceStream> stream, Encoding encoding)
//     {}

// ScriptCompiler::StreamedSource::~StreamedSource() = default;

Local<Script> UnboundScript::BindToCurrentContext() {
  /*
    @note Unboundscript includes an `isolate` which will be used to
    get the `current context` of the isolate.

    @note Script includes the `current context` obtained from the above
    step, and it will be used on Script::Run.

    @note ValueWrap includes an extra space to carry the gc objects above.
  */

  auto _unboundscript = VAL(this);
  auto _isolateUsed = _unboundscript->getExtra<IsolateWrap>(0).getChecked();
  auto _script = ValueWrap::createScript(_unboundscript->script());

  // add the `current context` into this ValueWrap for Script
  ExtraData extra(1, _isolateUsed->CurrentContext());
  _script->setExtra(std::move(extra));

  return Local<Script>::New(IsolateWrap::currentIsolate()->toV8(), _script);
}

int UnboundScript::GetId() {
  LWNODE_RETURN_0;
}

int UnboundScript::GetLineNumber(int code_pos) {
  LWNODE_RETURN_0;
}

Local<Value> UnboundScript::GetScriptName() {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> UnboundScript::GetSourceURL() {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> UnboundScript::GetSourceMappingURL() {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> Script::Run(Local<Context> context) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());

  auto _value = VAL(this);
  auto _contextUsed = _value->getExtra<ContextWrap>(0).getChecked();

  auto r = Evaluator::execute(
      _contextUsed->get(),
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      _value->script());

  if (!r.isSuccessful()) {
    lwIsolate->scheduleThrow(r.error.get());
    return MaybeLocal<Value>();
  }

  return Local<Value>::New(lwIsolate->toV8(), ValueWrap::createValue(r.result));
}

Local<Value> ScriptOrModule::GetResourceName() {
  LWNODE_RETURN_LOCAL(Value);
}

Local<PrimitiveArray> ScriptOrModule::GetHostDefinedOptions() {
  LWNODE_RETURN_LOCAL(PrimitiveArray);
}

Local<UnboundScript> Script::GetUnboundScript() {
  LWNODE_RETURN_LOCAL(UnboundScript);
}

// static
Local<PrimitiveArray> PrimitiveArray::New(Isolate* v8_isolate, int length) {
  LWNODE_RETURN_LOCAL(PrimitiveArray);
}

int PrimitiveArray::Length() const {
  LWNODE_RETURN_0;
}

void PrimitiveArray::Set(Isolate* v8_isolate,
                         int index,
                         Local<Primitive> item) {
  LWNODE_RETURN_VOID;
}

Local<Primitive> PrimitiveArray::Get(Isolate* v8_isolate, int index) {
  LWNODE_RETURN_LOCAL(Primitive);
}

Module::Status Module::GetStatus() const {
  LWNODE_UNIMPLEMENT;
  return kErrored;
}

Local<Value> Module::GetException() const {
  LWNODE_RETURN_LOCAL(Value);
}

int Module::GetModuleRequestsLength() const {
  LWNODE_RETURN_0;
}

Local<String> Module::GetModuleRequest(int i) const {
  LWNODE_RETURN_LOCAL(String);
}

Location Module::GetModuleRequestLocation(int i) const {
  LWNODE_UNIMPLEMENT;
  return v8::Location(0, 0);
}

Local<Value> Module::GetModuleNamespace() {
  LWNODE_RETURN_LOCAL(Value);
}

Local<UnboundModuleScript> Module::GetUnboundModuleScript() {
  LWNODE_RETURN_LOCAL(UnboundModuleScript);
}

int Module::GetIdentityHash() const {
  LWNODE_RETURN_0;
}

Maybe<bool> Module::InstantiateModule(Local<Context> context,
                                      Module::ResolveCallback callback) {
  LWNODE_RETURN_MAYBE(bool);
}

MaybeLocal<Value> Module::Evaluate(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Module> Module::CreateSyntheticModule(
    Isolate* isolate,
    Local<String> module_name,
    const std::vector<Local<v8::String>>& export_names,
    v8::Module::SyntheticModuleEvaluationSteps evaluation_steps) {
  LWNODE_RETURN_LOCAL(Module);
}

Maybe<bool> Module::SetSyntheticModuleExport(Isolate* isolate,
                                             Local<String> export_name,
                                             Local<v8::Value> export_value) {
  LWNODE_RETURN_MAYBE(bool);
}

void Module::SetSyntheticModuleExport(Local<String> export_name,
                                      Local<v8::Value> export_value) {}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundInternal(
    Isolate* v8_isolate,
    Source* source,
    CompileOptions options,
    NoCacheReason no_cache_reason) {
  LWNODE_RETURN_LOCAL(UnboundScript);
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* v8_isolate,
    Source* source,
    CompileOptions options,
    NoCacheReason no_cache_reason) {
  API_ENTER(v8_isolate, MaybeLocal<UnboundScript>());

  if (options == kConsumeCodeCache) {
    LWNODE_UNIMPLEMENT;
    // @todo @feature
    // CodeCache feature seemingly requires finding a cache, which is the same
    // script compiled, based on the given source string. Escargot doesn't
    // provide such a feature. Do we need to handle it here?
  }

  // @todo @escargot
  // Escargot::NativeCodeBlock associates a Context to get access to
  // AtomicStrings. Why not a VmInstance instead of a Context? Context isn't
  // related to compiling scripts.

  auto __source = VAL(*source->source_string)->value()->asString();
  auto __resource_name = StringRef::emptyString();

  if (!source->resource_name.IsEmpty()) {
    LWNODE_UNIMPLEMENT;
  }

  ScriptParserRef* parser = lwIsolate->scriptParser();
  ScriptParserRef::InitializeScriptResult result =
      parser->initializeScript(__source, __resource_name, false);

  if (!result.isSuccessful()) {
    return MaybeLocal<UnboundScript>();
  }

  // wrap the parsed script with the current isolate
  auto _value = ValueWrap::createScript(result.script.get());
  ExtraData extra(1, lwIsolate);
  _value->setExtra(std::move(extra));

  return Local<UnboundScript>::New(v8_isolate, _value);
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
                                           Source* source,
                                           CompileOptions options,
                                           NoCacheReason no_cache_reason) {
  LWNODE_RETURN_LOCAL(Script);
}

MaybeLocal<Module> ScriptCompiler::CompileModule(
    Isolate* isolate,
    Source* source,
    CompileOptions options,
    NoCacheReason no_cache_reason) {
  LWNODE_RETURN_LOCAL(Module);
}

MaybeLocal<Function> ScriptCompiler::CompileFunctionInContext(
    Local<Context> v8_context,
    Source* source,
    size_t arguments_count,
    Local<String> arguments[],
    size_t context_extension_count,
    Local<Object> context_extensions[],
    CompileOptions options,
    NoCacheReason no_cache_reason,
    Local<ScriptOrModule>* script_or_module_out) {
  LWNODE_RETURN_LOCAL(Function);
}

void ScriptCompiler::ScriptStreamingTask::Run() {}

ScriptCompiler::ScriptStreamingTask* ScriptCompiler::StartStreamingScript(
    Isolate* v8_isolate, StreamedSource* source, CompileOptions options) {
  LWNODE_RETURN_NULLPTR;
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
                                           StreamedSource* v8_source,
                                           Local<String> full_source_string,
                                           const ScriptOrigin& origin) {
  LWNODE_RETURN_LOCAL(Script);
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
  LWNODE_RETURN_0;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundScript> unbound_script) {
  LWNODE_RETURN_NULLPTR;
}

// static
ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundModuleScript> unbound_module_script) {
  LWNODE_RETURN_NULLPTR;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCacheForFunction(
    Local<Function> function) {
  LWNODE_RETURN_NULLPTR;
}

MaybeLocal<Script> Script::Compile(Local<Context> context,
                                   Local<String> source,
                                   ScriptOrigin* origin) {
  if (origin) {
    ScriptCompiler::Source script_source(source, *origin);
    return ScriptCompiler::Compile(context, &script_source);
  }
  ScriptCompiler::Source script_source(source);
  return ScriptCompiler::Compile(context, &script_source);
}

}  // namespace v8
