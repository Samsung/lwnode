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

#include "api/context.h"
#include "api/escargot-util.h"
#include "api/handle.h"
#include "api/isolate.h"
#include "api/utils/string.h"
#include "escargotshim-base.h"

using namespace Escargot;
using namespace EscargotShim;

#define VAL(that) reinterpret_cast<const ValueWrap*>(that)

#define PRIVATE_UTIL_1(_isolate, bailout_value)                                \
  if (_isolate->IsExecutionTerminating()) {                                    \
    return bailout_value;                                                      \
  }

#define API_ENTER(isolate, bailout_value)                                      \
  auto _isolate = IsolateWrap::fromV8(isolate);                                \
  PRIVATE_UTIL_1(_isolate, bailout_value)

#define API_ENTER_WITH_CONTEXT(context, bailout_value)                         \
  auto _isolate = context.IsEmpty()                                            \
                      ? IsolateWrap::currentIsolate()                          \
                      : ValueWrap(reinterpret_cast<ValueWrap*>(*context))      \
                            .context()                                         \
                            ->GetIsolate();                                    \
  PRIVATE_UTIL_1(_isolate, bailout_value)

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
  LWNODE_CHECK(false);
}

void V8::ToLocalEmpty() {
  LWNODE_CHECK(false);
}

void V8::InternalFieldOutOfBounds(int index) {
  LWNODE_RETURN_VOID;
}

// --- H a n d l e s ---
// --- T e m p l a t e ---
// --- S c r i p t s ---
// --- E x c e p t i o n s ---
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
// --- D a t a ---
// --- E n v i r o n m e n t ---
// --- D e b u g   S u p p o r t ---

}  // namespace v8

#include "api-handles.hpp"
#include "api-template.hpp"
#include "api-scripts.hpp"
#include "api-exception.hpp"
#include "api-serialization.hpp"
#include "api-data.hpp"
#include "api-environment.hpp"
#include "api-debug.hpp"
