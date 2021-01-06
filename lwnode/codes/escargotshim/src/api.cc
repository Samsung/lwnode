/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <signal.h>
#include <algorithm>  // For min
#include <cmath>      // For isnan.
#include <limits>
#include <string>
#include <utility>  // For move
#include <vector>

#include "v8-profiler.h"
#include "v8-util.h"

#include "escargotshim-base.h"

namespace i = v8::internal;

// V has parameters (Type, type, TYPE, C type)
#define TYPED_ARRAYS(V)                                                        \
  V(Uint8, uint8, UINT8, uint8_t)                                              \
  V(Int8, int8, INT8, int8_t)                                                  \
  V(Uint16, uint16, UINT16, uint16_t)                                          \
  V(Int16, int16, INT16, int16_t)                                              \
  V(Uint32, uint32, UINT32, uint32_t)                                          \
  V(Int32, int32, INT32, int32_t)                                              \
  V(Float32, float32, FLOAT32, float)                                          \
  V(Float64, float64, FLOAT64, double)                                         \
  V(Uint8Clamped, uint8_clamped, UINT8_CLAMPED, uint8_t)                       \
  V(BigUint64, biguint64, BIGUINT64, uint64_t)                                 \
  V(BigInt64, bigint64, BIGINT64, int64_t)

namespace v8 {

namespace {

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  void* Allocate(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_calloc(length, 1);
#else
    void* data = calloc(length, 1);
#endif
    return data;
  }

  void* AllocateUninitialized(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_malloc(length);
#else
    void* data = malloc(length);
#endif
    return data;
  }

  void Free(void* data, size_t) override { free(data); }

  void* Reallocate(void* data, size_t old_length, size_t new_length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* new_data = __linux_realloc(data, new_length);
#else
    void* new_data = realloc(data, new_length);
#endif
    if (new_length > old_length) {
      memset(reinterpret_cast<uint8_t*>(new_data) + old_length,
             0,
             new_length - old_length);
    }
    return new_data;
  }
};

struct SnapshotCreatorData {
  explicit SnapshotCreatorData(Isolate* isolate)
      : isolate_(isolate),
        default_context_(),
        contexts_(isolate),
        created_(false) {}

  static SnapshotCreatorData* cast(void* data) {
    return reinterpret_cast<SnapshotCreatorData*>(data);
  }

  ArrayBufferAllocator allocator_;
  Isolate* isolate_;
  Persistent<Context> default_context_;
  SerializeInternalFieldsCallback default_embedder_fields_serializer_;
  PersistentValueVector<Context> contexts_;
  std::vector<SerializeInternalFieldsCallback> embedder_fields_serializers_;
  bool created_;
};

}  // namespace

SnapshotCreator::SnapshotCreator(Isolate* isolate,
                                 const intptr_t* external_references,
                                 StartupData* existing_snapshot) {
  LWNODE_UNIMPLEMENT;
}

SnapshotCreator::SnapshotCreator(const intptr_t* external_references,
                                 StartupData* existing_snapshot)
    : SnapshotCreator(
          Isolate::Allocate(), external_references, existing_snapshot) {}

SnapshotCreator::~SnapshotCreator() {
  LWNODE_UNIMPLEMENT;
}

Isolate* SnapshotCreator::GetIsolate() {
  LWNODE_RETURN_NULLPTR;
}

void SnapshotCreator::SetDefaultContext(
    Local<Context> context, SerializeInternalFieldsCallback callback) {
  LWNODE_RETURN_VOID;
}

size_t SnapshotCreator::AddContext(Local<Context> context,
                                   SerializeInternalFieldsCallback callback) {
  LWNODE_RETURN_0;
}

size_t SnapshotCreator::AddData(i::Address object) {
  LWNODE_RETURN_0;
}

size_t SnapshotCreator::AddData(Local<Context> context, i::Address object) {
  LWNODE_RETURN_0;
}

StartupData SnapshotCreator::CreateBlob(
    SnapshotCreator::FunctionCodeHandling function_code_handling) {
  LWNODE_UNIMPLEMENT;
  return StartupData();
}

bool StartupData::CanBeRehashed() const {
  LWNODE_RETURN_FALSE;
}

void V8::SetDcheckErrorHandler(DcheckErrorCallback that) {
  LWNODE_RETURN_VOID;
}

void V8::SetFlagsFromString(const char* str) {
  LWNODE_RETURN_VOID;
}

void V8::SetFlagsFromString(const char* str, size_t length) {
  LWNODE_RETURN_VOID;
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  LWNODE_RETURN_VOID;
}

// RegisteredExtension* RegisteredExtension::first_extension_ = nullptr;

// RegisteredExtension::RegisteredExtension(std::unique_ptr<Extension>
// extension)
//     : extension_(std::move(extension)) {}

// // static
// void RegisteredExtension::Register(std::unique_ptr<Extension> extension) {
//   LWNODE_RETURN_VOID;
// }

// // static
// void RegisteredExtension::UnregisterAll() {
//   LWNODE_RETURN_VOID;
// }

Extension::Extension(const char* name,
                     const char* source,
                     int dep_count,
                     const char** deps,
                     int source_length)
    : name_(name),
      source_length_(source_length >= 0
                         ? source_length
                         : (source ? static_cast<int>(strlen(source)) : 0)),
      dep_count_(dep_count),
      deps_(deps),
      auto_enable_(false) {
  LWNODE_UNIMPLEMENT;
}

void ResourceConstraints::ConfigureDefaultsFromHeapSize(
    size_t initial_heap_size_in_bytes, size_t maximum_heap_size_in_bytes) {
  LWNODE_RETURN_VOID;
}

void ResourceConstraints::ConfigureDefaults(uint64_t physical_memory,
                                            uint64_t virtual_memory_limit) {
  LWNODE_RETURN_VOID;
}

size_t ResourceConstraints::max_semi_space_size_in_kb() const {
  LWNODE_RETURN_0;
}

void ResourceConstraints::set_max_semi_space_size_in_kb(size_t limit_in_kb) {
  LWNODE_RETURN_VOID;
}

i::Address* V8::GlobalizeReference(i::Isolate* isolate, i::Address* obj) {
  LWNODE_RETURN_NULLPTR;
}

i::Address* V8::GlobalizeTracedReference(i::Isolate* isolate,
                                         i::Address* obj,
                                         internal::Address* slot,
                                         bool has_destructor) {
  LWNODE_RETURN_NULLPTR;
}

i::Address* V8::CopyGlobalReference(i::Address* from) {
  LWNODE_RETURN_NULLPTR;
}

void V8::MoveGlobalReference(internal::Address** from, internal::Address** to) {
  LWNODE_RETURN_VOID;
}

void V8::MoveTracedGlobalReference(internal::Address** from,
                                   internal::Address** to) {
  LWNODE_RETURN_VOID;
}

void V8::CopyTracedGlobalReference(const internal::Address* const* from,
                                   internal::Address** to) {
  LWNODE_RETURN_VOID;
}

void V8::MakeWeak(i::Address* location,
                  void* parameter,
                  WeakCallbackInfo<void>::Callback weak_callback,
                  WeakCallbackType type) {
  LWNODE_RETURN_VOID;
}

void V8::MakeWeak(i::Address** location_addr) {
  LWNODE_RETURN_VOID;
}

void* V8::ClearWeak(i::Address* location) {
  LWNODE_RETURN_NULLPTR;
}

void V8::AnnotateStrongRetainer(i::Address* location, const char* label) {
  LWNODE_RETURN_VOID;
}

void V8::DisposeGlobal(i::Address* location) {
  LWNODE_RETURN_VOID;
}

void V8::DisposeTracedGlobal(internal::Address* location) {
  LWNODE_RETURN_VOID;
}

void V8::SetFinalizationCallbackTraced(
    internal::Address* location,
    void* parameter,
    WeakCallbackInfo<void>::Callback callback) {
  LWNODE_RETURN_VOID;
}

Value* V8::Eternalize(Isolate* v8_isolate, Value* value) {
  // i::Isolate* isolate = reinterpret_cast<i::Isolate*>(v8_isolate);
  // i::Object object = *Utils::OpenHandle(value);
  // int index = -1;
  // isolate->eternal_handles()->Create(isolate, object, &index);
  // return reinterpret_cast<Value*>(
  //     isolate->eternal_handles()->Get(index).location());
  LWNODE_RETURN_NULLPTR;
}

void V8::FromJustIsNothing() {
  LWNODE_RETURN_VOID;
}

void V8::ToLocalEmpty() {
  LWNODE_RETURN_VOID;
}

void V8::InternalFieldOutOfBounds(int index) {
  LWNODE_RETURN_VOID;
}

// --- H a n d l e s ---

HandleScope::HandleScope(Isolate* isolate) {
  LWNODE_UNIMPLEMENT;
}

void HandleScope::Initialize(Isolate* isolate) {
  LWNODE_RETURN_VOID;
}

HandleScope::~HandleScope() {
  LWNODE_UNIMPLEMENT;
}

void* HandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new HandleScope(nullptr);
}
void* HandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new HandleScope(nullptr);
}
void HandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void HandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

int HandleScope::NumberOfHandles(Isolate* isolate) {
  LWNODE_RETURN_0;
}

i::Address* HandleScope::CreateHandle(i::Isolate* isolate, i::Address value) {
  LWNODE_RETURN_NULLPTR;
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate) {
  LWNODE_UNIMPLEMENT;
}

i::Address* EscapableHandleScope::Escape(i::Address* escape_value) {
  LWNODE_RETURN_NULLPTR;
}

void* EscapableHandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new EscapableHandleScope(nullptr);
}
void* EscapableHandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new EscapableHandleScope(nullptr);
}
void EscapableHandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void EscapableHandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)) {
  LWNODE_UNIMPLEMENT;
}

SealHandleScope::~SealHandleScope() {
  LWNODE_UNIMPLEMENT;
}

void* SealHandleScope::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new SealHandleScope(nullptr);
}
void* SealHandleScope::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new SealHandleScope(nullptr);
}
void SealHandleScope::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void SealHandleScope::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

void Context::Enter() {
  LWNODE_UNIMPLEMENT;
}

void Context::Exit() {
  LWNODE_UNIMPLEMENT;
}

Context::BackupIncumbentScope::BackupIncumbentScope(
    Local<Context> backup_incumbent_context)
    : backup_incumbent_context_(backup_incumbent_context) {
  LWNODE_UNIMPLEMENT;
}

Context::BackupIncumbentScope::~BackupIncumbentScope() {
  LWNODE_UNIMPLEMENT;
}

uint32_t Context::GetNumberOfEmbedderDataFields() {
  LWNODE_RETURN_0;
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
  LWNODE_RETURN_LOCAL(Value);
}

void Context::SetEmbedderData(int index, v8::Local<Value> value) {
  LWNODE_RETURN_VOID;
}

void* Context::SlowGetAlignedPointerFromEmbedderData(int index) {
  LWNODE_RETURN_NULLPTR;
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  LWNODE_RETURN_VOID;
}

// --- T e m p l a t e ---

void Template::Set(v8::Local<Name> name,
                   v8::Local<Data> value,
                   v8::PropertyAttribute attribute) {
  LWNODE_UNIMPLEMENT;
}

void Template::SetPrivate(v8::Local<Private> name,
                          v8::Local<Data> value,
                          v8::PropertyAttribute attribute) {}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
                                   v8::Local<FunctionTemplate> getter,
                                   v8::Local<FunctionTemplate> setter,
                                   v8::PropertyAttribute attribute,
                                   v8::AccessControl access_control) {
  LWNODE_RETURN_VOID;
}

// --- F u n c t i o n   T e m p l a t e ---

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void FunctionTemplate::SetPrototypeProviderTemplate(
    Local<FunctionTemplate> prototype_provider) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  LWNODE_RETURN_VOID;
}

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
                                              FunctionCallback callback,
                                              v8::Local<Value> data,
                                              v8::Local<Signature> signature,
                                              int length,
                                              ConstructorBehavior behavior,
                                              SideEffectType side_effect_type,
                                              const CFunction* c_function) {
  LWNODE_RETURN_LOCAL(FunctionTemplate);
}

Local<FunctionTemplate> FunctionTemplate::NewWithCache(
    Isolate* isolate,
    FunctionCallback callback,
    Local<Private> cache_property,
    Local<Value> data,
    Local<Signature> signature,
    int length,
    SideEffectType side_effect_type) {
  LWNODE_RETURN_LOCAL(FunctionTemplate);
}

Local<Signature> Signature::New(Isolate* isolate,
                                Local<FunctionTemplate> receiver) {
  LWNODE_RETURN_LOCAL(Signature);
}

Local<AccessorSignature> AccessorSignature::New(
    Isolate* isolate, Local<FunctionTemplate> receiver) {
  LWNODE_RETURN_LOCAL(AccessorSignature);
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data,
                                      SideEffectType side_effect_type,
                                      const CFunction* c_function) {
  LWNODE_RETURN_VOID;
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void FunctionTemplate::SetLength(int length) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::SetClassName(Local<String> name) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::SetAcceptAnyReceiver(bool value) {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::ReadOnlyPrototype() {
  LWNODE_RETURN_VOID;
}

void FunctionTemplate::RemovePrototype() {
  LWNODE_RETURN_VOID;
}

// --- O b j e c t T e m p l a t e ---

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  LWNODE_RETURN_LOCAL(ObjectTemplate);
}

void Template::SetNativeDataProperty(v8::Local<String> name,
                                     AccessorGetterCallback getter,
                                     AccessorSetterCallback setter,
                                     v8::Local<Value> data,
                                     PropertyAttribute attribute,
                                     v8::Local<AccessorSignature> signature,
                                     AccessControl settings,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetNativeDataProperty(v8::Local<Name> name,
                                     AccessorNameGetterCallback getter,
                                     AccessorNameSetterCallback setter,
                                     v8::Local<Value> data,
                                     PropertyAttribute attribute,
                                     v8::Local<AccessorSignature> signature,
                                     AccessControl settings,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetLazyDataProperty(v8::Local<Name> name,
                                   AccessorNameGetterCallback getter,
                                   v8::Local<Value> data,
                                   PropertyAttribute attribute,
                                   SideEffectType getter_side_effect_type,
                                   SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void Template::SetIntrinsicDataProperty(Local<Name> name,
                                        Intrinsic intrinsic,
                                        PropertyAttribute attribute) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessor(v8::Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter,
                                 v8::Local<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessor(v8::Local<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 v8::Local<Value> data,
                                 AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::MarkAsUndetectable() {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessCheckCallback(AccessCheckCallback callback,
                                            Local<Value> data) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetAccessCheckCallbackAndHandler(
    AccessCheckCallback callback,
    const NamedPropertyHandlerConfiguration& named_handler,
    const IndexedPropertyHandlerConfiguration& indexed_handler,
    Local<Value> data) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetHandler(
    const IndexedPropertyHandlerConfiguration& config) {
  LWNODE_RETURN_VOID;
}

void ObjectTemplate::SetCallAsFunctionHandler(FunctionCallback callback,
                                              Local<Value> data) {
  LWNODE_RETURN_VOID;
}

int ObjectTemplate::InternalFieldCount() {
  LWNODE_RETURN_0;
}

void ObjectTemplate::SetInternalFieldCount(int value) {}

bool ObjectTemplate::IsImmutableProto() {
  LWNODE_RETURN_FALSE;
}

void ObjectTemplate::SetImmutableProto() {}

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
  LWNODE_RETURN_LOCAL(Script);
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
  LWNODE_RETURN_LOCAL(Value);
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
  LWNODE_RETURN_LOCAL(UnboundScript);
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
  LWNODE_RETURN_LOCAL(Script);
}

// --- E x c e p t i o n s ---

v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(nullptr),
      next_(nullptr),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {}

v8::TryCatch::~TryCatch() {}

void* v8::TryCatch::operator new(size_t) {
  LWNODE_UNIMPLEMENT;
  return new TryCatch(nullptr);
}
void* v8::TryCatch::operator new[](size_t) {
  LWNODE_UNIMPLEMENT;
  return new TryCatch(nullptr);
}
void v8::TryCatch::operator delete(void*, size_t) {
  LWNODE_UNIMPLEMENT;
}
void v8::TryCatch::operator delete[](void*, size_t) {
  LWNODE_UNIMPLEMENT;
}

bool v8::TryCatch::HasCaught() const {
  LWNODE_RETURN_FALSE;
}

bool v8::TryCatch::CanContinue() const {
  LWNODE_RETURN_FALSE;
}

bool v8::TryCatch::HasTerminated() const {
  LWNODE_RETURN_FALSE;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::TryCatch::Exception() const {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::TryCatch::StackTrace(Local<Context> context,
                                           Local<Value> exception) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::TryCatch::StackTrace(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<v8::Message> v8::TryCatch::Message() const {
  LWNODE_RETURN_LOCAL(v8::Message);
}

void v8::TryCatch::Reset() {}

void v8::TryCatch::ResetInternal() {}

void v8::TryCatch::SetVerbose(bool value) {}

bool v8::TryCatch::IsVerbose() const {
  LWNODE_RETURN_FALSE;
}

void v8::TryCatch::SetCaptureMessage(bool value) {}

// --- M e s s a g e ---

Local<String> Message::Get() const {
  LWNODE_RETURN_LOCAL(String);
}

v8::Isolate* Message::GetIsolate() const {
  LWNODE_RETURN_NULLPTR;
}

ScriptOrigin Message::GetScriptOrigin() const {
  LWNODE_UNIMPLEMENT;
  return ScriptOrigin(Local<Value>());
}

v8::Local<Value> Message::GetScriptResourceName() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  LWNODE_RETURN_LOCAL(StackTrace);
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int)
}

int Message::GetStartPosition() const {
  LWNODE_RETURN_0;
}

int Message::GetEndPosition() const {
  LWNODE_RETURN_0;
}

int Message::ErrorLevel() const {
  LWNODE_RETURN_0;
}

int Message::GetStartColumn() const {
  LWNODE_RETURN_0;
}

int Message::GetWasmFunctionIndex() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int);
}

int Message::GetEndColumn() const {
  LWNODE_RETURN_0;
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int);
}

bool Message::IsSharedCrossOrigin() const {
  LWNODE_RETURN_FALSE;
}

bool Message::IsOpaque() const {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(String);
}

void Message::PrintCurrentStackTrace(Isolate* isolate, FILE* out) {
  LWNODE_UNIMPLEMENT;
}

// --- S t a c k T r a c e ---

Local<StackFrame> StackTrace::GetFrame(Isolate* v8_isolate,
                                       uint32_t index) const {
  LWNODE_RETURN_LOCAL(StackFrame);
}

int StackTrace::GetFrameCount() const {
  LWNODE_RETURN_0;
}

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  LWNODE_RETURN_LOCAL(StackTrace);
}

// --- S t a c k F r a m e ---

int StackFrame::GetLineNumber() const {
  LWNODE_RETURN_0;
}

int StackFrame::GetColumn() const {
  LWNODE_RETURN_0;
}

int StackFrame::GetScriptId() const {
  LWNODE_RETURN_0;
}

Local<String> StackFrame::GetScriptName() const {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> StackFrame::GetScriptNameOrSourceURL() const {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> StackFrame::GetFunctionName() const {
  LWNODE_RETURN_LOCAL(String);
}

bool StackFrame::IsEval() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsConstructor() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsWasm() const {
  LWNODE_RETURN_FALSE;
}

bool StackFrame::IsUserJavaScript() const {
  LWNODE_RETURN_FALSE;
}

// --- J S O N ---

MaybeLocal<Value> JSON::Parse(Local<Context> context,
                              Local<String> json_string) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<String> JSON::Stringify(Local<Context> context,
                                   Local<Value> json_object,
                                   Local<String> gap) {
  LWNODE_RETURN_LOCAL(String);
}

// --- V a l u e   S e r i a l i z a t i o n ---

Maybe<bool> ValueSerializer::Delegate::WriteHostObject(Isolate* v8_isolate,
                                                       Local<Object> object) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<uint32_t> ValueSerializer::Delegate::GetSharedArrayBufferId(
    Isolate* v8_isolate, Local<SharedArrayBuffer> shared_array_buffer) {
  LWNODE_RETURN_MAYBE(uint32_t);
}

Maybe<uint32_t> ValueSerializer::Delegate::GetWasmModuleTransferId(
    Isolate* v8_isolate, Local<WasmModuleObject> module) {
  LWNODE_RETURN_MAYBE(uint32_t);
}

void* ValueSerializer::Delegate::ReallocateBufferMemory(void* old_buffer,
                                                        size_t size,
                                                        size_t* actual_size) {
  LWNODE_RETURN_NULLPTR;
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
  LWNODE_UNIMPLEMENT;
}

// struct ValueSerializer::PrivateData {
//   LWNODE_UNIMPLEMENT;
// };

ValueSerializer::ValueSerializer(Isolate* isolate)
    : ValueSerializer(isolate, nullptr) {
  LWNODE_UNIMPLEMENT;
}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate) {
  LWNODE_UNIMPLEMENT;
}
// : private_(
//       new PrivateData(reinterpret_cast<i::Isolate*>(isolate),
//       delegate)) {}

ValueSerializer::~ValueSerializer() {
  LWNODE_UNIMPLEMENT;
}

void ValueSerializer::WriteHeader() {
  LWNODE_UNIMPLEMENT;
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  return std::make_pair(nullptr, 0);
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {}

void ValueSerializer::WriteUint32(uint32_t value) {}

void ValueSerializer::WriteUint64(uint64_t value) {}

void ValueSerializer::WriteDouble(double value) {}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {}

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(
    Isolate* v8_isolate){LWNODE_RETURN_LOCAL(Object)}

MaybeLocal<WasmModuleObject> ValueDeserializer::Delegate::GetWasmModuleFromId(
    Isolate* v8_isolate, uint32_t id){LWNODE_RETURN_LOCAL(WasmModuleObject)}

MaybeLocal<SharedArrayBuffer> ValueDeserializer::Delegate::
    GetSharedArrayBufferFromId(Isolate* v8_isolate, uint32_t id) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer)
}

struct ValueDeserializer::PrivateData {};

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size)
    : ValueDeserializer(isolate, data, size, nullptr) {}

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size,
                                     Delegate* delegate) {}

ValueDeserializer::~ValueDeserializer() {}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  LWNODE_RETURN_MAYBE(bool)
}

void ValueDeserializer::SetSupportsLegacyWireFormat(
    bool supports_legacy_wire_format) {}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  LWNODE_RETURN_0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Value)
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
                                            Local<ArrayBuffer> array_buffer) {}

void ValueDeserializer::TransferSharedArrayBuffer(
    uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadDouble(double* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
  LWNODE_RETURN_FALSE;
}

// --- D a t a ---

bool Value::FullIsUndefined() const {
  LWNODE_RETURN_FALSE;
}

bool Value::FullIsNull() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsTrue() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsFalse() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsName() const {
  LWNODE_RETURN_FALSE;
}

bool Value::FullIsString() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSymbol() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArray() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArrayBuffer() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsArrayBufferView() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsTypedArray() const {
  LWNODE_RETURN_FALSE;
}

#define VALUE_IS_TYPED_ARRAY(Type, typeName, TYPE, ctype)                      \
  bool Value::Is##Type##Array() const { LWNODE_RETURN_FALSE; }

TYPED_ARRAYS(VALUE_IS_TYPED_ARRAY)

#undef VALUE_IS_TYPED_ARRAY

bool Value::IsDataView() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSharedArrayBuffer() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsObject() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsNumber() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsBigInt() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsProxy() const {
  LWNODE_RETURN_FALSE;
}

#define VALUE_IS_SPECIFIC_TYPE(Type, Check)                                    \
  bool Value::Is##Type() const { LWNODE_RETURN_FALSE; }

VALUE_IS_SPECIFIC_TYPE(ArgumentsObject, JSArgumentsObject)
VALUE_IS_SPECIFIC_TYPE(BigIntObject, BigIntWrapper)
VALUE_IS_SPECIFIC_TYPE(BooleanObject, BooleanWrapper)
VALUE_IS_SPECIFIC_TYPE(NumberObject, NumberWrapper)
VALUE_IS_SPECIFIC_TYPE(StringObject, StringWrapper)
VALUE_IS_SPECIFIC_TYPE(SymbolObject, SymbolWrapper)
VALUE_IS_SPECIFIC_TYPE(Date, JSDate)
VALUE_IS_SPECIFIC_TYPE(Map, JSMap)
VALUE_IS_SPECIFIC_TYPE(Set, JSSet)
VALUE_IS_SPECIFIC_TYPE(WasmModuleObject, WasmModuleObject)
VALUE_IS_SPECIFIC_TYPE(WeakMap, JSWeakMap)
VALUE_IS_SPECIFIC_TYPE(WeakSet, JSWeakSet)

#undef VALUE_IS_SPECIFIC_TYPE

bool Value::IsBoolean() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsInt32() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsUint32() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsNativeError() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsRegExp() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsAsyncFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsGeneratorFunction() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsGeneratorObject() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsMapIterator() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsSetIterator() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsPromise() const {
  LWNODE_RETURN_FALSE;
}

bool Value::IsModuleNamespaceObject() const {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
    LWNODE_RETURN_LOCAL(String)}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
    LWNODE_RETURN_LOCAL(String)}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
    LWNODE_RETURN_LOCAL(Object)}

MaybeLocal<BigInt> Value::ToBigInt(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(BigInt)
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  LWNODE_RETURN_FALSE;
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  LWNODE_RETURN_LOCAL(Boolean);
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Number);
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Integer);
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Int32);
}

MaybeLocal<Uint32> Value::ToUint32(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Uint32);
}

i::Isolate* i::IsolateFromNeverReadOnlySpaceObject(i::Address obj) {
  LWNODE_RETURN_NULLPTR;
  ;
}

bool i::ShouldThrowOnError(i::Isolate* isolate) {
  LWNODE_RETURN_FALSE;
}

void i::Internals::CheckInitializedImpl(v8::Isolate* external_isolate) {
  LWNODE_RETURN_VOID;
}

void External::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Object::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Function::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Boolean::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Name::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::String::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Symbol::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Private::CheckCast(v8::Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::Number::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Integer::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Int32::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Uint32::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BigInt::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Array::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Map::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Set::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Promise::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Promise::Resolver::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Proxy::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::WasmModuleObject::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

// void v8::debug::AccessorPair::CheckCast(Value* that) {}

// void v8::debug::WasmValue::CheckCast(Value* that) {}

v8::BackingStore::~BackingStore() {}

void* v8::BackingStore::Data() const {
  LWNODE_RETURN_NULLPTR;
}

size_t v8::BackingStore::ByteLength() const {
  LWNODE_RETURN_0;
}

bool v8::BackingStore::IsShared() const {
  LWNODE_RETURN_FALSE;
}

// static
std::unique_ptr<v8::BackingStore> v8::BackingStore::Reallocate(
    v8::Isolate* isolate,
    std::unique_ptr<v8::BackingStore> backing_store,
    size_t byte_length) {
  LWNODE_UNIMPLEMENT;
  return backing_store;
}

// static
void v8::BackingStore::EmptyDeleter(void* data,
                                    size_t length,
                                    void* deleter_data) {}

std::shared_ptr<v8::BackingStore> v8::ArrayBuffer::GetBackingStore() {
  LWNODE_RETURN_NULLPTR;
}

std::shared_ptr<v8::BackingStore> v8::SharedArrayBuffer::GetBackingStore() {
  LWNODE_RETURN_NULLPTR;
}

void v8::ArrayBuffer::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::ArrayBufferView::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

constexpr size_t v8::TypedArray::kMaxLength;

void v8::TypedArray::CheckCast(Value* that) {
  LWNODE_UNIMPLEMENT;
}

#define CHECK_TYPED_ARRAY_CAST(Type, typeName, TYPE, ctype)                    \
  void v8::Type##Array::CheckCast(Value* that) { LWNODE_UNIMPLEMENT; }

TYPED_ARRAYS(CHECK_TYPED_ARRAY_CAST)

#undef CHECK_TYPED_ARRAY_CAST

void v8::DataView::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::SharedArrayBuffer::CheckCast(Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::Date::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::StringObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::SymbolObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::NumberObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BigIntObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::BooleanObject::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

void v8::RegExp::CheckCast(v8::Value* that) {
  LWNODE_RETURN_VOID;
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(double);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int64_t);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(int32_t);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  LWNODE_RETURN_MAYBE(uint32_t);
}

MaybeLocal<Uint32> Value::ToArrayIndex(Local<Context> context) const {
  LWNODE_RETURN_LOCAL(Uint32);
}

Maybe<bool> Value::Equals(Local<Context> context, Local<Value> that) const {
  LWNODE_RETURN_MAYBE(bool)
}

bool Value::StrictEquals(Local<Value> that) const {
  LWNODE_RETURN_FALSE;
}

bool Value::SameValue(Local<Value> that) const {
  LWNODE_RETURN_FALSE;
}

Local<String> Value::TypeOf(v8::Isolate* external_isolate) {
  LWNODE_RETURN_LOCAL(String);
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> object){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            v8::Local<Value> key,
                            v8::Local<Value> value){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            uint32_t index,
                            v8::Local<Value> value){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           v8::Local<Name> key,
                                           v8::Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
                                           uint32_t index,
                                           v8::Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool)
}

struct v8::PropertyDescriptor::PrivateData {
  PrivateData() {}
};

v8::PropertyDescriptor::PropertyDescriptor() : private_(nullptr) {}

// DataDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

// DataDescriptor with writable field
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
                                           bool writable)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

// AccessorDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
                                           v8::Local<v8::Value> set)
    : private_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

v8::PropertyDescriptor::~PropertyDescriptor() {
  delete private_;
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  LWNODE_RETURN_LOCAL(Value);
}

bool v8::PropertyDescriptor::has_value() const {
  LWNODE_RETURN_FALSE;
}
bool v8::PropertyDescriptor::has_get() const {
  LWNODE_RETURN_FALSE;
}
bool v8::PropertyDescriptor::has_set() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::writable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_writable() const {
  LWNODE_RETURN_FALSE;
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {}

bool v8::PropertyDescriptor::enumerable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_enumerable() const {
  LWNODE_RETURN_FALSE;
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {}

bool v8::PropertyDescriptor::configurable() const {
  LWNODE_RETURN_FALSE;
}

bool v8::PropertyDescriptor::has_configurable() const {
  LWNODE_RETURN_FALSE;
}

Maybe<bool> v8::Object::DefineOwnProperty(v8::Local<v8::Context> context,
                                          v8::Local<Name> key,
                                          v8::Local<Value> value,
                                          v8::PropertyAttribute attributes) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context,
                                   Local<Private> key,
                                   Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool);
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
                                  Local<Value> key) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<PropertyAttribute> v8::Object::GetPropertyAttributes(
    Local<Context> context,
    Local<Value> key){LWNODE_RETURN_MAYBE(PropertyAttribute)}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                       Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> v8::Object::GetPrototype() {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<bool> v8::Object::SetPrototype(Local<Context> context,
                                     Local<Value> value) {
  LWNODE_RETURN_MAYBE(bool);
}

Local<Object> v8::Object::FindInstanceInPrototypeChain(
    v8::Local<FunctionTemplate> tmpl) {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context,
    KeyCollectionMode mode,
    PropertyFilter property_filter,
    IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(
    Local<Context> context,
    PropertyFilter filter,
    KeyConversionMode key_conversion) {
  LWNODE_RETURN_LOCAL(Array);
}

MaybeLocal<String> v8::Object::ObjectProtoToString(Local<Context> context) {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> v8::Object::GetConstructorName() {
  LWNODE_RETURN_LOCAL(String);
}

Maybe<bool> v8::Object::SetIntegrityLevel(Local<Context> context,
                                          IntegrityLevel level){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Delete(Local<Context> context,
                               Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::Has(Local<Context> context,
                            Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context, Local<Private> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::Delete(Local<Context> context,
                               uint32_t index){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::Has(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

template <typename Getter, typename Setter, typename Data>
static Maybe<bool> ObjectSetAccessor(Local<Context> context,
                                     Object* self,
                                     Local<Name> name,
                                     Getter getter,
                                     Setter setter,
                                     Data data,
                                     AccessControl settings,
                                     PropertyAttribute attributes,
                                     bool is_special_data_property,
                                     bool replace_on_access,
                                     SideEffectType getter_side_effect_type,
                                     SideEffectType setter_side_effect_type){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data,
                                AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type,
                                SideEffectType setter_side_effect_type) {
  LWNODE_RETURN_MAYBE(bool)
}

void Object::SetAccessorProperty(Local<Name> name,
                                 Local<Function> getter,
                                 Local<Function> setter,
                                 PropertyAttribute attribute,
                                 AccessControl settings) {}

Maybe<bool> Object::SetNativeDataProperty(
    v8::Local<v8::Context> context,
    v8::Local<Name> name,
    AccessorNameGetterCallback getter,
    AccessorNameSetterCallback setter,
    v8::Local<Value> data,
    PropertyAttribute attributes,
    SideEffectType getter_side_effect_type,
    SideEffectType setter_side_effect_type){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Object::SetLazyDataProperty(v8::Local<v8::Context> context,
                                        v8::Local<Name> name,
                                        AccessorNameGetterCallback getter,
                                        v8::Local<Value> data,
                                        PropertyAttribute attributes,
                                        SideEffectType getter_side_effect_type,
                                        SideEffectType setter_side_effect_type){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
                                       Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context, uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealNamedProperty(Local<Context> context,
                                             Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealIndexedProperty(Local<Context> context,
                                               uint32_t index) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<bool> v8::Object::HasRealNamedCallbackProperty(Local<Context> context,
                                                     Local<Name> key) {
  LWNODE_RETURN_MAYBE(bool);
}

bool v8::Object::HasNamedLookupInterceptor() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::HasIndexedLookupInterceptor() {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<Value> v8::Object::GetRealNamedPropertyInPrototypeChain(
    Local<Context> context, Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<PropertyAttribute>
v8::Object::GetRealNamedPropertyAttributesInPrototypeChain(
    Local<Context> context,
    Local<Name> key){LWNODE_RETURN_MAYBE(PropertyAttribute)}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
                                                   Local<Name> key) {
  LWNODE_RETURN_LOCAL(Value);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
    Local<Context> context,
    Local<Name> key){LWNODE_RETURN_MAYBE(PropertyAttribute)}

Local<v8::Object> v8::Object::Clone() {
  LWNODE_RETURN_LOCAL(Object);
}

Local<v8::Context> v8::Object::CreationContext() {
  LWNODE_RETURN_LOCAL(Context);
}

int v8::Object::GetIdentityHash() {
  LWNODE_RETURN_0;
}

bool v8::Object::IsCallable() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsConstructor() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsApiWrapper() {
  LWNODE_RETURN_FALSE;
}

bool v8::Object::IsUndetectable() {
  LWNODE_RETURN_FALSE;
}

MaybeLocal<Value> Object::CallAsFunction(Local<Context> context,
                                         Local<Value> recv,
                                         int argc,
                                         Local<Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Value> Object::CallAsConstructor(Local<Context> context,
                                            int argc,
                                            Local<Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback,
                                   Local<Value> data,
                                   int length,
                                   ConstructorBehavior behavior,
                                   SideEffectType side_effect_type) {
  LWNODE_RETURN_LOCAL(Function);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context,
                                         int argc,
                                         v8::Local<v8::Value> argv[]) const {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<Object> Function::NewInstanceWithSideEffectType(
    Local<Context> context,
    int argc,
    v8::Local<v8::Value> argv[],
    SideEffectType side_effect_type) const {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
                                     v8::Local<v8::Value> recv,
                                     int argc,
                                     v8::Local<v8::Value> argv[]) {
  LWNODE_RETURN_LOCAL(Value);
}

void Function::SetName(v8::Local<v8::String> name) {}

Local<Value> Function::GetName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetInferredName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetDebugName() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Function::GetDisplayName() const {
  LWNODE_RETURN_LOCAL(Value);
}

ScriptOrigin Function::GetScriptOrigin() const {
  LWNODE_UNIMPLEMENT;
  return ScriptOrigin(Local<Value>());
}

const int Function::kLineOffsetNotFound = -1;

int Function::GetScriptLineNumber() const {
  LWNODE_RETURN_0;
}

int Function::GetScriptColumnNumber() const {
  LWNODE_RETURN_0;
}

int Function::ScriptId() const {
  LWNODE_RETURN_0;
}

Local<v8::Value> Function::GetBoundFunction() const {
  LWNODE_RETURN_LOCAL(Value);
}

int Name::GetIdentityHash() {
  LWNODE_RETURN_0;
}

int String::Length() const {
  LWNODE_RETURN_0;
}

bool String::IsOneByte() const {
  LWNODE_RETURN_FALSE;
}

bool String::ContainsOnlyOneByte() const {
  LWNODE_RETURN_FALSE;
}

int String::Utf8Length(Isolate* isolate) const {
  LWNODE_RETURN_0;
}

int String::WriteUtf8(Isolate* v8_isolate,
                      char* buffer,
                      int capacity,
                      int* nchars_ref,
                      int options) const {
  LWNODE_RETURN_0;
}

int String::WriteOneByte(Isolate* isolate,
                         uint8_t* buffer,
                         int start,
                         int length,
                         int options) const {
  LWNODE_RETURN_0;
}

int String::Write(Isolate* isolate,
                  uint16_t* buffer,
                  int start,
                  int length,
                  int options) const {
  LWNODE_RETURN_0;
}

bool v8::String::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

bool v8::String::IsExternalOneByte() const {
  LWNODE_RETURN_FALSE;
}

void v8::String::VerifyExternalStringResource(
    v8::String::ExternalStringResource* value) const {}

void v8::String::VerifyExternalStringResourceBase(
    v8::String::ExternalStringResourceBase* value, Encoding encoding) const {}

String::ExternalStringResource* String::GetExternalStringResourceSlow() const {
  LWNODE_RETURN_NULLPTR;
}

String::ExternalStringResourceBase* String::GetExternalStringResourceBaseSlow(
    String::Encoding* encoding_out) const {
  LWNODE_RETURN_NULLPTR;
}

const v8::String::ExternalOneByteStringResource*
v8::String::GetExternalOneByteStringResource() const {
  LWNODE_RETURN_NULLPTR;
}

Local<Value> Symbol::Description() const {
  LWNODE_RETURN_LOCAL(Value);
}

Local<Value> Private::Name() const {
  LWNODE_RETURN_LOCAL(Value);
}

double Number::Value() const {
  LWNODE_RETURN_0;
}

bool Boolean::Value() const {
  LWNODE_RETURN_FALSE;
}

int64_t Integer::Value() const {
  LWNODE_RETURN_0;
}

int32_t Int32::Value() const {
  LWNODE_RETURN_0;
}

uint32_t Uint32::Value() const {
  LWNODE_RETURN_0;
}

int v8::Object::InternalFieldCount() {
  LWNODE_RETURN_0;
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
  LWNODE_RETURN_LOCAL(Value);
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  LWNODE_RETURN_NULLPTR;
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {}

void v8::Object::SetAlignedPointerInInternalFields(int argc,
                                                   int indices[],
                                                   void* values[]) {}

// static void* ExternalValue(i::Object obj) {
//   // Obscure semantics for undefined, but somehow checked in our unit
//   tests... if (obj.IsUndefined()) {
//     return nullptr;
//   }
//   i::Object foreign = i::JSObject::cast(obj).GetEmbedderField(0);
//   return
//   reinterpret_cast<void*>(i::Foreign::cast(foreign).foreign_address());
// }

// --- E n v i r o n m e n t ---

void v8::V8::InitializePlatform(Platform* platform) {}

void v8::V8::ShutdownPlatform() {
  LWNODE_RETURN_VOID;
}

bool v8::V8::Initialize(const int build_config) {
  LWNODE_RETURN_FALSE;
}

bool V8::TryHandleSignal(int signum, void* info, void* context) {
  LWNODE_RETURN_FALSE;
}

bool TryHandleWebAssemblyTrapPosix(int sig_code,
                                   siginfo_t* info,
                                   void* context) {
  LWNODE_RETURN_FALSE;
}

bool V8::EnableWebAssemblyTrapHandler(bool use_v8_signal_handler) {
  LWNODE_RETURN_FALSE;
}

void v8::V8::SetEntropySource(EntropySource entropy_source) {
  LWNODE_RETURN_VOID;
}

void v8::V8::SetReturnAddressLocationResolver(
    ReturnAddressLocationResolver return_address_resolver) {
  LWNODE_RETURN_VOID;
}

bool v8::V8::Dispose() {
  LWNODE_RETURN_FALSE;
}

SharedMemoryStatistics::SharedMemoryStatistics()
    : read_only_space_size_(0),
      read_only_space_used_size_(0),
      read_only_space_physical_size_(0) {}

HeapStatistics::HeapStatistics()
    : total_heap_size_(0),
      total_heap_size_executable_(0),
      total_physical_size_(0),
      total_available_size_(0),
      used_heap_size_(0),
      heap_size_limit_(0),
      malloced_memory_(0),
      external_memory_(0),
      peak_malloced_memory_(0),
      does_zap_garbage_(false),
      number_of_native_contexts_(0),
      number_of_detached_contexts_(0) {}

HeapSpaceStatistics::HeapSpaceStatistics()
    : space_name_(nullptr),
      space_size_(0),
      space_used_size_(0),
      space_available_size_(0),
      physical_space_size_(0) {}

HeapObjectStatistics::HeapObjectStatistics()
    : object_type_(nullptr),
      object_sub_type_(nullptr),
      object_count_(0),
      object_size_(0) {}

HeapCodeStatistics::HeapCodeStatistics()
    : code_and_metadata_size_(0),
      bytecode_and_metadata_size_(0),
      external_script_source_size_(0) {}

bool v8::V8::InitializeICU(const char* icu_data_file) {
  LWNODE_RETURN_FALSE;
}

bool v8::V8::InitializeICUDefaultLocation(const char* exec_path,
                                          const char* icu_data_file) {
  LWNODE_RETURN_FALSE;
}

void v8::V8::InitializeExternalStartupData(const char* directory_path) {
  LWNODE_RETURN_VOID;
}

// static
void v8::V8::InitializeExternalStartupDataFromFile(const char* snapshot_blob) {
  LWNODE_RETURN_VOID;
}

const char* v8::V8::GetVersion() {
  LWNODE_RETURN_NULLPTR;
}

void V8::GetSharedMemoryStatistics(SharedMemoryStatistics* statistics) {
  LWNODE_RETURN_VOID;
}

Local<Context> NewContext(
    v8::Isolate* external_isolate,
    v8::ExtensionConfiguration* extensions,
    v8::MaybeLocal<ObjectTemplate> global_template,
    v8::MaybeLocal<Value> global_object,
    size_t context_snapshot_index,
    v8::DeserializeInternalFieldsCallback embedder_fields_deserializer,
    v8::MicrotaskQueue* microtask_queue) {
  LWNODE_RETURN_LOCAL(Context);
}

Local<Context> v8::Context::New(
    v8::Isolate* external_isolate,
    v8::ExtensionConfiguration* extensions,
    v8::MaybeLocal<ObjectTemplate> global_template,
    v8::MaybeLocal<Value> global_object,
    DeserializeInternalFieldsCallback internal_fields_deserializer,
    v8::MicrotaskQueue* microtask_queue) {
  LWNODE_RETURN_LOCAL(Context);
}

MaybeLocal<Context> v8::Context::FromSnapshot(
    v8::Isolate* external_isolate,
    size_t context_snapshot_index,
    v8::DeserializeInternalFieldsCallback embedder_fields_deserializer,
    v8::ExtensionConfiguration* extensions,
    MaybeLocal<Value> global_object,
    v8::MicrotaskQueue* microtask_queue) {
  LWNODE_RETURN_LOCAL(Context);
}

MaybeLocal<Object> v8::Context::NewRemoteContext(
    v8::Isolate* external_isolate,
    v8::Local<ObjectTemplate> global_template,
    v8::MaybeLocal<v8::Value> global_object) {
  LWNODE_RETURN_LOCAL(Object);
}

void v8::Context::SetSecurityToken(Local<Value> token) {
  LWNODE_RETURN_VOID;
}

void v8::Context::UseDefaultSecurityToken() {
  LWNODE_RETURN_VOID;
}

Local<Value> v8::Context::GetSecurityToken() {
  LWNODE_RETURN_LOCAL(Value);
}

v8::Isolate* Context::GetIsolate() {
  LWNODE_RETURN_NULLPTR;
}

v8::Local<v8::Object> Context::Global() {
  LWNODE_RETURN_LOCAL(Object);
}

void Context::DetachGlobal() {
  LWNODE_RETURN_VOID;
}

Local<v8::Object> Context::GetExtrasBindingObject() {
  LWNODE_RETURN_LOCAL(Object);
}

void Context::AllowCodeGenerationFromStrings(bool allow) {}

bool Context::IsCodeGenerationFromStringsAllowed() {
  LWNODE_RETURN_FALSE;
}

void Context::SetErrorMessageForCodeGenerationFromStrings(Local<String> error) {
  LWNODE_RETURN_VOID;
}

void Context::SetAbortScriptExecution(
    Context::AbortScriptExecutionCallback callback) {
  LWNODE_UNIMPLEMENT;
}

Local<Value> Context::GetContinuationPreservedEmbedderData() const {
  LWNODE_RETURN_LOCAL(Value);
}

void Context::SetContinuationPreservedEmbedderData(Local<Value> data) {
  LWNODE_UNIMPLEMENT;
}

i::Address* Context::GetDataFromSnapshotOnce(size_t index) {
  LWNODE_RETURN_NULLPTR;
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Object);
}

void v8::ObjectTemplate::CheckCast(Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::FunctionTemplate::CheckCast(Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::Signature::CheckCast(Data* that) {
  LWNODE_RETURN_VOID;
}

void v8::AccessorSignature::CheckCast(Data* that) {
  LWNODE_RETURN_VOID;
}

MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Function);
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
  LWNODE_RETURN_LOCAL(Object);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  LWNODE_RETURN_FALSE;
}

Local<External> v8::External::New(Isolate* isolate, void* value) {
  LWNODE_RETURN_LOCAL(External);
}

void* External::Value() const {
  LWNODE_RETURN_NULLPTR;
}

// TODO(dcarney): throw a context free exception.
#define NEW_STRING(                                                            \
    isolate, class_name, function_name, Char, data, type, length)              \
  MaybeLocal<String> result;                                                   \
  if (length == 0) {                                                           \
    result = String::Empty(isolate);                                           \
  } else if (length > i::String::kMaxLength) {                                 \
    result = MaybeLocal<String>();                                             \
  } else {                                                                     \
    i::Isolate* i_isolate = reinterpret_cast<internal::Isolate*>(isolate);     \
    ENTER_V8_NO_SCRIPT_NO_EXCEPTION(i_isolate);                                \
    LOG_API(i_isolate, class_name, function_name);                             \
    if (length < 0) length = StringLength(data);                               \
    i::Handle<i::String> handle_result =                                       \
        NewString(                                                             \
            i_isolate->factory(), type, i::Vector<const Char>(data, length))   \
            .ToHandleChecked();                                                \
    result = Utils::ToLocal(handle_result);                                    \
  }

Local<String> String::NewFromUtf8Literal(Isolate* isolate,
                                         const char* literal,
                                         NewStringType type,
                                         int length) {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> String::NewFromUtf8(Isolate* isolate,
                                       const char* data,
                                       NewStringType type,
                                       int length) {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate,
                                          const uint8_t* data,
                                          NewStringType type,
                                          int length) {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type,
                                          int length) {
  LWNODE_RETURN_LOCAL(String);
}

Local<String> v8::String::Concat(Isolate* v8_isolate,
                                 Local<String> left,
                                 Local<String> right) {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> v8::String::NewExternalTwoByte(
    Isolate* isolate, v8::String::ExternalStringResource* resource) {
  LWNODE_RETURN_LOCAL(String);
}

MaybeLocal<String> v8::String::NewExternalOneByte(
    Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
  LWNODE_RETURN_LOCAL(String);
}

bool v8::String::MakeExternal(v8::String::ExternalStringResource* resource) {
  LWNODE_RETURN_FALSE;
}

bool v8::String::MakeExternal(
    v8::String::ExternalOneByteStringResource* resource) {
  LWNODE_RETURN_FALSE;
}

bool v8::String::CanMakeExternal() {
  LWNODE_RETURN_FALSE;
}

bool v8::String::StringEquals(Local<String> that) {
  LWNODE_RETURN_FALSE;
}

Isolate* v8::Object::GetIsolate() {
  LWNODE_RETURN_NULLPTR;
}

Local<v8::Object> v8::Object::New(Isolate* isolate) {
  LWNODE_RETURN_LOCAL(Object);
}

Local<v8::Object> v8::Object::New(Isolate* isolate,
                                  Local<Value> prototype_or_null,
                                  Local<Name>* names,
                                  Local<Value>* values,
                                  size_t length) {
  LWNODE_RETURN_LOCAL(Object);
}

Local<v8::Value> v8::NumberObject::New(Isolate* isolate, double value) {
  LWNODE_RETURN_LOCAL(Value);
}

double v8::NumberObject::ValueOf() const {
  LWNODE_RETURN_0;
}

Local<v8::Value> v8::BigIntObject::New(Isolate* isolate, int64_t value) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<v8::BigInt> v8::BigIntObject::ValueOf() const {
  LWNODE_RETURN_LOCAL(BigInt);
}

Local<v8::Value> v8::BooleanObject::New(Isolate* isolate, bool value) {
  LWNODE_RETURN_LOCAL(Value);
}

bool v8::BooleanObject::ValueOf() const {
  LWNODE_RETURN_FALSE;
}

Local<v8::Value> v8::StringObject::New(Isolate* v8_isolate,
                                       Local<String> value) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<v8::String> v8::StringObject::ValueOf() const {
  LWNODE_RETURN_LOCAL(String);
}

Local<v8::Value> v8::SymbolObject::New(Isolate* isolate, Local<Symbol> value) {
  LWNODE_RETURN_LOCAL(Value);
}

Local<v8::Symbol> v8::SymbolObject::ValueOf() const {
  LWNODE_RETURN_LOCAL(Symbol);
}

MaybeLocal<v8::Value> v8::Date::New(Local<Context> context, double time) {
  LWNODE_RETURN_LOCAL(Value);
}

double v8::Date::ValueOf() const {
  LWNODE_RETURN_0;
}

MaybeLocal<v8::RegExp> v8::RegExp::New(Local<Context> context,
                                       Local<String> pattern,
                                       Flags flags) {
  LWNODE_RETURN_LOCAL(RegExp);
}

MaybeLocal<v8::RegExp> v8::RegExp::NewWithBacktrackLimit(
    Local<Context> context,
    Local<String> pattern,
    Flags flags,
    uint32_t backtrack_limit) {
  LWNODE_RETURN_LOCAL(RegExp);
}

Local<v8::String> v8::RegExp::GetSource() const {
  LWNODE_RETURN_LOCAL(String);
}

v8::RegExp::Flags v8::RegExp::GetFlags() const {
  LWNODE_UNIMPLEMENT;
  return kNone;
}

MaybeLocal<v8::Object> v8::RegExp::Exec(Local<Context> context,
                                        Local<v8::String> subject) {
  LWNODE_RETURN_LOCAL(Object);
}

Local<v8::Array> v8::Array::New(Isolate* isolate, int length) {
  LWNODE_RETURN_LOCAL(Array);
}

Local<v8::Array> v8::Array::New(Isolate* isolate,
                                Local<Value>* elements,
                                size_t length) {
  LWNODE_RETURN_LOCAL(Array);
}

uint32_t v8::Array::Length() const {
  LWNODE_RETURN_0;
}

Local<v8::Map> v8::Map::New(Isolate* isolate) {
  LWNODE_RETURN_LOCAL(Map);
}

size_t v8::Map::Size() const {
  LWNODE_RETURN_0;
}

void Map::Clear() {}

MaybeLocal<Value> Map::Get(Local<Context> context, Local<Value> key) {
  LWNODE_RETURN_LOCAL(Value);
}

MaybeLocal<Map> Map::Set(Local<Context> context,
                         Local<Value> key,
                         Local<Value> value) {
  LWNODE_RETURN_LOCAL(Map);
}

Maybe<bool> Map::Has(Local<Context> context,
                     Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Map::Delete(Local<Context> context,
                        Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Local<Array> Map::AsArray() const {
  LWNODE_RETURN_LOCAL(Array);
}

Local<v8::Set> v8::Set::New(Isolate* isolate){LWNODE_RETURN_LOCAL(Set)}

size_t v8::Set::Size() const {
  LWNODE_RETURN_0;
}

void Set::Clear() {}

MaybeLocal<Set> Set::Add(Local<Context> context,
                         Local<Value> key){LWNODE_RETURN_LOCAL(Set)}

Maybe<bool> Set::Has(Local<Context> context,
                     Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Set::Delete(Local<Context> context,
                        Local<Value> key){LWNODE_RETURN_MAYBE(bool)}

Local<Array> Set::AsArray() const {LWNODE_RETURN_LOCAL(Array)}

MaybeLocal<Promise::Resolver> Promise::Resolver::New(Local<Context> context){
    LWNODE_RETURN_LOCAL(Promise::Resolver)}

Local<Promise> Promise::Resolver::GetPromise(){LWNODE_RETURN_LOCAL(Promise)}

Maybe<bool> Promise::Resolver::Resolve(Local<Context> context,
                                       Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

Maybe<bool> Promise::Resolver::Reject(Local<Context> context,
                                      Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

MaybeLocal<Promise> Promise::Catch(Local<Context> context,
                                   Local<Function> handler){
    LWNODE_RETURN_LOCAL(Promise)}

MaybeLocal<Promise> Promise::Then(Local<Context> context,
                                  Local<Function> handler){
    LWNODE_RETURN_LOCAL(Promise)}

MaybeLocal<Promise> Promise::Then(Local<Context> context,
                                  Local<Function> on_fulfilled,
                                  Local<Function> on_rejected) {
  LWNODE_RETURN_LOCAL(Promise)
}

bool Promise::HasHandler() {
  LWNODE_RETURN_FALSE;
}

Local<Value> Promise::Result(){LWNODE_RETURN_LOCAL(Value)}

Promise::PromiseState Promise::State() {
  LWNODE_UNIMPLEMENT;
  return kRejected;
}

void Promise::MarkAsHandled() {}

Local<Value> Proxy::GetTarget(){LWNODE_RETURN_LOCAL(Value)}

Local<Value> Proxy::GetHandler() {
  LWNODE_RETURN_LOCAL(Value)
}

bool Proxy::IsRevoked() {
  LWNODE_RETURN_FALSE;
}

void Proxy::Revoke() {}

MaybeLocal<Proxy> Proxy::New(Local<Context> context,
                             Local<Object> local_target,
                             Local<Object> local_handler){
    LWNODE_RETURN_LOCAL(Proxy)}

CompiledWasmModule::CompiledWasmModule(
    std::shared_ptr<internal::wasm::NativeModule> native_module,
    const char* source_url,
    size_t url_length)
    : native_module_(std::move(native_module)) {}

OwnedBuffer CompiledWasmModule::Serialize() {
  LWNODE_UNIMPLEMENT;
  std::unique_ptr<uint8_t[]> buffer(new uint8_t[0]);
  return {std::move(buffer), 0};
}

MemorySpan<const uint8_t> CompiledWasmModule::GetWireBytesRef() {
  LWNODE_UNIMPLEMENT;
  return {nullptr, 0};
}

CompiledWasmModule WasmModuleObject::GetCompiledModule() {
  LWNODE_UNIMPLEMENT;
  return CompiledWasmModule(nullptr, nullptr, 0);
}

MaybeLocal<WasmModuleObject> WasmModuleObject::FromCompiledModule(
    Isolate* isolate, const CompiledWasmModule& compiled_module){
    LWNODE_RETURN_LOCAL(WasmModuleObject)}

WasmModuleObjectBuilderStreaming::WasmModuleObjectBuilderStreaming(
    Isolate* isolate) {}

Local<Promise> WasmModuleObjectBuilderStreaming::GetPromise() {
  return {};
}

void WasmModuleObjectBuilderStreaming::OnBytesReceived(const uint8_t* bytes,
                                                       size_t size) {}

void WasmModuleObjectBuilderStreaming::Finish() {}

void WasmModuleObjectBuilderStreaming::Abort(MaybeLocal<Value> exception) {}

void* v8::ArrayBuffer::Allocator::Reallocate(void* data,
                                             size_t old_length,
                                             size_t new_length) {
  LWNODE_RETURN_NULLPTR;
}

// static
v8::ArrayBuffer::Allocator* v8::ArrayBuffer::Allocator::NewDefaultAllocator() {
  return new ArrayBufferAllocator();
}

bool v8::ArrayBuffer::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

bool v8::ArrayBuffer::IsDetachable() const {
  LWNODE_RETURN_FALSE;
}

v8::ArrayBuffer::Contents::Contents(void* data,
                                    size_t byte_length,
                                    void* allocation_base,
                                    size_t allocation_length,
                                    Allocator::AllocationMode allocation_mode,
                                    DeleterCallback deleter,
                                    void* deleter_data)
    : data_(data),
      byte_length_(byte_length),
      allocation_base_(allocation_base),
      allocation_length_(allocation_length),
      allocation_mode_(allocation_mode),
      deleter_(deleter),
      deleter_data_(deleter_data) {}

v8::ArrayBuffer::Contents v8::ArrayBuffer::Externalize() {
  return GetContents(true);
}

void v8::ArrayBuffer::Externalize(
    const std::shared_ptr<BackingStore>& backing_store) {}

v8::ArrayBuffer::Contents v8::ArrayBuffer::GetContents() {
  return GetContents(false);
}

v8::ArrayBuffer::Contents v8::ArrayBuffer::GetContents(bool externalize) {
  LWNODE_UNIMPLEMENT;
  return ArrayBuffer::Contents();
}

void v8::ArrayBuffer::Detach() {}

size_t v8::ArrayBuffer::ByteLength() const {
  LWNODE_RETURN_0;
}

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* isolate, size_t byte_length) {
  LWNODE_RETURN_LOCAL(ArrayBuffer);
}

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* isolate,
                                        void* data,
                                        size_t byte_length,
                                        ArrayBufferCreationMode mode) {
  LWNODE_RETURN_LOCAL(ArrayBuffer);
}

Local<ArrayBuffer> v8::ArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  LWNODE_RETURN_LOCAL(ArrayBuffer);
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    Isolate* isolate, size_t byte_length) {
  LWNODE_RETURN_NULLPTR;
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    void* data,
    size_t byte_length,
    v8::BackingStore::DeleterCallback deleter,
    void* deleter_data) {
  LWNODE_RETURN_NULLPTR;
}

Local<ArrayBuffer> v8::ArrayBufferView::Buffer() {
  LWNODE_RETURN_LOCAL(ArrayBuffer);
}

size_t v8::ArrayBufferView::CopyContents(void* dest, size_t byte_length) {
  LWNODE_RETURN_0;
}

bool v8::ArrayBufferView::HasBuffer() const {
  LWNODE_RETURN_FALSE;
}

size_t v8::ArrayBufferView::ByteOffset() {
  LWNODE_RETURN_0;
}

size_t v8::ArrayBufferView::ByteLength() {
  LWNODE_RETURN_0;
}

size_t v8::TypedArray::Length() {
  LWNODE_RETURN_0;
}

#define TYPED_ARRAY_NEW(Type, type, TYPE, ctype)                               \
  Local<Type##Array> Type##Array::New(                                         \
      Local<ArrayBuffer> array_buffer, size_t byte_offset, size_t length) {    \
    LWNODE_RETURN_LOCAL(Type##Array);                                          \
  }                                                                            \
  Local<Type##Array> Type##Array::New(                                         \
      Local<SharedArrayBuffer> shared_array_buffer,                            \
      size_t byte_offset,                                                      \
      size_t length) {                                                         \
    LWNODE_RETURN_LOCAL(Type##Array);                                          \
  }

TYPED_ARRAYS(TYPED_ARRAY_NEW)
#undef TYPED_ARRAY_NEW

Local<DataView> DataView::New(Local<ArrayBuffer> array_buffer,
                              size_t byte_offset,
                              size_t byte_length) {
  LWNODE_RETURN_LOCAL(DataView);
}

Local<DataView> DataView::New(Local<SharedArrayBuffer> shared_array_buffer,
                              size_t byte_offset,
                              size_t byte_length) {
  LWNODE_RETURN_LOCAL(DataView);
}

bool v8::SharedArrayBuffer::IsExternal() const {
  LWNODE_RETURN_FALSE;
}

v8::SharedArrayBuffer::Contents::Contents(
    void* data,
    size_t byte_length,
    void* allocation_base,
    size_t allocation_length,
    Allocator::AllocationMode allocation_mode,
    DeleterCallback deleter,
    void* deleter_data)
    : data_(data),
      byte_length_(byte_length),
      allocation_base_(allocation_base),
      allocation_length_(allocation_length),
      allocation_mode_(allocation_mode),
      deleter_(deleter),
      deleter_data_(deleter_data) {
  LWNODE_UNIMPLEMENT;
}

v8::SharedArrayBuffer::Contents v8::SharedArrayBuffer::Externalize() {
  return GetContents(true);
}

void v8::SharedArrayBuffer::Externalize(
    const std::shared_ptr<BackingStore>& backing_store) {}

v8::SharedArrayBuffer::Contents v8::SharedArrayBuffer::GetContents() {
  return GetContents(false);
}

v8::SharedArrayBuffer::Contents v8::SharedArrayBuffer::GetContents(
    bool externalize) {
  return v8::SharedArrayBuffer::Contents();
}

size_t v8::SharedArrayBuffer::ByteLength() const {
  LWNODE_RETURN_0;
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(Isolate* isolate,
                                                    size_t byte_length) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer);
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
    Isolate* isolate,
    void* data,
    size_t byte_length,
    ArrayBufferCreationMode mode) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer);
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer);
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
    Isolate* isolate,
    const SharedArrayBuffer::Contents& contents,
    ArrayBufferCreationMode mode) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer);
}

std::unique_ptr<v8::BackingStore> v8::SharedArrayBuffer::NewBackingStore(
    Isolate* isolate, size_t byte_length) {
  LWNODE_RETURN_NULLPTR;
}

std::unique_ptr<v8::BackingStore> v8::SharedArrayBuffer::NewBackingStore(
    void* data,
    size_t byte_length,
    v8::BackingStore::DeleterCallback deleter,
    void* deleter_data) {
  LWNODE_RETURN_NULLPTR;
}

Local<Symbol> v8::Symbol::New(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Symbol);
}

Local<Symbol> v8::Symbol::For(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Symbol);
}

Local<Symbol> v8::Symbol::ForApi(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Symbol);
}

#define WELL_KNOWN_SYMBOLS(V)                                                  \
  V(AsyncIterator, async_iterator)                                             \
  V(HasInstance, has_instance)                                                 \
  V(IsConcatSpreadable, is_concat_spreadable)                                  \
  V(Iterator, iterator)                                                        \
  V(Match, match)                                                              \
  V(Replace, replace)                                                          \
  V(Search, search)                                                            \
  V(Split, split)                                                              \
  V(ToPrimitive, to_primitive)                                                 \
  V(ToStringTag, to_string_tag)                                                \
  V(Unscopables, unscopables)

#define SYMBOL_GETTER(Name, name)                                              \
  Local<Symbol> v8::Symbol::Get##Name(Isolate* isolate) {                      \
    return Local<Symbol>();                                                    \
  }

WELL_KNOWN_SYMBOLS(SYMBOL_GETTER)

#undef SYMBOL_GETTER
#undef WELL_KNOWN_SYMBOLS

Local<Private> v8::Private::New(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Private);
}

Local<Private> v8::Private::ForApi(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Private);
}

Local<Number> v8::Number::New(Isolate* isolate,
                              double value){LWNODE_RETURN_LOCAL(Number)}

Local<Integer> v8::Integer::New(Isolate* isolate, int32_t value) {
  LWNODE_RETURN_LOCAL(Integer);
}

Local<Integer> v8::Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
  LWNODE_RETURN_LOCAL(Integer);
}

Local<BigInt> v8::BigInt::New(Isolate* isolate, int64_t value) {
  LWNODE_RETURN_LOCAL(BigInt);
}

Local<BigInt> v8::BigInt::NewFromUnsigned(Isolate* isolate, uint64_t value) {
  LWNODE_RETURN_LOCAL(BigInt);
}

MaybeLocal<BigInt> v8::BigInt::NewFromWords(Local<Context> context,
                                            int sign_bit,
                                            int word_count,
                                            const uint64_t* words) {
  LWNODE_RETURN_LOCAL(BigInt);
}

uint64_t v8::BigInt::Uint64Value(bool* lossless) const {
  LWNODE_RETURN_0;
}

int64_t v8::BigInt::Int64Value(bool* lossless) const {
  LWNODE_RETURN_0;
}

int BigInt::WordCount() const {
  LWNODE_RETURN_0;
}

void BigInt::ToWordsArray(int* sign_bit,
                          int* word_count,
                          uint64_t* words) const {}

void Isolate::ReportExternalAllocationLimitReached() {}

// Node v14.x ABI compat dummy.
void Isolate::CheckMemoryPressure() {
  LWNODE_RETURN_VOID;
}

HeapProfiler* Isolate::GetHeapProfiler() {
  LWNODE_RETURN_NULLPTR;
}

void Isolate::SetIdle(bool is_idle) {
  LWNODE_RETURN_VOID;
}

ArrayBuffer::Allocator* Isolate::GetArrayBufferAllocator() {
  LWNODE_RETURN_NULLPTR;
}

bool Isolate::InContext() {
  LWNODE_RETURN_FALSE;
}

void Isolate::ClearKeptObjects() {
  LWNODE_RETURN_VOID;
}

v8::Local<v8::Context> Isolate::GetCurrentContext() {
  LWNODE_RETURN_LOCAL(Context);
}

v8::Local<v8::Context> Isolate::GetEnteredContext() {
  LWNODE_RETURN_LOCAL(Context);
}

v8::Local<v8::Context> Isolate::GetEnteredOrMicrotaskContext() {
  LWNODE_RETURN_LOCAL(Context);
}

v8::Local<v8::Context> Isolate::GetIncumbentContext() {
  LWNODE_RETURN_LOCAL(Context);
}

v8::Local<Value> Isolate::ThrowException(v8::Local<v8::Value> value) {
  LWNODE_RETURN_LOCAL(Value);
}

void Isolate::AddGCPrologueCallback(GCCallbackWithData callback,
                                    void* data,
                                    GCType gc_type) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveGCPrologueCallback(GCCallbackWithData callback,
                                       void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddGCEpilogueCallback(GCCallbackWithData callback,
                                    void* data,
                                    GCType gc_type) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveGCEpilogueCallback(GCCallbackWithData callback,
                                       void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddGCPrologueCallback(GCCallback callback, GCType gc_type) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveGCPrologueCallback(GCCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddGCEpilogueCallback(GCCallback callback, GCType gc_type) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveGCEpilogueCallback(GCCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetEmbedderHeapTracer(EmbedderHeapTracer* tracer) {
  LWNODE_RETURN_VOID;
}

EmbedderHeapTracer* Isolate::GetEmbedderHeapTracer() {
  LWNODE_RETURN_NULLPTR;
}

void Isolate::SetGetExternallyAllocatedMemoryInBytesCallback(
    GetExternallyAllocatedMemoryInBytesCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::TerminateExecution() {
  LWNODE_RETURN_VOID;
}

bool Isolate::IsExecutionTerminating() {
  LWNODE_RETURN_FALSE;
}

void Isolate::CancelTerminateExecution() {
  LWNODE_RETURN_VOID;
}

void Isolate::RequestInterrupt(InterruptCallback callback, void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::RequestGarbageCollectionForTesting(GarbageCollectionType type) {
  LWNODE_RETURN_VOID;
}

Isolate* Isolate::GetCurrent() {
  LWNODE_RETURN_NULLPTR;
}

// static
Isolate* Isolate::Allocate() {
  LWNODE_RETURN_NULLPTR;
}

// static
// This is separate so that tests can provide a different |isolate|.
void Isolate::Initialize(Isolate* isolate,
                         const v8::Isolate::CreateParams& params) {
  LWNODE_RETURN_VOID;
}

Isolate* Isolate::New(const Isolate::CreateParams& params) {
  LWNODE_RETURN_NULLPTR;
}

void Isolate::Dispose() {
  LWNODE_RETURN_VOID;
}

void Isolate::DumpAndResetStats() {
  LWNODE_RETURN_VOID;
}

void Isolate::DiscardThreadSpecificMetadata() {
  LWNODE_RETURN_VOID;
}

void Isolate::Enter() {
  LWNODE_RETURN_VOID;
}

void Isolate::Exit() {
  LWNODE_RETURN_VOID;
}

void Isolate::SetAbortOnUncaughtExceptionCallback(
    AbortOnUncaughtExceptionCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetHostCleanupFinalizationGroupCallback(
    HostCleanupFinalizationGroupCallback callback) {
  LWNODE_RETURN_VOID;
}

Maybe<bool> FinalizationGroup::Cleanup(
    Local<FinalizationGroup> finalization_group) {
  LWNODE_RETURN_MAYBE(bool);
}

void Isolate::SetHostImportModuleDynamicallyCallback(
    HostImportModuleDynamicallyCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetHostInitializeImportMetaObjectCallback(
    HostInitializeImportMetaObjectCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetPrepareStackTraceCallback(PrepareStackTraceCallback callback) {
  LWNODE_RETURN_VOID;
}

Isolate::DisallowJavascriptExecutionScope::DisallowJavascriptExecutionScope(
    Isolate* isolate,
    Isolate::DisallowJavascriptExecutionScope::OnFailure on_failure)
    : on_failure_(on_failure) {
  LWNODE_RETURN_VOID;
}

Isolate::DisallowJavascriptExecutionScope::~DisallowJavascriptExecutionScope() {
  LWNODE_RETURN_VOID;
}

Isolate::AllowJavascriptExecutionScope::AllowJavascriptExecutionScope(
    Isolate* isolate) {
  LWNODE_UNIMPLEMENT;
}

Isolate::AllowJavascriptExecutionScope::~AllowJavascriptExecutionScope() {
  LWNODE_UNIMPLEMENT;
}

Isolate::SuppressMicrotaskExecutionScope::SuppressMicrotaskExecutionScope(
    Isolate* isolate, MicrotaskQueue* microtask_queue)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)),
      microtask_queue_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

Isolate::SuppressMicrotaskExecutionScope::~SuppressMicrotaskExecutionScope() {}

Isolate::SafeForTerminationScope::SafeForTerminationScope(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<i::Isolate*>(isolate)), prev_value_(nullptr) {
  LWNODE_UNIMPLEMENT;
}

Isolate::SafeForTerminationScope::~SafeForTerminationScope() {
  LWNODE_UNIMPLEMENT;
}

i::Address* Isolate::GetDataFromSnapshotOnce(size_t index) {
  LWNODE_RETURN_NULLPTR;
}

void Isolate::GetHeapStatistics(HeapStatistics* heap_statistics) {
  LWNODE_RETURN_VOID;
}

size_t Isolate::NumberOfHeapSpaces() {
  LWNODE_RETURN_0;
}

bool Isolate::GetHeapSpaceStatistics(HeapSpaceStatistics* space_statistics,
                                     size_t index) {
  LWNODE_RETURN_FALSE;
}

size_t Isolate::NumberOfTrackedHeapObjectTypes() {
  LWNODE_RETURN_0;
}

bool Isolate::GetHeapObjectStatisticsAtLastGC(
    HeapObjectStatistics* object_statistics, size_t type_index) {
  LWNODE_RETURN_FALSE;
}

bool Isolate::GetHeapCodeAndMetadataStatistics(
    HeapCodeStatistics* code_statistics) {
  LWNODE_RETURN_FALSE;
}

v8::MaybeLocal<v8::Promise> Isolate::MeasureMemory(
    v8::Local<v8::Context> context, MeasureMemoryMode mode) {
  LWNODE_RETURN_LOCAL(Promise);
}

bool Isolate::MeasureMemory(std::unique_ptr<MeasureMemoryDelegate> delegate,
                            MeasureMemoryExecution execution) {
  LWNODE_RETURN_FALSE;
}

std::unique_ptr<MeasureMemoryDelegate> MeasureMemoryDelegate::Default(
    Isolate* isolate,
    Local<Context> context,
    Local<Promise::Resolver> promise_resolver,
    MeasureMemoryMode mode) {
  LWNODE_RETURN_NULLPTR;
}

void Isolate::GetStackSample(const RegisterState& state,
                             void** frames,
                             size_t frames_limit,
                             SampleInfo* sample_info) {
  LWNODE_RETURN_VOID;
}

size_t Isolate::NumberOfPhantomHandleResetsSinceLastCall() {
  LWNODE_RETURN_0;
}

void Isolate::SetEventLogger(LogEventCallback that) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddBeforeCallEnteredCallback(BeforeCallEnteredCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveBeforeCallEnteredCallback(
    BeforeCallEnteredCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddCallCompletedCallback(CallCompletedCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveCallCompletedCallback(CallCompletedCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::AtomicsWaitWakeHandle::Wake() {
  LWNODE_RETURN_VOID;
}

void Isolate::SetAtomicsWaitCallback(AtomicsWaitCallback callback, void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetPromiseHook(PromiseHook hook) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetPromiseRejectCallback(PromiseRejectCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::PerformMicrotaskCheckpoint() {
  LWNODE_RETURN_VOID;
}

void Isolate::EnqueueMicrotask(Local<Function> v8_function) {
  LWNODE_RETURN_VOID;
}

void Isolate::EnqueueMicrotask(MicrotaskCallback callback, void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
  LWNODE_RETURN_VOID;
}

MicrotasksPolicy Isolate::GetMicrotasksPolicy() const {
  LWNODE_UNIMPLEMENT;
  return MicrotasksPolicy();
}

void Isolate::AddMicrotasksCompletedCallback(
    MicrotasksCompletedCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::AddMicrotasksCompletedCallback(
    MicrotasksCompletedCallbackWithData callback, void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveMicrotasksCompletedCallback(
    MicrotasksCompletedCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveMicrotasksCompletedCallback(
    MicrotasksCompletedCallbackWithData callback, void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetUseCounterCallback(UseCounterCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetCounterFunction(CounterLookupCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetCreateHistogramFunction(CreateHistogramCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetAddHistogramSampleFunction(
    AddHistogramSampleCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetAddCrashKeyCallback(AddCrashKeyCallback callback) {
  LWNODE_RETURN_VOID;
}

bool Isolate::IdleNotificationDeadline(double deadline_in_seconds) {
  LWNODE_RETURN_FALSE;
}

void Isolate::LowMemoryNotification() {
  LWNODE_RETURN_VOID;
}

int Isolate::ContextDisposedNotification(bool dependant_context) {
  LWNODE_RETURN_0;
}

void Isolate::IsolateInForegroundNotification() {
  LWNODE_RETURN_VOID;
}

void Isolate::IsolateInBackgroundNotification() {
  LWNODE_RETURN_VOID;
}

void Isolate::MemoryPressureNotification(MemoryPressureLevel level) {
  LWNODE_RETURN_VOID;
}

void Isolate::EnableMemorySavingsMode() {
  LWNODE_RETURN_VOID;
}

void Isolate::DisableMemorySavingsMode() {
  LWNODE_RETURN_VOID;
}

void Isolate::SetRAILMode(RAILMode rail_mode) {
  LWNODE_RETURN_VOID;
}

void Isolate::IncreaseHeapLimitForDebugging() {
  // No-op.
  LWNODE_RETURN_VOID;
}

void Isolate::RestoreOriginalHeapLimit() {
  // No-op.
  LWNODE_RETURN_VOID;
}

bool Isolate::IsHeapLimitIncreasedForDebugging() {
  LWNODE_RETURN_FALSE;
}

void Isolate::SetJitCodeEventHandler(JitCodeEventOptions options,
                                     JitCodeEventHandler event_handler) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetStackLimit(uintptr_t stack_limit) {
  LWNODE_RETURN_VOID;
}

void Isolate::GetCodeRange(void** start, size_t* length_in_bytes) {
  LWNODE_RETURN_VOID;
}

UnwindState Isolate::GetUnwindState() {
  UnwindState unwind_state;
  LWNODE_UNIMPLEMENT;

  return unwind_state;
}

JSEntryStubs Isolate::GetJSEntryStubs() {
  JSEntryStubs entry_stubs;

  LWNODE_UNIMPLEMENT;

  return entry_stubs;
}

size_t Isolate::CopyCodePages(size_t capacity, MemoryRange* code_pages_out) {
  LWNODE_RETURN_0;
}

#define CALLBACK_SETTER(ExternalName, Type, InternalName)                      \
  void Isolate::Set##ExternalName(Type callback) { LWNODE_RETURN_VOID; }

CALLBACK_SETTER(FatalErrorHandler, FatalErrorCallback, exception_behavior)
CALLBACK_SETTER(OOMErrorHandler, OOMErrorCallback, oom_behavior)
CALLBACK_SETTER(AllowCodeGenerationFromStringsCallback,
                AllowCodeGenerationFromStringsCallback,
                allow_code_gen_callback)
CALLBACK_SETTER(ModifyCodeGenerationFromStringsCallback,
                ModifyCodeGenerationFromStringsCallback,
                modify_code_gen_callback)
CALLBACK_SETTER(AllowWasmCodeGenerationCallback,
                AllowWasmCodeGenerationCallback,
                allow_wasm_code_gen_callback)

CALLBACK_SETTER(WasmModuleCallback, ExtensionCallback, wasm_module_callback)
CALLBACK_SETTER(WasmInstanceCallback, ExtensionCallback, wasm_instance_callback)

CALLBACK_SETTER(WasmStreamingCallback,
                WasmStreamingCallback,
                wasm_streaming_callback)

CALLBACK_SETTER(WasmThreadsEnabledCallback,
                WasmThreadsEnabledCallback,
                wasm_threads_enabled_callback)

CALLBACK_SETTER(WasmLoadSourceMapCallback,
                WasmLoadSourceMapCallback,
                wasm_load_source_map_callback)

CALLBACK_SETTER(WasmSimdEnabledCallback,
                WasmSimdEnabledCallback,
                wasm_simd_enabled_callback)

void Isolate::AddNearHeapLimitCallback(v8::NearHeapLimitCallback callback,
                                       void* data) {
  LWNODE_RETURN_VOID;
}

void Isolate::RemoveNearHeapLimitCallback(v8::NearHeapLimitCallback callback,
                                          size_t heap_limit) {
  LWNODE_RETURN_VOID;
}

void Isolate::AutomaticallyRestoreInitialHeapLimit(double threshold_percent) {
  LWNODE_RETURN_VOID;
}

bool Isolate::IsDead() {
  LWNODE_RETURN_FALSE;
}

bool Isolate::AddMessageListener(MessageCallback that, Local<Value> data) {
  LWNODE_RETURN_FALSE;
}

bool Isolate::AddMessageListenerWithErrorLevel(MessageCallback that,
                                               int message_levels,
                                               Local<Value> data) {
  LWNODE_RETURN_FALSE;
}

void Isolate::RemoveMessageListeners(MessageCallback that) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetFailedAccessCheckCallbackFunction(
    FailedAccessCheckCallback callback) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetCaptureStackTraceForUncaughtExceptions(
    bool capture, int frame_limit, StackTrace::StackTraceOptions options) {
  LWNODE_RETURN_VOID;
}

void Isolate::VisitExternalResources(ExternalResourceVisitor* visitor) {
  LWNODE_RETURN_VOID;
}

bool Isolate::IsInUse() {
  LWNODE_RETURN_FALSE;
}

void Isolate::VisitHandlesWithClassIds(PersistentHandleVisitor* visitor) {
  LWNODE_RETURN_VOID;
}

void Isolate::VisitWeakHandles(PersistentHandleVisitor* visitor) {
  LWNODE_RETURN_VOID;
}

void Isolate::SetAllowAtomicsWait(bool allow) {
  LWNODE_RETURN_VOID;
}

void v8::Isolate::DateTimeConfigurationChangeNotification(
    TimeZoneDetection time_zone_detection) {
  LWNODE_RETURN_VOID;
}

void v8::Isolate::LocaleConfigurationChangeNotification() {
  LWNODE_RETURN_VOID;
}

// static
std::unique_ptr<MicrotaskQueue> MicrotaskQueue::New(Isolate* isolate,
                                                    MicrotasksPolicy policy) {
  LWNODE_UNIMPLEMENT;
  return std::unique_ptr<MicrotaskQueue>();
}

MicrotasksScope::MicrotasksScope(Isolate* isolate, MicrotasksScope::Type type)
    : MicrotasksScope(isolate, nullptr, type) {}

MicrotasksScope::MicrotasksScope(Isolate* isolate,
                                 MicrotaskQueue* microtask_queue,
                                 MicrotasksScope::Type type)
    : isolate_(nullptr), microtask_queue_(nullptr), run_(false) {
  LWNODE_UNIMPLEMENT;
}

MicrotasksScope::~MicrotasksScope() {
  LWNODE_UNIMPLEMENT;
}

void MicrotasksScope::PerformCheckpoint(Isolate* v8_isolate) {
  LWNODE_RETURN_VOID;
}

int MicrotasksScope::GetCurrentDepth(Isolate* v8_isolate) {
  LWNODE_RETURN_0;
}

bool MicrotasksScope::IsRunningMicrotasks(Isolate* v8_isolate) {
  LWNODE_RETURN_FALSE;
}

String::Utf8Value::Utf8Value(v8::Isolate* isolate, v8::Local<v8::Value> obj)
    : str_(nullptr), length_(0) {
  LWNODE_UNIMPLEMENT;
}

String::Utf8Value::~Utf8Value() {
  LWNODE_UNIMPLEMENT;
}

String::Value::Value(v8::Isolate* isolate, v8::Local<v8::Value> obj)
    : str_(nullptr), length_(0) {
  LWNODE_UNIMPLEMENT;
}

String::Value::~Value() {
  LWNODE_UNIMPLEMENT;
}

#define DEFINE_ERROR(NAME, name)                                               \
  Local<Value> Exception::NAME(v8::Local<v8::String> raw_message) {            \
    LWNODE_RETURN_LOCAL(Value);                                                \
  }

DEFINE_ERROR(RangeError, range_error)
DEFINE_ERROR(ReferenceError, reference_error)
DEFINE_ERROR(SyntaxError, syntax_error)
DEFINE_ERROR(TypeError, type_error)
DEFINE_ERROR(WasmCompileError, wasm_compile_error)
DEFINE_ERROR(WasmLinkError, wasm_link_error)
DEFINE_ERROR(WasmRuntimeError, wasm_runtime_error)
DEFINE_ERROR(Error, error)

#undef DEFINE_ERROR

Local<Message> Exception::CreateMessage(Isolate* isolate,
                                        Local<Value> exception) {
  LWNODE_RETURN_LOCAL(Message);
}

Local<StackTrace> Exception::GetStackTrace(Local<Value> exception) {
  LWNODE_RETURN_LOCAL(StackTrace);
}

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
  LWNODE_RETURN_VOID;
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
  LWNODE_RETURN_VOID;
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
