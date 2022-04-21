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

#pragma once

#include <inttypes.h>  // For PRIu64
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
#include "api/error-message.h"
#include "api/es-helper.h"
#include "api/es-v8-helper.h"
#include "api/extra-data.h"
#include "api/function.h"
#include "api/handle-external-string.h"
#include "api/handle.h"
#include "api/isolate.h"
#include "api/module.h"
#include "api/object.h"
#include "api/serializer.h"
#include "api/utils/string-util.h"

namespace i = v8::internal;

namespace Escargot {
class ValueRef;
class ScriptRef;
class FunctionTemplateRef;
class ObjectTemplateRef;
}  // namespace Escargot

namespace v8 {
class Utils {
 public:
  // @lwnode
  static v8::Local<v8::Context> NewLocal(Isolate* isolate,
                                         EscargotShim::ContextWrap* ptr) {
    return v8::Local<v8::Context>::New(
        isolate,
        reinterpret_cast<v8::Context*>(
            EscargotShim::ValueWrap::createContext(ptr)));
  }

  template <typename T>
  static v8::Local<T> NewLocal(Isolate* isolate, EscargotShim::ValueWrap* ptr) {
    return v8::Local<T>::New(isolate, reinterpret_cast<T*>(ptr));
  }

  template <typename T>
  static v8::Local<T> NewLocal(Isolate* isolate, Escargot::ValueRef* ptr) {
    return v8::Local<T>::New(
        isolate,
        reinterpret_cast<T*>(EscargotShim::ValueWrap::createValue(ptr)));
  }

  template <typename T>
  static v8::Local<T> NewLocal(Isolate* isolate, Escargot::ScriptRef* ptr) {
    return v8::Local<T>::New(
        isolate,
        reinterpret_cast<T*>(EscargotShim::ValueWrap::createScript(ptr)));
  }

  static v8::Local<v8::ObjectTemplate> NewLocal(
      Isolate* isolate, Escargot::ObjectTemplateRef* ptr) {
    return v8::Local<v8::ObjectTemplate>::New(
        isolate,
        reinterpret_cast<v8::ObjectTemplate*>(
            EscargotShim::ValueWrap::createObjectTemplate(ptr)));
  }

  static v8::Local<v8::FunctionTemplate> NewLocal(
      Isolate* isolate, Escargot::FunctionTemplateRef* ptr) {
    return v8::Local<v8::FunctionTemplate>::New(
        isolate,
        reinterpret_cast<v8::FunctionTemplate*>(
            EscargotShim::ValueWrap::createFunctionTemplate(ptr)));
  }

  static v8::Local<v8::Signature> NewLocalSignature(
      Isolate* isolate, EscargotShim::ValueWrap* ptr) {
    return v8::Local<v8::Signature>::New(isolate,
                                         reinterpret_cast<v8::Signature*>(ptr));
  }

  template <class T>
  static Local<T> ToLocal(EscargotShim::ValueWrap* ptr) {
    return v8::Local<T>(reinterpret_cast<T*>(ptr));
  }

  template <class T>
  static Local<T> ToLocal(Escargot::ValueRef* ptr) {
    return v8::Local<T>(
        reinterpret_cast<T*>(EscargotShim::ValueWrap::createValue(ptr)));
  }

  // end @lwnode
};

// @lwnode
class v8::PropertyDescriptor::PrivateData {
 public:
  PrivateData() = default;
  PrivateData(Escargot::ObjectPropertyDescriptorRef& esDescriptor);
  PrivateData(Escargot::ValueRef* value);
  PrivateData(Escargot::ValueRef* value, bool writable);
  PrivateData(Escargot::ValueRef* get, Escargot::ValueRef* set);
  ~PrivateData();

  void setDescriptor(Escargot::ObjectPropertyDescriptorRef* descriptor);
  Escargot::ObjectPropertyDescriptorRef* descriptor() { return descriptor_; }

 private:
  bool isExternalDescriptor_{false};
  Escargot::ObjectPropertyDescriptorRef* descriptor_{nullptr};
};
// end @lwnode

// v8 Extension
class RegisteredExtension {
 public:
  static void registerExtension(std::unique_ptr<Extension> extension);
  static void unregisterAll();
  static void applyAll(ContextRef* context);

 private:
  explicit RegisteredExtension(Extension*) {}
  explicit RegisteredExtension(std::unique_ptr<Extension>) {}

  static bool isLwExtension(Extension* extension);
  static void applyV8Extension(ContextRef* context, Extension* extension);

  thread_local static std::vector<std::unique_ptr<Extension>> extensions;
};

class LwExtension : public v8::Extension {
 public:
  LwExtension(const char* name,
              const char* source = nullptr,
              int dep_count = 0,
              const char** deps = nullptr,
              int source_length = -1)
      : v8::Extension(name, source, dep_count, deps, source_length) {}

  v8::Local<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) override {
    return Local<FunctionTemplate>();
  };

  virtual bool isRegisteredExtension() = 0;
  virtual void apply(ContextRef* context) = 0;
};

class ExternalizeStringExtension : public LwExtension {
 public:
  ExternalizeStringExtension() : LwExtension("v8/externalize") {}
  v8::Local<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) override;

  bool isRegisteredExtension() override;
  void apply(ContextRef* context) override;

  FunctionTemplateRef* createExternalizeString(
      EscargotShim::IsolateWrap* isolate);
  FunctionTemplateRef* createIsOneByteString(
      EscargotShim::IsolateWrap* isolate);
  FunctionTemplateRef* createXFunction(EscargotShim::IsolateWrap* isolate);

  static ValueRef* externalizeStringCallback(ExecutionStateRef* state,
                                             ValueRef* thisValue,
                                             size_t argc,
                                             ValueRef** argv,
                                             bool isConstructCall);
  static ValueRef* externalizeString(ExecutionStateRef* state,
                                     ValueRef* thisValue,
                                     size_t argc,
                                     ValueRef** argv,
                                     OptionalRef<ObjectRef> newTarget) {
    return externalizeStringCallback(state, thisValue, argc, argv, false);
  }

  static ValueRef* isOneByteStringCallback(ExecutionStateRef* state,
                                           ValueRef* thisValue,
                                           size_t argc,
                                           ValueRef** argv,
                                           bool isConstructCall);
  static ValueRef* isOneByteString(ExecutionStateRef* state,
                                   ValueRef* thisValue,
                                   size_t argc,
                                   ValueRef** argv,
                                   OptionalRef<ObjectRef> newTarget) {
    return isOneByteStringCallback(state, thisValue, argc, argv, false);
  }

  static ValueRef* xFunctionCallback(ExecutionStateRef* state,
                                     ValueRef* thisValue,
                                     size_t argc,
                                     ValueRef** argv,
                                     bool isConstructCall);
  static ValueRef* xFunction(ExecutionStateRef* state,
                             ValueRef* thisValue,
                             size_t argc,
                             ValueRef** argv,
                             OptionalRef<ObjectRef> newTarget) {
    return xFunctionCallback(state, thisValue, argc, argv, false);
  }

 private:
};

class ExternalizeGcExtension : public LwExtension {
 public:
  ExternalizeGcExtension() : LwExtension("v8/gc") {}
  v8::Local<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) override;

  bool isRegisteredExtension() override;
  void apply(ContextRef* context) override;

  FunctionTemplateRef* createGcFunctionTemplate(
      EscargotShim::IsolateWrap* isolate);

  static ValueRef* gcCallback(ExecutionStateRef* state,
                              ValueRef* thisValue,
                              size_t argc,
                              ValueRef** argv,
                              bool isConstructCall);

  static ValueRef* gc(ExecutionStateRef* state,
                      ValueRef* thisValue,
                      size_t argc,
                      ValueRef** argv,
                      OptionalRef<ObjectRef> newTarget) {
    return gcCallback(state, thisValue, argc, argv, false);
  }
};

}  // namespace v8

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

}  // namespace
