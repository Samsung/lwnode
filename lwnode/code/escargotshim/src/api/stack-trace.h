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

#include <EscargotPublic.h>
#include <GCUtil.h>

using namespace Escargot;

namespace EscargotShim {

class StackTrace {
 public:
  class NativeAccessorProperty
      : public ObjectRef::NativeDataAccessorPropertyData {
   public:
    NativeAccessorProperty(bool isWritable,
                           bool isEnumerable,
                           bool isConfigurable,
                           ObjectRef::NativeDataAccessorPropertyGetter getter,
                           ObjectRef::NativeDataAccessorPropertySetter setter,
                           ValueVectorRef* stackTraceVector)
        : NativeDataAccessorPropertyData(
              isWritable, isEnumerable, isConfigurable, getter, setter),
          stackTraceVector_(stackTraceVector) {}

    ValueVectorRef* stackTraceVector() { return stackTraceVector_; }

    void* operator new(size_t size) { return GC_MALLOC(size); }

   private:
    ValueVectorRef* stackTraceVector_ = nullptr;
  };

  StackTrace(ExecutionStateRef* state) : state_(state) {}

  static ValueRef* createCaptureStackTrace(ExecutionStateRef* state);
  static ValueRef* captureStackTraceCallback(ExecutionStateRef* state,
                                             ValueRef* thisValue,
                                             size_t argc,
                                             ValueRef** argv,
                                             bool isConstructCall);

  static ValueRef* createPrepareStackTrace(Escargot::ExecutionStateRef* state);
  static ValueRef* prepareStackTraceCallback(ExecutionStateRef* state,
                                             ValueRef* thisValue,
                                             size_t argc,
                                             ValueRef** argv,
                                             bool isConstructCall);

  static ValueRef* StackTraceGetter(
      ExecutionStateRef* state,
      ObjectRef* self,
      ValueRef* receiver,
      ObjectRef::NativeDataAccessorPropertyData* data);
  static bool StackTraceSetter(ExecutionStateRef* state,
                               ObjectRef* self,
                               ValueRef* receiver,
                               ObjectRef::NativeDataAccessorPropertyData* data,
                               ValueRef* setterInputData);

  static void addStackProperty(ExecutionStateRef* state,
                               ObjectRef* object,
                               ValueVectorRef* stackTraceVector);

  ArrayObjectRef* genCallSites();

  // FIXME: Having maxStackSize=1 does not print full stack. Find the right
  // value
  StringRef* formatStackTraceStringNodeStyle(ObjectRef* errorObject,
                                             size_t maxStackSize = 1);

 private:
  ExecutionStateRef* state_ = nullptr;

  static size_t getStackTraceLimit(ExecutionStateRef* state);

  std::string formatStackTraceLine(const Evaluator::StackTraceData& line);

  // NOTE: StackTrace can only be initialized as a local variable
  void* operator new(size_t size);
  void* operator new[](size_t size);
  void operator delete(void*, size_t);
  void operator delete[](void*, size_t);
};

class CallSite : public gc {
 public:
  CallSite(Escargot::ContextRef* context);

  ValueRef* instantiate(ContextRef* context,
                        const Evaluator::StackTraceData& data);

 private:
  ContextRef* context_ = nullptr;
  FunctionTemplateRef* template_ = nullptr;

  void injectSitePrototype();
  void setCallSitePrototype(
      const std::string& name,
      Escargot::FunctionObjectRef::NativeFunctionPointer fn);
};

}  // namespace EscargotShim
