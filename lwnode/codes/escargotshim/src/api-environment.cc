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
#include "escargotshim-base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {
// --- E n v i r o n m e n t ---

void v8::V8::InitializePlatform(Platform* platform) {
  /* NOTHING TO DO */
}

void v8::V8::ShutdownPlatform() {
  /* NOTHING TO DO */
}

bool v8::V8::Initialize(const int build_config) {
  Engine::Initialize();
  return true;
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
  LWNODE_CALL_TRACE;
}

void v8::V8::SetReturnAddressLocationResolver(
    ReturnAddressLocationResolver return_address_resolver) {
  LWNODE_RETURN_VOID;
}

bool v8::V8::Dispose() {
  Engine::Dispose();
  return true;
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
  return "@todo NOT DEFINED YET";
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
  if (extensions || !global_template.IsEmpty() || !global_object.IsEmpty() ||
      microtask_queue) {
    LWNODE_UNIMPLEMENT;
  }

  auto _context =
      ValueWrap::createContext(IsolateWrap::fromV8(external_isolate));
  auto l = Local<Context>::New(external_isolate, _context);
  return l;
  return Local<Context>::New(external_isolate, _context);
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
    if (length < 0) length = stringLength(data);                               \
    i::Handle<i::String> handle_result =                                       \
        NewString(                                                             \
            i_isolate->factory(), type, i::Vector<const Char>(data, length))   \
            .ToHandleChecked();                                                \
    result = Utils::ToLocal(handle_result);                                    \
  }

Local<String> String::_Empty(Isolate* isolate) {
  return Local<String>::New(isolate,
                            ValueWrap::createValue(StringRef::emptyString()));
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
  MaybeLocal<String> result;

  if (length == 0) {
    result = String::Empty(isolate);
  } else if (length > v8::String::kMaxLength) {
    result = MaybeLocal<String>();
  } else {
    if (length < 0) length = stringLength(data);
    StringRef* __source = StringRef::createFromUTF8(data, length);
    result = Local<String>::New(isolate, ValueWrap::createValue(__source));
  }

  return result;
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate,
                                          const uint8_t* data,
                                          NewStringType type,
                                          int length) {
  MaybeLocal<String> result;

  if (length == 0) {
    result = String::Empty(isolate);
  } else if (length > v8::String::kMaxLength) {
    result = MaybeLocal<String>();
  } else {
    if (length < 0) length = stringLength(data);
    StringRef* __source = StringRef::createFromLatin1(data, length);
    result = Local<String>::New(isolate, ValueWrap::createValue(__source));
  }

  return result;
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type,
                                          int length) {
  MaybeLocal<String> result;

  if (length == 0) {
    result = String::Empty(isolate);
  } else if (length > v8::String::kMaxLength) {
    result = MaybeLocal<String>();
  } else {
    if (length < 0) length = stringLength(data);
    StringRef* __source = StringRef::createFromUTF16(
        reinterpret_cast<const char16_t*>(data), length);
    result = Local<String>::New(isolate, ValueWrap::createValue(__source));
  }

  return result;
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
  API_ENTER_NO_EXCEPTION(isolate);

  StringRef* __name;

  if (name.IsEmpty()) {
    __name = StringRef::emptyString();
  } else {
    LWNODE_CHECK(VAL(*name)->value()->isString());
    __name = VAL(*name)->value()->asString();
  }
  auto _value = ValueWrap::createValue(SymbolRef::create(__name));

  return Local<Symbol>::New(isolate, _value);
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
  API_ENTER_NO_EXCEPTION(isolate);
  StringRef* __name;

  if (name.IsEmpty()) {
    __name = StringRef::emptyString();
  } else {
    LWNODE_CHECK(VAL(*name)->value()->isString());
    __name = VAL(*name)->value()->asString();
  }
  // @note
  // A private symbol in V8 is similar to a Symbol, except that it’s not
  // enumerable and doesn’t leak to userspace JavaScript.
  // @todo For now, we ignore the private attribute and use a normal Symbol
  // instead.
  auto _value = ValueWrap::createValue(SymbolRef::create(__name));

  return Local<Private>::New(isolate, _value);
}

Local<Private> v8::Private::ForApi(Isolate* isolate, Local<String> name) {
  LWNODE_RETURN_LOCAL(Private);
}

Local<Number> v8::Number::New(Isolate* isolate, double value) {
  auto _number = ValueWrap::createValue(Escargot::ValueRef::create(value));
  return Local<Integer>::New(isolate, _number);
}

Local<Integer> v8::Integer::New(Isolate* isolate, int32_t value) {
  auto _integer = ValueWrap::createValue(Escargot::ValueRef::create(value));
  return Local<Integer>::New(isolate, _integer);
}

Local<Integer> v8::Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
  auto _integer = ValueWrap::createValue(Escargot::ValueRef::create(value));
  return Local<Integer>::New(isolate, _integer);
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
  return IsolateWrap::fromV8(this)->array_buffer_allocator();
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
  return IsolateWrap::toV8(IsolateWrap::New());
}

// static
// This is separate so that tests can provide a different |isolate|.
void Isolate::Initialize(Isolate* isolate,
                         const v8::Isolate::CreateParams& params) {
  IsolateWrap::fromV8(isolate)->Initialize(params);
}

Isolate* Isolate::New(const Isolate::CreateParams& params) {
  Isolate* isolate = Allocate();
  Initialize(isolate, params);
  return isolate;
}

void Isolate::Dispose() {
  IsolateWrap::fromV8(this)->Dispose();
}

void Isolate::DumpAndResetStats() {
  LWNODE_RETURN_VOID;
}

void Isolate::DiscardThreadSpecificMetadata() {
  LWNODE_RETURN_VOID;
}

void Isolate::Enter() {
  IsolateWrap::fromV8(this)->Enter();
}

void Isolate::Exit() {
  IsolateWrap::fromV8(this)->Exit();
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
  auto _isolate = IsolateWrap::fromV8(isolate);
  auto _context = _isolate->CurrentContext();
  auto _value = VAL(*obj);

  auto r = Evaluator::execute(
      _context->get(),
      [](ExecutionStateRef* state, ValueRef* value) -> ValueRef* {
        return value->toString(state);
      },
      _value->value());

  if (!r.isSuccessful()) {
    return;
  }

  auto str = r.result->asString()->toStdUTF8String();

  length_ = str.size();
  if (length_ == 0) {
    return;
  }

  str_ = new char[length_ + 1];
  strncpy(str_, str.data(), length_);
  str_[length_] = '\0';
}

String::Utf8Value::~Utf8Value() {
  delete[] str_;
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
}  // namespace v8
