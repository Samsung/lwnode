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
// --- D e b u g   S u p p o r t ---

// void debug::SetContextId(Local<Context> context, int id) {
//   LWNODE_RETURN_VOID;
// }

// int debug::GetContextId(Local<Context> context) {
//   LWNODE_RETURN_0;
// }

// void debug::SetInspector(Isolate* isolate,
//                          v8_inspector::V8Inspector* inspector){
//     LWNODE_RETURN_VOID;}

// v8_inspector::V8Inspector* debug::GetInspector(Isolate* isolate) {
//   LWNODE_RETURN_NULLPTR;
// }

// void debug::SetBreakOnNextFunctionCall(Isolate* isolate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::ClearBreakOnNextFunctionCall(Isolate*
// isolate){LWNODE_RETURN_VOID;}

// MaybeLocal<Array> debug::GetInternalProperties(Isolate* v8_isolate,
//                                                Local<Value> value) {
//   LWNODE_RETURN_LOCAL(Array);
// }

// bool debug::GetPrivateMembers(Local<Context> context,
//                               Local<Object> value,
//                               std::vector<Local<Value>>* names_out,
//                               std::vector<Local<Value>>* values_out){
//     LWNODE_RETURN_FALSE;}

// Local<Context> debug::GetCreationContext(Local<Object> value) {
//   LWNODE_RETURN_LOCAL(Context);
// }

// void debug::ChangeBreakOnException(Isolate* isolate, ExceptionBreakState
// type) {
//   LWNODE_RETURN_VOID;
// }

// void debug::SetBreakPointsActive(Isolate* v8_isolate, bool is_active) {
//   LWNODE_RETURN_VOID;
// }

// void debug::PrepareStep(Isolate* v8_isolate, StepAction action) {
//   LWNODE_RETURN_VOID;
// }

// void debug::ClearStepping(Isolate* v8_isolate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::BreakRightNow(Isolate* v8_isolate) {
//   i::Isolate* isolate = reinterpret_cast<i::Isolate*>(v8_isolate);
//   ENTER_V8_DO_NOT_USE(isolate);
//   isolate->debug()->HandleDebugBreak(i::kIgnoreIfAllFramesBlackboxed);
// }

// void debug::SetTerminateOnResume(Isolate* v8_isolate) {
//   LWNODE_RETURN_VOID;
// }

// bool debug::AllFramesOnStackAreBlackboxed(Isolate* v8_isolate){
//     LWNODE_RETURN_FALSE;}

// v8::Isolate* debug::Script::GetIsolate() const {LWNODE_RETURN_NULLPTR;}

// ScriptOriginOptions debug::Script::OriginOptions() const {
//   LWNODE_UNIMPLEMENT;
//   return ScriptOriginOptions();
// }

// bool debug::Script::WasCompiled() const {
//   LWNODE_RETURN_FALSE;
// }

// bool debug::Script::IsEmbedded() const {
//   LWNODE_RETURN_FALSE;
// }

// int debug::Script::Id() const {
//   LWNODE_RETURN_0;
// }

// int debug::Script::LineOffset() const {
//   LWNODE_RETURN_0;
// }

// int debug::Script::ColumnOffset() const {LWNODE_RETURN_0;}

// std::vector<int> debug::Script::LineEnds() const {
//   LWNODE_UNIMPLEMENT;
//   return std::vector<int>();
// }

// MaybeLocal<String> debug::Script::Name() const {LWNODE_RETURN_LOCAL(String)}

// MaybeLocal<String> debug::Script::SourceURL() const {
//     LWNODE_RETURN_LOCAL(String)}

// MaybeLocal<String> debug::Script::SourceMappingURL() const {
//     LWNODE_RETURN_LOCAL(String)}

// Maybe<int> debug::Script::ContextId() const {LWNODE_RETURN_MAYBE(int)}

// MaybeLocal<String> debug::Script::Source() const {
//   LWNODE_RETURN_LOCAL(String);
// }

// bool debug::Script::IsWasm() const {
//   LWNODE_RETURN_FALSE;
// }

// bool debug::Script::IsModule() const {
//   LWNODE_RETURN_FALSE;
// }

// bool debug::Script::GetPossibleBreakpoints(
//     const debug::Location& start,
//     const debug::Location& end,
//     bool restrict_to_function,
//     std::vector<debug::BreakLocation>* locations) const {
//   LWNODE_RETURN_FALSE;
// }

// int debug::Script::GetSourceOffset(const debug::Location& location) const {
//     LWNODE_RETURN_0;}

// v8::debug::Location debug::Script::GetSourceLocation(int offset) const {
//   LWNODE_UNIMPLEMENT;
//   return debug::Location(0, 0);
// }

// bool debug::Script::SetScriptSource(v8::Local<v8::String> newSource,
//                                     bool preview,
//                                     debug::LiveEditResult* result) const {
//   LWNODE_RETURN_FALSE;
// }

// bool debug::Script::SetBreakpoint(v8::Local<v8::String> condition,
//                                   debug::Location* location,
//                                   debug::BreakpointId* id) const {
//   LWNODE_RETURN_FALSE;
// }

// bool debug::Script::SetBreakpointOnScriptEntry(BreakpointId* id) const {
//   LWNODE_RETURN_FALSE;
// }

// void debug::Script::RemoveWasmBreakpoint(debug::BreakpointId id) {
//   LWNODE_RETURN_VOID;
// }

// void debug::RemoveBreakpoint(Isolate* v8_isolate,
//                              BreakpointId id){LWNODE_RETURN_VOID;}

// v8::Platform* debug::GetCurrentPlatform(){LWNODE_RETURN_NULLPTR;}

// debug::WasmScript* debug::WasmScript::Cast(debug::Script* script){
//     LWNODE_RETURN_NULLPTR;}

// debug::WasmScript::DebugSymbolsType debug::WasmScript::GetDebugSymbolType()
//     const {
//   LWNODE_UNIMPLEMENT;
//   return debug::WasmScript::DebugSymbolsType::None;
// }

// MemorySpan<const char> debug::WasmScript::ExternalSymbolsURL() const {
//   LWNODE_UNIMPLEMENT;
//   return {nullptr, 0};
// }

// int debug::WasmScript::NumFunctions() const {
//   LWNODE_RETURN_0;
// }

// int debug::WasmScript::NumImportedFunctions() const {LWNODE_RETURN_0;}

// MemorySpan<const uint8_t> debug::WasmScript::Bytecode() const {
//   LWNODE_UNIMPLEMENT;
//   return {nullptr, 0};
// }

// std::pair<int, int> debug::WasmScript::GetFunctionRange(
//     int function_index) const {
//   LWNODE_UNIMPLEMENT;
//   return std::make_pair(0, 0);
// }

// int debug::WasmScript::GetContainingFunction(int byte_offset) const {
//     LWNODE_RETURN_0;}

// uint32_t debug::WasmScript::GetFunctionHash(int function_index) {
//   LWNODE_RETURN_0;
// }

// int debug::WasmScript::CodeOffset() const {LWNODE_RETURN_0;}

// debug::Location::Location(int line_number, int column_number)
//     : line_number_(line_number),
//       column_number_(column_number),
//       is_empty_(false) {}

// debug::Location::Location()
//     : line_number_(v8::Function::kLineOffsetNotFound),
//       column_number_(v8::Function::kLineOffsetNotFound),
//       is_empty_(true) {}

// int debug::Location::GetLineNumber() const {
//   LWNODE_RETURN_0;
// }

// int debug::Location::GetColumnNumber() const {
//   LWNODE_RETURN_0;
// }

// bool debug::Location::IsEmpty() const {
//   LWNODE_RETURN_0;
// }

// void debug::GetLoadedScripts(v8::Isolate* v8_isolate,
//                              PersistentValueVector<debug::Script>& scripts){
//     LWNODE_RETURN_VOID;}

// MaybeLocal<UnboundScript> debug::CompileInspectorScript(Isolate* v8_isolate,
//                                                         Local<String> source)
//                                                         {
//   LWNODE_RETURN_Local(UnboundScript)
// }

// void debug::TierDownAllModulesPerIsolate(Isolate* v8_isolate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::TierUpAllModulesPerIsolate(Isolate* v8_isolate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::SetDebugDelegate(Isolate* v8_isolate,
//                              debug::DebugDelegate* delegate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::SetAsyncEventDelegate(Isolate* v8_isolate,
//                                   debug::AsyncEventDelegate* delegate) {
//   LWNODE_RETURN_VOID;
// }

// void debug::ResetBlackboxedStateCache(Isolate* v8_isolate,
//                                       v8::Local<debug::Script> script) {
//   LWNODE_RETURN_VOID;
// }

// int debug::EstimatedValueSize(Isolate* v8_isolate,
//                               v8::Local<v8::Value> value){LWNODE_RETURN_0;}

v8::MaybeLocal<v8::Array> v8::Object::PreviewEntries(bool* is_key_value) {
  LWNODE_RETURN_LOCAL(Array);
}

// Local<Function> debug::GetBuiltin(Isolate* v8_isolate, Builtin builtin) {
//   LWNODE_RETURN_LOCAL(Function);
// }

// void debug::SetConsoleDelegate(Isolate* v8_isolate,
//                                ConsoleDelegate*
//                                delegate){LWNODE_RETURN_VOID;}

// debug::ConsoleCallArguments::ConsoleCallArguments(
//     const v8::FunctionCallbackInfo<v8::Value>& info)
//     : v8::FunctionCallbackInfo<v8::Value>(
//           nullptr, info.values_, info.length_){LWNODE_UNIMPLEMENT;}

//       debug::ConsoleCallArguments::ConsoleCallArguments(
//           const internal::BuiltinArguments& args)
//     : v8::FunctionCallbackInfo<v8::Value>(
//           nullptr,
//           // Drop the first argument (receiver, i.e. the "console" object).
//           args.length() > 1 ? args.address_of_first_argument() : nullptr,
//           args.length() - 1) {}

// int debug::GetStackFrameId(v8::Local<v8::StackFrame> frame){LWNODE_RETURN_0;}

// v8::Local<v8::StackTrace> debug::GetDetailedStackTrace(
//     Isolate* v8_isolate,
//     v8::Local<v8::Object> v8_error){LWNODE_RETURN_LOCAL(StackTrace)}

// MaybeLocal<debug::Script> debug::GeneratorObject::Script(){
//     LWNODE_RETURN_MAYBE(debug::Script)}

// Local<Function> debug::GeneratorObject::Function(){
//     LWNODE_RETURN_LOCAL(Function);}

// debug::Location debug::GeneratorObject::SuspendedLocation() {
//   LWNODE_UNIMPLEMENT;
//   return debug::Location(0, 0);
// }

// bool debug::GeneratorObject::IsSuspended(){LWNODE_RETURN_FALSE;}

// v8::Local<debug::GeneratorObject> debug::GeneratorObject::Cast(
//     v8::Local<v8::Value> value){LWNODE_RETURN_LOCAL(debug::GeneratorObject);}

// MaybeLocal<v8::Value> debug::EvaluateGlobal(v8::Isolate* isolate,
//                                             v8::Local<v8::String> source,
//                                             EvaluateGlobalMode mode,
//                                             bool repl) {
//   LWNODE_RETURN_LOCAL(Value);
// }

// void debug::QueryObjects(v8::Local<v8::Context> v8_context,
//                          QueryObjectPredicate* predicate,
//                          PersistentValueVector<v8::Object>* objects) {
//   LWNODE_RETURN_VOID;
// }

// void debug::GlobalLexicalScopeNames(
//     v8::Local<v8::Context> v8_context,
//     v8::PersistentValueVector<v8::String>* names) {
//   LWNODE_RETURN_VOID;
// }

// void debug::SetReturnValue(v8::Isolate* v8_isolate,
//                            v8::Local<v8::Value> value){LWNODE_RETURN_VOID;}

// int64_t debug::GetNextRandomInt64(v8::Isolate* v8_isolate) {
//   LWNODE_RETURN_0;
// }

// int debug::GetDebuggingId(v8::Local<v8::Function> function) {
//   LWNODE_RETURN_0;
// }

// bool debug::SetFunctionBreakpoint(v8::Local<v8::Function> function,
//                                   v8::Local<v8::String> condition,
//                                   BreakpointId* id){LWNODE_RETURN_FALSE;}

// debug::PostponeInterruptsScope::PostponeInterruptsScope(v8::Isolate* isolate)
//     : scope_(
//           new
//           i::PostponeInterruptsScope(reinterpret_cast<i::Isolate*>(isolate),
//                                          i::StackGuard::API_INTERRUPT)) {}

// debug::PostponeInterruptsScope::~PostponeInterruptsScope() = default;

// Local<String> CpuProfileNode::GetFunctionName() const {
//   LWNODE_RETURN_LOCAL(String);
// }

// int debug::Coverage::BlockData::StartOffset() const {
//   LWNODE_RETURN_0;
// }
// int debug::Coverage::BlockData::EndOffset() const {
//     LWNODE_RETURN_0;} uint32_t debug::Coverage::BlockData::Count() const {
//   LWNODE_RETURN_0;
// }

// int debug::Coverage::FunctionData::StartOffset() const {
//   LWNODE_RETURN_0;
// }
// int debug::Coverage::FunctionData::EndOffset() const {
//     LWNODE_RETURN_0;} uint32_t debug::Coverage::FunctionData::Count() const {
//     LWNODE_RETURN_0;}

// MaybeLocal<String> debug::Coverage::FunctionData::Name() const {
//     LWNODE_RETURN_LOCAL(String);}

// size_t debug::Coverage::FunctionData::BlockCount() const {
//   LWNODE_RETURN_0;
// }

// bool debug::Coverage::FunctionData::HasBlockCoverage() const {
//     LWNODE_RETURN_FALSE;}

// debug::Coverage::BlockData debug::Coverage::FunctionData::GetBlockData(
//     size_t i) const {
//   return BlockData(&function_->blocks.at(i), coverage_);
// }

// Local<debug::Script> debug::Coverage::ScriptData::GetScript() const {
//   return ToApiHandle<debug::Script>(script_->script);
// }

// size_t debug::Coverage::ScriptData::FunctionCount() const {
//   return script_->functions.size();
// }

// debug::Coverage::FunctionData debug::Coverage::ScriptData::GetFunctionData(
//     size_t i) const {
//   return FunctionData(&script_->functions.at(i), coverage_);
// }

// debug::Coverage::ScriptData::ScriptData(size_t index,
//                                         std::shared_ptr<i::Coverage>
//                                         coverage)
//     : script_(&coverage->at(index)), coverage_(std::move(coverage)) {}

// size_t debug::Coverage::ScriptCount() const {
//   return coverage_->size();
// }

// debug::Coverage::ScriptData debug::Coverage::GetScriptData(size_t i) const {
//   return ScriptData(i, coverage_);
// }

// debug::Coverage debug::Coverage::CollectPrecise(Isolate* isolate) {
//   return Coverage(
//       i::Coverage::CollectPrecise(reinterpret_cast<i::Isolate*>(isolate)));
// }

// debug::Coverage debug::Coverage::CollectBestEffort(Isolate* isolate) {
//   LWNODE_UNIMPLEMENT;
//   return Coverage(nullptr);
// }

// void debug::Coverage::SelectMode(Isolate* isolate, debug::CoverageMode mode)
// {
//   LWNODE_RETURN_VOID;
// }

// int debug::TypeProfile::Entry::SourcePosition() const {LWNODE_RETURN_0;}

// std::vector<MaybeLocal<String>> debug::TypeProfile::Entry::Types() const {
//   LWNODE_UNIMPLEMENT;
//   std::vector<MaybeLocal<String>> result;
//   return result;
// }

// debug::TypeProfile::ScriptData::ScriptData(
//     size_t index, std::shared_ptr<i::TypeProfile> type_profile)
//     : script_(&type_profile->at(index)),
//       type_profile_(std::move(type_profile)) {}

// Local<debug::Script> debug::TypeProfile::ScriptData::GetScript() const {
//   LWNODE_RETURN_LOCAL(debug::Script);
// }

// std::vector<debug::TypeProfile::Entry>
// debug::TypeProfile::ScriptData::Entries()
//     const {
//   LWNODE_UNIMPLEMENT;
//   std::vector<debug::TypeProfile::Entry> result;
//   return result;
// }

// debug::TypeProfile debug::TypeProfile::Collect(Isolate* isolate) {
//   return TypeProfile(nullptr);
// }

// void debug::TypeProfile::SelectMode(Isolate* isolate,
//                                     debug::TypeProfileMode mode){
//     LWNODE_RETURN_VOID;}

// size_t debug::TypeProfile::ScriptCount() const {LWNODE_RETURN_0;}

// debug::TypeProfile::ScriptData debug::TypeProfile::GetScriptData(
//     size_t i) const {
//   LWNODE_UNIMPLEMENT;
//   return ScriptData(i, type_profile_);
// }

// v8::MaybeLocal<v8::Value> debug::WeakMap::Get(v8::Local<v8::Context> context,
//                                               v8::Local<v8::Value> key){
//     LWNODE_RETURN_LOCAL(Value);}

// v8::MaybeLocal<debug::WeakMap> debug::WeakMap::Set(
//     v8::Local<v8::Context> context,
//     v8::Local<v8::Value> key,
//     v8::Local<v8::Value> value){LWNODE_RETURN_LOCAL(debug::WeakMap);}

// Local<debug::WeakMap> debug::WeakMap::New(v8::Isolate* isolate){
//     LWNODE_RETURN_LOCAL(debug::WeakMap);}

// debug::WeakMap* debug::WeakMap::Cast(v8::Value*
// value){LWNODE_RETURN_NULLPTR;}

// Local<Value> debug::AccessorPair::getter(){LWNODE_RETURN_LOCAL(Value);}

// Local<Value> debug::AccessorPair::setter() {
//   LWNODE_RETURN_LOCAL(Value);
// }

// bool debug::AccessorPair::IsAccessorPair(Local<Value> that) {
//   LWNODE_RETURN_FALSE;
// }

// int debug::WasmValue::value_type(){LWNODE_RETURN_0;}

// v8::Local<v8::Array> debug::WasmValue::bytes(){LWNODE_RETURN_LOCAL(Array);}

// v8::Local<v8::Value> debug::WasmValue::ref() {
//   LWNODE_RETURN_LOCAL(Value);
// }

// bool debug::WasmValue::IsWasmValue(Local<Value> that){LWNODE_RETURN_FALSE;}

// MaybeLocal<Message> debug::GetMessageFromPromise(Local<Promise> p) {
//   LWNODE_RETURN_LOCAL(Message);
// }

const char* CpuProfileNode::GetFunctionNameStr() const {
  LWNODE_RETURN_NULLPTR;
}

int CpuProfileNode::GetScriptId() const {
  LWNODE_RETURN_0;
}

Local<String> CpuProfileNode::GetScriptResourceName() const {
  LWNODE_RETURN_LOCAL(String);
}

const char* CpuProfileNode::GetScriptResourceNameStr() const {
  LWNODE_RETURN_NULLPTR;
}

bool CpuProfileNode::IsScriptSharedCrossOrigin() const {
  LWNODE_RETURN_FALSE;
}

int CpuProfileNode::GetLineNumber() const {
  LWNODE_RETURN_0;
}

int CpuProfileNode::GetColumnNumber() const {
  LWNODE_RETURN_0;
}

unsigned int CpuProfileNode::GetHitLineCount() const {
  LWNODE_RETURN_0;
}

bool CpuProfileNode::GetLineTicks(LineTick* entries,
                                  unsigned int length) const {
  LWNODE_RETURN_FALSE;
}

const char* CpuProfileNode::GetBailoutReason() const {
  LWNODE_RETURN_NULLPTR;
}

unsigned CpuProfileNode::GetHitCount() const {
  LWNODE_RETURN_0;
}

unsigned CpuProfileNode::GetNodeId() const {
  LWNODE_RETURN_0;
}

CpuProfileNode::SourceType CpuProfileNode::GetSourceType() const {
  LWNODE_UNIMPLEMENT;
  return kScript;
}

int CpuProfileNode::GetChildrenCount() const {
  LWNODE_RETURN_0;
}

const CpuProfileNode* CpuProfileNode::GetChild(int index) const {
  LWNODE_RETURN_NULLPTR;
}

const CpuProfileNode* CpuProfileNode::GetParent() const {
  LWNODE_RETURN_NULLPTR;
}

void CpuProfile::Delete() {
  LWNODE_RETURN_VOID;
}

Local<String> CpuProfile::GetTitle() const {
  LWNODE_RETURN_LOCAL(String);
}

const CpuProfileNode* CpuProfile::GetTopDownRoot() const {
  LWNODE_RETURN_NULLPTR;
}

const CpuProfileNode* CpuProfile::GetSample(int index) const {
  LWNODE_RETURN_NULLPTR;
}

int64_t CpuProfile::GetSampleTimestamp(int index) const {
  LWNODE_RETURN_0;
}

int64_t CpuProfile::GetStartTime() const {
  LWNODE_RETURN_0;
}

int64_t CpuProfile::GetEndTime() const {
  LWNODE_RETURN_0;
}

int CpuProfile::GetSamplesCount() const {
  LWNODE_RETURN_0;
}

CpuProfiler* CpuProfiler::New(Isolate* isolate,
                              CpuProfilingNamingMode naming_mode,
                              CpuProfilingLoggingMode logging_mode) {
  LWNODE_RETURN_NULLPTR;
}

CpuProfilingOptions::CpuProfilingOptions(CpuProfilingMode mode,
                                         unsigned max_samples,
                                         int sampling_interval_us,
                                         MaybeLocal<Context> filter_context)
    : mode_(mode),
      max_samples_(max_samples),
      sampling_interval_us_(sampling_interval_us) {
  LWNODE_UNIMPLEMENT;
}

void* CpuProfilingOptions::raw_filter_context() const {
  LWNODE_RETURN_NULLPTR;
}

void CpuProfiler::Dispose() {
  LWNODE_RETURN_VOID;
}

// static
void CpuProfiler::CollectSample(Isolate* isolate) {
  LWNODE_RETURN_VOID;
}

void CpuProfiler::SetSamplingInterval(int us) {
  LWNODE_RETURN_VOID;
}

void CpuProfiler::SetUsePreciseSampling(bool use_precise_sampling) {
  LWNODE_RETURN_VOID;
}

void CpuProfiler::StartProfiling(Local<String> title,
                                 CpuProfilingOptions options) {
  LWNODE_RETURN_VOID;
}

void CpuProfiler::StartProfiling(Local<String> title, bool record_samples) {
  LWNODE_RETURN_VOID;
}

void CpuProfiler::StartProfiling(Local<String> title,
                                 CpuProfilingMode mode,
                                 bool record_samples,
                                 unsigned max_samples) {
  LWNODE_RETURN_VOID;
}

CpuProfile* CpuProfiler::StopProfiling(Local<String> title) {
  LWNODE_RETURN_NULLPTR;
}

void CpuProfiler::UseDetailedSourcePositionsForProfiling(Isolate* isolate) {
  LWNODE_CALL_TRACE();
}

uintptr_t CodeEvent::GetCodeStartAddress() {
  LWNODE_RETURN_0;
}

size_t CodeEvent::GetCodeSize() {
  LWNODE_RETURN_0;
}

Local<String> CodeEvent::GetFunctionName() {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> CodeEvent::GetScriptName() {
  LWNODE_RETURN_LOCAL(String);
}

int CodeEvent::GetScriptLine() {
  LWNODE_RETURN_0;
}

int CodeEvent::GetScriptColumn() {
  LWNODE_RETURN_0;
}

CodeEventType CodeEvent::GetCodeType() {
  LWNODE_UNIMPLEMENT;
  return kUnknownType;
}

const char* CodeEvent::GetComment() {
  LWNODE_RETURN_NULLPTR;
}

uintptr_t CodeEvent::GetPreviousCodeStartAddress() {
  LWNODE_RETURN_0;
}

const char* CodeEvent::GetCodeEventTypeName(CodeEventType code_event_type) {
  LWNODE_RETURN_NULLPTR;
}

CodeEventHandler::CodeEventHandler(Isolate* isolate) {
  LWNODE_UNIMPLEMENT;
}

CodeEventHandler::~CodeEventHandler() {
  LWNODE_UNIMPLEMENT;
}

void CodeEventHandler::Enable() {
  LWNODE_RETURN_VOID;
}

void CodeEventHandler::Disable() {
  LWNODE_RETURN_VOID;
}

HeapGraphEdge::Type HeapGraphEdge::GetType() const {
  LWNODE_UNIMPLEMENT;
  return kContextVariable;
}

Local<Value> HeapGraphEdge::GetName() const {
  LWNODE_RETURN_LOCAL(Value);
}

const HeapGraphNode* HeapGraphEdge::GetFromNode() const {
  LWNODE_RETURN_NULLPTR;
}

const HeapGraphNode* HeapGraphEdge::GetToNode() const {
  LWNODE_RETURN_NULLPTR;
}

HeapGraphNode::Type HeapGraphNode::GetType() const {
  LWNODE_UNIMPLEMENT;
  return kHidden;
}

Local<String> HeapGraphNode::GetName() const {
  LWNODE_RETURN_LOCAL(String);
}

SnapshotObjectId HeapGraphNode::GetId() const {
  LWNODE_RETURN_0;
}

size_t HeapGraphNode::GetShallowSize() const {
  LWNODE_RETURN_0;
}

int HeapGraphNode::GetChildrenCount() const {
  LWNODE_RETURN_0;
}

const HeapGraphEdge* HeapGraphNode::GetChild(int index) const {
  LWNODE_RETURN_NULLPTR;
}

void HeapSnapshot::Delete() {
  LWNODE_RETURN_VOID;
}

const HeapGraphNode* HeapSnapshot::GetRoot() const {
  LWNODE_RETURN_NULLPTR;
}

const HeapGraphNode* HeapSnapshot::GetNodeById(SnapshotObjectId id) const {
  LWNODE_RETURN_NULLPTR;
}

int HeapSnapshot::GetNodesCount() const {
  LWNODE_RETURN_0;
}

const HeapGraphNode* HeapSnapshot::GetNode(int index) const {
  LWNODE_RETURN_NULLPTR;
}

SnapshotObjectId HeapSnapshot::GetMaxSnapshotJSObjectId() const {
  LWNODE_RETURN_0;
}

void HeapSnapshot::Serialize(OutputStream* stream,
                             HeapSnapshot::SerializationFormat format) const {
  LWNODE_RETURN_VOID;
}

int HeapProfiler::GetSnapshotCount() {
  LWNODE_RETURN_0;
}

const HeapSnapshot* HeapProfiler::GetHeapSnapshot(int index) {
  LWNODE_RETURN_NULLPTR;
}

SnapshotObjectId HeapProfiler::GetObjectId(Local<Value> value) {
  LWNODE_RETURN_0;
}

SnapshotObjectId HeapProfiler::GetObjectId(NativeObject value) {
  LWNODE_RETURN_0;
}

Local<Value> HeapProfiler::FindObjectById(SnapshotObjectId id) {
  LWNODE_RETURN_LOCAL(Value);
}

void HeapProfiler::ClearObjectIds() {
  LWNODE_RETURN_VOID;
}

const HeapSnapshot* HeapProfiler::TakeHeapSnapshot(
    ActivityControl* control,
    ObjectNameResolver* resolver,
    bool treat_global_objects_as_roots) {
  LWNODE_RETURN_NULLPTR;
}

void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) {
  LWNODE_RETURN_VOID;
}

void HeapProfiler::StopTrackingHeapObjects() {
  LWNODE_RETURN_VOID;
}

SnapshotObjectId HeapProfiler::GetHeapStats(OutputStream* stream,
                                            int64_t* timestamp_us) {
  LWNODE_RETURN_0;
}

bool HeapProfiler::StartSamplingHeapProfiler(uint64_t sample_interval,
                                             int stack_depth,
                                             SamplingFlags flags) {
  LWNODE_RETURN_FALSE;
}

void HeapProfiler::StopSamplingHeapProfiler() {
  LWNODE_RETURN_VOID;
}

AllocationProfile* HeapProfiler::GetAllocationProfile() {
  LWNODE_RETURN_NULLPTR;
}

void HeapProfiler::DeleteAllHeapSnapshots() {
  LWNODE_RETURN_VOID;
}

void HeapProfiler::AddBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  LWNODE_UNIMPLEMENT_IGNORED;
}

void HeapProfiler::RemoveBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::SetStackStart(void* stack_start) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::NotifyEmptyEmbedderStack() {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::FinalizeTracing() {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::GarbageCollectionForTesting(
    EmbedderStackState stack_state) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::IncreaseAllocatedSize(size_t bytes) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::DecreaseAllocatedSize(size_t bytes) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::RegisterEmbedderReference(
    const TracedReferenceBase<v8::Data>& ref) {
  LWNODE_RETURN_VOID;
}

void EmbedderHeapTracer::IterateTracedGlobalHandles(
    TracedGlobalHandleVisitor* visitor) {
  LWNODE_RETURN_VOID;
}

bool EmbedderHeapTracer::IsRootForNonTracingGC(
    const v8::TracedReference<v8::Value>& handle) {
  LWNODE_RETURN_FALSE;
}

bool EmbedderHeapTracer::IsRootForNonTracingGC(
    const v8::TracedGlobal<v8::Value>& handle) {
  LWNODE_RETURN_FALSE;
}

void EmbedderHeapTracer::ResetHandleInNonTracingGC(
    const v8::TracedReference<v8::Value>& handle) {
  LWNODE_RETURN_VOID;
}

// const void* CTypeInfo::GetWrapperInfo() const {LWNODE_RETURN_NULLPTR;}

// CFunction::CFunction(const void* address, const CFunctionInfo* type_info)
//     : address_(address), type_info_(type_info) {
//   LWNODE_RETURN_VOID;
// }
}  // namespace v8
