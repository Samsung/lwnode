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

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  flag_t flags = FlagType::Empty;

  for (int i = 1; i < *argc; i++) {
    char* arg = argv[i];
    bool checked = false;

    if (strEquals("--expose-gc", arg) || strEquals("--expose_gc", arg)) {
      flags |= FlagType::ExposeGC;
      checked = true;
    } else if (strEquals("--use-strict", arg) ||
               strEquals("--use_strict", arg)) {
      flags |= FlagType::UseStrict;
      checked = true;
    } else if (strEquals("--off-idlegc", arg) ||
               strEquals("--off_idlegc", arg)) {
      flags |= FlagType::DisableIdleGC;
      checked = true;
    } else if (strEquals("--harmony-top-level-await", arg)) {
      // @check https://github.sec.samsung.net/lws/node-escargot/issues/394
      flags |= FlagType::TopLevelWait;
      checked = true;
    } else if (strEquals("--trace-gc", arg)) {
      flags |= FlagType::TraceGC;
      checked = true;
    } else if (strStartsWith(arg, "--trace-call")) {
      flags |= FlagType::TraceCall;
      checked = true;

      std::string str(arg);
      std::string::size_type pos = str.find_first_of('=');
      if (std::string::npos != pos) {
        std::stringstream ss(str.substr(pos + 1));  // +1 for skipping =
        std::string token;
        while (std::getline(ss, token, ',')) {
          if (token.find('-') == 0) {
            Flags::setNagativeTraceCallId(token.substr(1));
          } else {
            Flags::setTraceCallId(token);
          }
        }
      }
    } else if (strEquals("--internal-log", arg)) {
      flags |= FlagType::InternalLog;
      checked = true;
    } else if (remove_flags && (strStartsWith(arg, "--debug") ||
                                strStartsWith(arg, "--stack-size=") ||
                                strStartsWith(arg, "--nolazy") ||
                                strStartsWith(arg, "--trace_debug"))) {
      checked = true;
    } else {
      LWNODE_DLOG_WARN("'%s' flag is ignored", arg);
    }

    if (checked && remove_flags) {
      argv[i] = nullptr;
    }
  }

  Flags::set(flags);

  if (remove_flags) {
    int count = 0;
    for (int idx = 0; idx < *argc; idx++) {
      if (argv[idx]) {
        argv[count++] = argv[idx];
      }
    }
    *argc = count;
  }
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
  IsolateWrap::fromV8(isolate)->globalHandles()->Create(VAL(obj));
  Engine::current()->gcHeap()->GlobalizeReference(obj, isolate);
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

  Engine::current()->gcHeap()->MakeWeak(
      location, parameter, weak_callback, type);

#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  if (type != WeakCallbackType::kParameter) {
    LWNODE_RETURN_VOID;  // TODO
  }

  GlobalHandles::MakeWeak(VAL(location), parameter, weak_callback);
#else
  LWNODE_RETURN_VOID;
#endif
}

void V8::MakeWeak(i::Address** location_addr) {
  LWNODE_RETURN_VOID;
}

void* V8::ClearWeak(i::Address* location) {
  LWNODE_CALL_TRACE();
  Engine::current()->gcHeap()->ClearWeak(location);
  LWNODE_RETURN_NULLPTR;
}

void V8::AnnotateStrongRetainer(i::Address* location, const char* label) {
  LWNODE_RETURN_VOID;
}

void V8::DisposeGlobal(i::Address* location) {
  LWNODE_CALL_TRACE();
  GlobalHandles::Destroy(VAL(location));
  Engine::current()->gcHeap()->DisposeGlobal(location);
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
  auto lwContext = CVAL(*context)->context();

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
  auto lwContext = CVAL(*context)->context();

  if (!gap.IsEmpty()) {
    LWNODE_UNIMPLEMENT;
  }

  auto r = Evaluator::execute(
      lwContext->get(),
      [](ExecutionStateRef* state,
         ValueRef* jsonObject,
         StringRef* gap) -> ValueRef* {
        auto fn = state->context()->globalObject()->jsonStringify();
        auto str = fn->call(state, ValueRef::createUndefined(), 1, &jsonObject);
        return str;
      },
      CVAL(*json_object)->value(),
      StringRef::emptyString());
  API_HANDLE_EXCEPTION(r, lwIsolate, MaybeLocal<String>());

  return Utils::NewLocal<String>(lwIsolate->toV8(), r.result);
}

}  // namespace v8
