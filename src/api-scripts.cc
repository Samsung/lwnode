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
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto esScript = VAL(this)->script();
  auto esContext = lwIsolate->GetCurrentContext()->get();

  ScriptParserRef* parser = esContext->scriptParser();
  ScriptParserRef::InitializeScriptResult result =
      parser->initializeScript(esScript->sourceCode(), esScript->src(), false);

  // @note UnboundScript is already once successfully compiled.
  LWNODE_CHECK(result.isSuccessful());

  return Utils::NewLocal<Script>(lwIsolate->toV8(), result.script.get());
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
  auto lwContext = lwIsolate->GetCurrentContext();

  auto esScript = VAL(this)->script();

  // @check: which context is used when doing script->execute?
  // 1) lwContextUsed->get() ?
  // 2) script->context() ?

  auto r = Evaluator::execute(
      esScript->context(),
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      esScript);

  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  return Utils::NewLocal<Value>(lwIsolate->toV8(), r.result);
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
  auto array = Array::New(v8_isolate, length);
  return Utils::NewLocal<PrimitiveArray>(v8_isolate, CVAL(*array)->value());
}

int PrimitiveArray::Length() const {
  auto v8Isolate = Isolate::GetCurrent();
  auto array = Utils::NewLocal<Array>(v8Isolate, CVAL(this)->value());
  return array->Length();
}

void PrimitiveArray::Set(Isolate* v8_isolate,
                         int index,
                         Local<Primitive> item) {
  auto v8Context = v8_isolate->GetCurrentContext();
  auto array = Utils::NewLocal<Array>(v8_isolate, CVAL(this)->value());
  auto ok = array->Set(v8Context, index, item);
}

Local<Primitive> PrimitiveArray::Get(Isolate* v8_isolate, int index) {
  auto v8Context = v8_isolate->GetCurrentContext();
  auto array = Utils::NewLocal<Array>(v8_isolate, CVAL(this)->value());
  auto val = array->Get(v8Context, index);
  auto r = val.FromMaybe(
      Utils::NewLocal<Value>(v8_isolate, ValueRef::createUndefined()));
  return Utils::NewLocal<Primitive>(v8_isolate, CVAL(*r)->value());
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
  LWNODE_ONCE(LWNODE_UNIMPLEMENT);
  return 0;
}

Maybe<bool> Module::InstantiateModule(Local<Context> context,
                                      Module::ResolveCallback callback) {
  LWNODE_ONCE(LWNODE_UNIMPLEMENT_WORKAROUND);
  return Just(true);
}

MaybeLocal<Value> Module::Evaluate(Local<Context> context) {
  LWNODE_ONCE(LWNODE_UNIMPLEMENT_WORKAROUND);
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());
  return v8::Utils::ToLocal<Value>(lwIsolate->undefined());
}

Local<Module> Module::CreateSyntheticModule(
    Isolate* isolate,
    Local<String> module_name,
    const std::vector<Local<v8::String>>& export_names,
    v8::Module::SyntheticModuleEvaluationSteps evaluation_steps) {
  API_ENTER(isolate, Local<Module>());
  auto esContext = lwIsolate->GetCurrentContext()->get();

  LWNODE_CALL_TRACE(
      "module name: %s\n",
      std::string(*(String::Utf8Value(isolate, module_name))).data());

  ScriptParserRef* parser = esContext->scriptParser();
  ScriptParserRef::InitializeScriptResult result =
      parser->initializeScript(lwIsolate->emptyString()->value()->asString(),
                               CVAL(*module_name)->value()->asString(),
                               true);
  LWNODE_CHECK_MSG(result.isSuccessful(), "Cannot create Synthetic Module!");
  // TODO: save the result value to the ModuleWrap
  return v8::Utils::NewLocal<Module>(isolate,
                                     ValueWrap::createModule(new ModuleWrap()));
}

Maybe<bool> Module::SetSyntheticModuleExport(Isolate* isolate,
                                             Local<String> export_name,
                                             Local<v8::Value> export_value) {
  LWNODE_ONCE(LWNODE_UNIMPLEMENT_WORKAROUND);
  return Just(true);
}

void Module::SetSyntheticModuleExport(Local<String> export_name,
                                      Local<v8::Value> export_value) {
  LWNODE_RETURN_VOID;
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundInternal(
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

  auto esSource = VAL(*source->source_string)->value()->asString();
  auto esResourceName = StringRef::emptyString();

  if (!source->resource_name.IsEmpty()) {
    esResourceName = VAL(*source->resource_name)->value()->asString();
  }

  ContextRef* esPureContext = ContextRef::create(lwIsolate->vmInstance());
  ScriptParserRef* parser = esPureContext->scriptParser();
  ScriptParserRef::InitializeScriptResult result = parser->initializeScript(
      esSource, esResourceName, source->GetResourceOptions().IsModule());

  if (!result.isSuccessful()) {
    return MaybeLocal<UnboundScript>();
  }

  // wrap the parsed script with the current isolate
  // auto lwValue = ValueWrap::createScript(result.script.get());

  // ExtraData extra(1, lwIsolate);
  // lwValue->setExtra(std::move(extra));
  return Utils::NewLocal<UnboundScript>(v8_isolate, result.script.get());
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* v8_isolate,
    Source* source,
    CompileOptions options,
    NoCacheReason no_cache_reason) {
  return CompileUnboundInternal(v8_isolate, source, options, no_cache_reason);
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
                                           Source* source,
                                           CompileOptions options,
                                           NoCacheReason no_cache_reason) {
  auto isolate = context->GetIsolate();
  auto maybe =
      CompileUnboundInternal(isolate, source, options, no_cache_reason);
  Local<UnboundScript> result;
  if (!maybe.ToLocal(&result)) return MaybeLocal<Script>();
  v8::Context::Scope scope(context);
  return result->BindToCurrentContext();
}

MaybeLocal<Module> ScriptCompiler::CompileModule(
    Isolate* isolate,
    Source* source,
    CompileOptions options,
    NoCacheReason no_cache_reason) {
  LWNODE_CHECK(options == kNoCompileOptions || options == kConsumeCodeCache);
  API_ENTER(isolate, MaybeLocal<Module>());

  LWNODE_CHECK_MSG(source->GetResourceOptions().IsModule(),
                   "Invalid ScriptOrigin: is_module must be true");

  auto maybe =
      CompileUnboundInternal(isolate, source, options, no_cache_reason);
  Local<UnboundScript> unbound;
  if (!maybe.ToLocal(&unbound)) return MaybeLocal<Module>();

  // TODO: save the unbound result to the ModuleWrap
  return v8::Utils::NewLocal<Module>(isolate,
                                     ValueWrap::createModule(new ModuleWrap()));
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
  API_ENTER_WITH_CONTEXT(v8_context, MaybeLocal<Function>());

  LWNODE_DCHECK(options == CompileOptions::kConsumeCodeCache ||
                options == CompileOptions::kEagerCompile ||
                options == CompileOptions::kNoCompileOptions);

  Isolate* isolate = v8_context->GetIsolate();

  if (options == CompileOptions::kConsumeCodeCache) {
    CachedData* cached_data = source->cached_data;
    LWNODE_DCHECK(cached_data->length == 0);

    // @note node_contextify.cc:783 is related.
    cached_data->rejected = true;

    LWNODE_DLOG_INFO("ignored: CachedData (%s)",
                     VAL(*source->resource_name)
                         ->value()
                         ->asString()
                         ->toStdUTF8String()
                         .c_str());
  }

  if (context_extension_count > 0) {
    LWNODE_UNIMPLEMENT;
  }

  auto esContext = VAL(*v8_context)->context()->get();
  auto esSource = VAL(*source->source_string)->value()->asString();

  GCVector<ValueRef*> arguments_list;

  for (size_t i = 0; i < arguments_count; i++) {
    arguments_list.push_back(VAL(*arguments[i])->value());
  }
  arguments_list.push_back(esSource);

  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state,
         FunctionObjectRef* function,
         const size_t argc,
         ValueRef** argv) -> ValueRef* {
        LWNODE_CHECK(function->isConstructible());
        return function->construct(state, argc, argv);
      },
      esContext->globalObject()->function(),
      arguments_list.size(),
      arguments_list.data());

  // note: expand API_HANDLE_EXCEPTION and add the resource name
  if (!r.isSuccessful()) {
    LWNODE_DLOG_ERROR("Evaluate");
    LWNODE_DLOG_RAW("Execute:\n  %s (%s:%d)\nResource:\n  %s\n%s",
                    TRACE_ARGS2,
                    VAL(*source->resource_name)
                        ->value()
                        ->asString()
                        ->toStdUTF8String()
                        .c_str(),
                    EvalResultHelper::getErrorString(
                        lwIsolate->GetCurrentContext()->get(), r)
                        .c_str());

    lwIsolate->ScheduleThrow(r.error.get());
    return MaybeLocal<Function>();
  }

  return Utils::NewLocal<Function>(lwIsolate->toV8(), r.result);
}

void ScriptCompiler::ScriptStreamingTask::Run() {
  LWNODE_RETURN_VOID;
}

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

#ifndef NDEBUG
static size_t s_track_data_size;
#define TRACK_MSG_FMT                                                          \
  "total size of new CachedData: " COLOR_GREEN "%zuB" COLOR_RESET
#endif

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundScript> unbound_script) {
#ifndef NDEBUG
  s_track_data_size += sizeof(CachedData);
  LWNODE_CALL_TRACE(TRACK_MSG_FMT, s_track_data_size);
#endif
  return new CachedData();
}

// static
ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundModuleScript> unbound_module_script) {
#ifndef NDEBUG
  s_track_data_size += sizeof(CachedData);
  LWNODE_CALL_TRACE(TRACK_MSG_FMT, s_track_data_size);
#endif
  return new CachedData();
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCacheForFunction(
    Local<Function> function) {
  LWNODE_CALL_TRACE();
  // @note
  // this is because of CHECK_NOT_NULL in node_native_module.cc:318.
  // if the total size of new CachedData is too much then we may modify
  // the above node source. Before that is confirmed, track the size.
#ifndef NDEBUG
  s_track_data_size += sizeof(CachedData);
  LWNODE_CALL_TRACE(TRACK_MSG_FMT, s_track_data_size);
#endif
  return new CachedData();
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
