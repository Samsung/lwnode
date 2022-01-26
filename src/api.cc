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

#include "api.h"

#include <malloc.h>  // for malloc_trim
#include <sstream>

#include "base.h"

using namespace Escargot;
using namespace EscargotShim;

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

static Flag validFlags[] = {
    // v8 flags
    Flag("--expose-gc", Flag::Type::ExposeGC),
    Flag("--use-strict", Flag::Type::UseStrict),
    Flag("--off-idlegc", Flag::Type::DisableIdleGC),
    Flag("--harmony-top-level-await", Flag::Type::TopLevelWait),
    Flag("--allow-code-generation-from-strings",
         Flag::Type::AllowCodeGenerationFromString),
    Flag("--abort-on-uncaught-exception", Flag::Type::AbortOnUncaughtException),
    Flag("--expose-externalize-string", Flag::Type::ExposeExternalizeString),
    Flag("--trace-debug", Flag::Type::LWNodeOther, true),
    Flag("--debug", Flag::Type::LWNodeOther, true),
    Flag("--stack-size=", Flag::Type::LWNodeOther, true),
    Flag("--nolazy", Flag::Type::LWNodeOther, true),
    // lwnode flags
    Flag("--trace-gc", Flag::Type::TraceGC),
    Flag("--trace-call=", Flag::Type::TraceCall, true),
    Flag("--internal-log", Flag::Type::InternalLog),
};

static Flag::Type findFlag(const std::string& name) {
  std::string normalized = name;
  std::replace(normalized.begin(), normalized.end(), '_', '-');

  for (auto flag : validFlags) {
    if ((flag.name() == normalized) || flag.isPrefixOf(normalized)) {
      return flag.type();
    }
  }

  return Flag::Type::Empty;
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  flag_t userOptions = Flag::Type::Empty;

  for (int i = 1; i < *argc; i++) {
    std::string userOption = argv[i];
    bool isValidToRemove = false;

    // @check https://github.sec.samsung.net/lws/node-escargot/issues/394
    flag_t nameType = findFlag(userOption);
    if (nameType == Flag::Type::Empty) {
      LWNODE_DLOG_WARN("'%s' flag is ignored", userOption.c_str());
      continue;
    }

    if (nameType != Flag::Type::LWNodeOther) {
      userOptions |= nameType;
    }

    if (nameType == Flag::Type::TraceCall) {
      std::string traceCallOption = userOption.substr(
          userOption.find_first_of('=') + 1);  // +1 for skipping '='
      auto tokens = strSplit(traceCallOption, ',');
      for (auto token : tokens) {
        Flags::setTraceCallId(token);
      }
    }

    if (remove_flags) {
      argv[i] = nullptr;
    }
  }

  Flags::set(userOptions);

  if (remove_flags) {
    Flags::shrinkArgumentList(argc, argv);
  }
}

// v8::Extension

class ExtensionSourceString : public v8::String::ExternalOneByteStringResource {
 public:
  explicit ExtensionSourceString(const char* data)
      : v8::String::ExternalOneByteStringResource(), data_(data) {}
  virtual ~ExtensionSourceString() override {}

  static void* operator new(size_t size) { return malloc(size); }
  static void operator delete(void* ptr) { free(ptr); }

  const char* data() const override { return data_; }
  size_t length() const override { return strlen(data_); }

 private:
  const char* data_;
};

std::vector<std::unique_ptr<Extension>> RegisteredExtension::extensions;

void RegisterExtension(std::unique_ptr<Extension> extension) {
  RegisteredExtension::registerExtension(std::move(extension));
}

void RegisteredExtension::registerExtension(
    std::unique_ptr<Extension> extension) {
  extensions.push_back(std::move(extension));
}

void RegisteredExtension::unregisterAll() {
  extensions.clear();
}

void RegisteredExtension::applyAll(ContextRef* context) {
  for (auto& extension : extensions) {
    if (isLwExtension(extension.get())) {
      auto lwExtension = reinterpret_cast<LwExtension*>(extension.get());
      if (lwExtension->isRegisteredExtension()) {
        lwExtension->apply(context);
      }
    } else {
      applyV8Extension(context, extension.get());
    }
  }
}

bool RegisteredExtension::isLwExtension(Extension* extension) {
  std::string name = extension->name();
  return (name == "v8/externalize") || (name == "v8/gc");
}

void RegisteredExtension::applyV8Extension(ContextRef* context,
                                           Extension* extension) {
  // Register v8 native function extension
  auto lwIsolate = IsolateWrap::GetCurrent();
  v8::Local<v8::String> str;
  v8::Local<v8::FunctionTemplate> v8FunctionTemplate =
      extension->GetNativeFunctionTemplate(lwIsolate->toV8(), str);

  auto esFunctionTemplate = CVAL(*v8FunctionTemplate)->ftpl();
  std::string src = extension->source()->data();
  std::string prefix = "native function ";
  size_t s = src.find(prefix.c_str());
  src = src.substr(s + prefix.length());
  src = src.substr(0, src.find("("));
  std::string name = src;

  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         std::string* name,
         FunctionTemplateRef* functionTemplate) -> ValueRef* {
        state->context()->globalObject()->defineDataProperty(
            state,
            StringRef::createFromASCII(name->c_str(), name->length()),
            functionTemplate->instantiate(state->context()),
            false,
            false,
            false);
        return ValueRef::createUndefined();
      },
      &name,
      esFunctionTemplate);
  LWNODE_CHECK(r.isSuccessful());
}

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
  if (source) {
    source_ = new ExtensionSourceString(source);
  }
}

// --expose-externalize-string

ValueRef* ExternalizeStringExtension::externalizeStringCallback(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    bool isConstructCall) {
  if (argc > 0 && argv[0]->isString()) {
    return argv[0]->asString();
  }

  return ValueRef::createUndefined();
}

ValueRef* ExternalizeStringExtension::isOneByteStringCallback(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    bool isConstructCall) {
  if (argc > 0 && argv[0]->isString()) {
    return ValueRef::create(
        StringRefHelper::isOneByteString(argv[0]->asString()));
  }
  return ValueRef::create(false);
}

// NOTE: --expose-externalize-string requires to define this dummy function
// i.e., function x() { return 1; }
ValueRef* ExternalizeStringExtension::xFunctionCallback(
    ExecutionStateRef* state,
    ValueRef* thisValue,
    size_t argc,
    ValueRef** argv,
    bool isConstructCall) {
  return ValueRef::create(1);
}

v8::Local<v8::FunctionTemplate>
ExternalizeStringExtension::GetNativeFunctionTemplate(
    v8::Isolate* isolate, v8::Local<v8::String> name) {
  EsScope scope(isolate);
  std::string functionName = scope.asValue(name)->toStdUTF8String();

  FunctionTemplateRef* functionTemplate = nullptr;
  if (functionName == "externalizeString") {
    functionTemplate = createExternalizeString(scope.lwIsolate());
  } else if (functionName == "isOneByteString") {
    functionTemplate = createIsOneByteString(scope.lwIsolate());
  } else {
    functionTemplate = createXFunction(scope.lwIsolate());
  }

  return Utils::NewLocal(isolate, functionTemplate);
}

bool ExternalizeStringExtension::isRegisteredExtension() {
  return Flags::isExposeExternalizeString();
}

void ExternalizeStringExtension::apply(ContextRef* context) {
  ObjectRefHelper::addNativeFunction(
      context,
      context->globalObject(),
      StringRef::createFromASCII("externalizeString"),
      ExternalizeStringExtension::externalizeStringCallback);
  ObjectRefHelper::addNativeFunction(
      context,
      context->globalObject(),
      StringRef::createFromASCII("isOneByteString"),
      ExternalizeStringExtension::isOneByteStringCallback);
  ObjectRefHelper::addNativeFunction(
      context,
      context->globalObject(),
      StringRef::createFromASCII("x"),
      ExternalizeStringExtension::xFunctionCallback);
}

FunctionTemplateRef* ExternalizeStringExtension::createExternalizeString(
    IsolateWrap* isolate) {
  auto functionTemplate = FunctionTemplateRef::create(
      AtomicStringRef::emptyAtomicString(),
      1,
      false,
      false,
      ExternalizeStringExtension::externalizeString);
  auto extraData = new FunctionTemplateData(functionTemplate,
                                            isolate->toV8(),
                                            v8::FunctionCallback(),
                                            nullptr,
                                            nullptr);
  ExtraDataHelper::setExtraData(functionTemplate, extraData);

  return functionTemplate;
}

FunctionTemplateRef* ExternalizeStringExtension::createIsOneByteString(
    IsolateWrap* isolate) {
  auto functionTemplate =
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                  1,
                                  false,
                                  false,
                                  ExternalizeStringExtension::isOneByteString);
  auto extraData = new FunctionTemplateData(functionTemplate,
                                            isolate->toV8(),
                                            v8::FunctionCallback(),
                                            nullptr,
                                            nullptr);
  ExtraDataHelper::setExtraData(functionTemplate, extraData);

  return functionTemplate;
}

FunctionTemplateRef* ExternalizeStringExtension::createXFunction(
    IsolateWrap* isolate) {
  auto functionTemplate =
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                  1,
                                  false,
                                  false,
                                  ExternalizeStringExtension::xFunction);
  auto extraData = new FunctionTemplateData(functionTemplate,
                                            isolate->toV8(),
                                            v8::FunctionCallback(),
                                            nullptr,
                                            nullptr);
  ExtraDataHelper::setExtraData(functionTemplate, extraData);

  return functionTemplate;
}

// --expose-gc

v8::Local<v8::FunctionTemplate>
ExternalizeGcExtension::GetNativeFunctionTemplate(v8::Isolate* isolate,
                                                  v8::Local<v8::String> name) {
  EsScope scope(isolate);
  std::string functionName = scope.asValue(name)->toStdUTF8String();

  FunctionTemplateRef* gcFunctionTemplate = nullptr;
  if (functionName == "gc") {
    gcFunctionTemplate = createGcFunctionTemplate(scope.lwIsolate());
  }

  return Utils::NewLocal(isolate, gcFunctionTemplate);
}

FunctionTemplateRef* ExternalizeGcExtension::createGcFunctionTemplate(
    EscargotShim::IsolateWrap* isolate) {
  auto gcFunctionTemplate =
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                  1,
                                  false,
                                  false,
                                  ExternalizeGcExtension::gc);
  auto extraData = new FunctionTemplateData(gcFunctionTemplate,
                                            isolate->toV8(),
                                            v8::FunctionCallback(),
                                            nullptr,
                                            nullptr);

  return gcFunctionTemplate;
}

ValueRef* ExternalizeGcExtension::gcCallback(ExecutionStateRef* state,
                                             ValueRef* thisValue,
                                             size_t argc,
                                             ValueRef** argv,
                                             bool isConstructCall) {
  if (argc > 0) {
    return ValueRef::createUndefined();
  }

  Escargot::Memory::gc();
  malloc_trim(0);
  return ValueRef::createUndefined();
}

bool ExternalizeGcExtension::isRegisteredExtension() {
  return Flags::isExposeGCEnabled();
}

void ExternalizeGcExtension::apply(ContextRef* context) {
  ObjectRefHelper::addNativeFunction(context,
                                     context->globalObject(),
                                     StringRef::createFromASCII("gc"),
                                     ExternalizeGcExtension::gcCallback);
}

// -------------------------

void ResourceConstraints::ConfigureDefaultsFromHeapSize(
    size_t initial_heap_size_in_bytes, size_t maximum_heap_size_in_bytes) {
  LWNODE_RETURN_VOID;
}

void ResourceConstraints::ConfigureDefaults(uint64_t physical_memory,
                                            uint64_t virtual_memory_limit) {
  int MB = 1024 * 1024;
  LWNODE_LOG_INFO("physical memory: %" PRIu64 "MB"
                  ", virtual memory limit: %" PRIu64 "MB",
                  physical_memory / MB,
                  virtual_memory_limit / MB);
}

size_t ResourceConstraints::max_semi_space_size_in_kb() const {
  LWNODE_RETURN_0;
}

void ResourceConstraints::set_max_semi_space_size_in_kb(size_t limit_in_kb) {
  LWNODE_RETURN_VOID;
}

i::Address* V8::GlobalizeReference(i::Isolate* isolate, i::Address* obj) {
  LWNODE_CALL_TRACE();
  LWNODE_CHECK(isolate);
#if defined(GC_HEAP_TRACE_ONLY)
  auto persistent = PersistentWrap::GlobalizeReference(
      reinterpret_cast<Isolate*>(isolate), obj);
  return reinterpret_cast<i::Address*>(persistent);
#endif

  IsolateWrap::fromV8(isolate)->globalHandles()->Create(VAL(obj));
  return obj;
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
  // Nothing to do for this
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
  LWNODE_CALL_TRACE();

#if defined(GC_HEAP_TRACE_ONLY)
  PersistentWrap::MakeWeak(location, parameter, weak_callback);
  return;
#endif

#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  if (type != WeakCallbackType::kParameter) {
    LWNODE_RETURN_VOID;  // TODO
  }
  GlobalHandles::MakeWeak(VAL(location), parameter, weak_callback);
#endif
}

void V8::MakeWeak(i::Address** location_addr) {
#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  GlobalHandles::MakeWeak(
      VAL(*location_addr), reinterpret_cast<void*>(location_addr), nullptr);
#endif
  LWNODE_RETURN_VOID;
}

void* V8::ClearWeak(i::Address* location) {
  LWNODE_CALL_TRACE();
#if defined(GC_HEAP_TRACE_ONLY)
  return PersistentWrap::ClearWeak(location);
#endif

#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  return GlobalHandles::ClearWeakness(VAL(location));
#else
  LWNODE_RETURN_NULLPTR;
#endif
}

void V8::AnnotateStrongRetainer(i::Address* location, const char* label) {
  LWNODE_RETURN_VOID;
}

void V8::DisposeGlobal(i::Address* location) {
  LWNODE_CALL_TRACE();
#if defined(GC_HEAP_TRACE_ONLY)
  PersistentWrap::DisposeGlobal(location);
  return;
#endif

  GlobalHandles::Destroy(VAL(location));
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
  API_ENTER_NO_EXCEPTION(v8_isolate);
  lwIsolate->addEternal(VAL(value));
  return value;
}

void V8::FromJustIsNothing() {
  LWNODE_CHECK(false);
}

void V8::ToLocalEmpty() {
  LWNODE_CHECK(false);
}

void V8::InternalFieldOutOfBounds(int index) {
  LWNODE_RETURN_VOID;
}

MaybeLocal<Value> JSON::Parse(Local<Context> context,
                              Local<String> json_string) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());
  auto lwContext = lwIsolate->GetCurrentContext();

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* state, ValueRef* jsonString) -> ValueRef* {
        auto fn = state->context()->globalObject()->jsonParse();
        auto parsed =
            fn->call(state, ValueRef::createUndefined(), 1, &jsonString);
        return parsed;
      },
      CVAL(*json_string)->value());
  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<Value>());

  return Utils::NewLocal<Object>(lwIsolate->toV8(), r.result);
}

MaybeLocal<String> JSON::Stringify(Local<Context> context,
                                   Local<Value> json_object,
                                   Local<String> gap) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<String>());
  auto lwContext = lwIsolate->GetCurrentContext();

  StringRef* esGap = gap.IsEmpty() ? StringRef::emptyString()
                                   : CVAL(*gap)->value()->asString();
  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* state,
         ValueRef* jsonObject,
         StringRef* gap) -> ValueRef* {
        auto fn = state->context()->globalObject()->jsonStringify();
        ValueRef* params[] = {jsonObject, ValueRef::createNull(), gap};
        auto str = fn->call(state, ValueRef::createUndefined(), 3, params);
        return str;
      },
      CVAL(*json_object)->value(),
      esGap);
  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<String>());

  return Utils::NewLocal<String>(lwIsolate->toV8(), r.result);
}

}  // namespace v8
