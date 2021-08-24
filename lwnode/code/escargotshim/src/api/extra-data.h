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
#include <memory>
#include "utils/gc.h"
#include "v8.h"

namespace EscargotShim {
class ExternalObjectData;
class GlobalObjectData;
class ExceptionObjectData;
class StackTraceData;
class ValueWrap;
class BackingStoreWrap;

using namespace Escargot;

class ObjectData : public gc {
 public:
  ExternalObjectData* asExternalObjectData() {
    LWNODE_CHECK(isExternalObjectData());
    return reinterpret_cast<ExternalObjectData*>(this);
  }

  GlobalObjectData* asGlobalObjectData() {
    LWNODE_CHECK(isGlobalObjectData());
    return reinterpret_cast<GlobalObjectData*>(this);
  }

  ExceptionObjectData* asExceptionObjectData() {
    LWNODE_CHECK(isExceptionObjectData());
    return reinterpret_cast<ExceptionObjectData*>(this);
  }

  StackTraceData* asStackTraceData() {
    LWNODE_CHECK(isStackTraceData());
    return reinterpret_cast<StackTraceData*>(this);
  }

  virtual bool isFunctionData() const { return false; }
  virtual bool isExternalObjectData() const { return false; }
  virtual bool isExceptionObjectData() const { return false; }
  virtual bool isStackTraceData() const { return false; }
  virtual bool isGlobalObjectData() const { return false; }

  ObjectData* clone();

  // InternalFields
  int internalFieldCount();
  void setInternalFieldCount(int size);
  void setInternalField(int idx, void* lwValue);
  void* internalField(int idx);

  // Template
  void setInstanceTemplate(Escargot::FunctionTemplateRef* tpl);
  Escargot::FunctionTemplateRef* instanceTemplate();

 private:
  GCContainer<void*>* m_internalFields{nullptr};
  Escargot::FunctionTemplateRef* instanceTemplate_{nullptr};
};

class FunctionData : public ObjectData {
 public:
  FunctionData() = default;
  FunctionData(v8::Isolate* isolate,
               v8::FunctionCallback callback,
               v8::Value* callbackData,
               v8::Signature* signature)
      : m_isolate(isolate),
        m_callback(callback),
        m_callbackData(callbackData),
        m_signature(signature) {}

  bool isFunctionData() const override { return true; }

  static FunctionData* toFunctionData(void* ptr) {
    LWNODE_CHECK_NOT_NULL(ptr);
    auto data = reinterpret_cast<FunctionData*>(ptr);
    LWNODE_CHECK(data->isFunctionData());
    return data;
  }

  v8::Isolate* isolate() { return m_isolate; }
  v8::FunctionCallback callback() { return m_callback; }
  void setCallback(v8::FunctionCallback callback) { m_callback = callback; }
  v8::Value* callbackData() { return m_callbackData; }
  void setCallbackData(v8::Value* data) { m_callbackData = data; }

  bool checkSignature(Escargot::ExecutionStateRef* state, ValueRef* thisValue);

 private:
  v8::Isolate* m_isolate{nullptr};
  v8::FunctionCallback m_callback{nullptr};
  v8::Value* m_callbackData{nullptr};
  v8::Signature* m_signature{nullptr};
};

class ExternalObjectData : public ObjectData {
 public:
  enum InternalFields {
    kValueSlot,
    kInternalFieldCount,
  };
  bool isExternalObjectData() const override { return true; }
};

// TODO: check to make sure StackTraceData inherit ObjectData.
class StackTraceData : public ObjectData {
 public:
  StackTraceData(const Escargot::Evaluator::StackTraceData& data)
      : src_(data.src),
        sourceCode_(data.sourceCode),
        loc_(data.loc),
        functionName_(data.functionName),
        isConstructor_(data.isConstructor),
        isAssociatedWithJavaScriptCode_(data.isAssociatedWithJavaScriptCode),
        isEval_(data.isEval) {}

  bool isStackTraceData() const override { return true; }

  StringRef* src() const { return src_; }
  StringRef* sourceCode() const { return sourceCode_; }
  Escargot::Evaluator::LOC loc() const { return loc_; }
  StringRef* functionName() const { return functionName_; }
  bool isFunction() const { return isFunction_; }
  bool isConstructor() const { return isConstructor_; }
  bool isAssociatedWithJavaScriptCode() const {
    return isAssociatedWithJavaScriptCode_;
  }
  bool isEval() const { return isEval_; }

 private:
  StringRef* src_{nullptr};
  StringRef* sourceCode_{nullptr};
  Escargot::Evaluator::LOC loc_{0, 0, 0};
  StringRef* functionName_{nullptr};
  bool isFunction_{false};
  bool isConstructor_{false};
  bool isAssociatedWithJavaScriptCode_{false};
  bool isEval_{false};
};

class ExceptionObjectData : public ObjectData {
 public:
 public:
  ExceptionObjectData(
      GCManagedVector<Escargot::Evaluator::StackTraceData>& stackTraceData,
      bool isThisExceptionUndefined = false);

  ExceptionObjectData(GCVector<StackTraceData*>* stackTrace,
                      bool isThisExceptionUndefined = false)
      : isThisExceptionUndefined_(isThisExceptionUndefined) {
    {
      for (const auto& iter : *stackTrace) {
        stackTraces_.push_back(iter);
      }
    }
  }

  bool isExceptionObjectData() const override { return true; }

  GCVector<StackTraceData*>* stackTrace() { return &stackTraces_; }
  static GCVector<StackTraceData*>* stackTrace(ObjectRef* exceptionObject);

 private:
  GCVector<StackTraceData*> stackTraces_;
  bool isThisExceptionUndefined_{false};
};

class GlobalObjectData : public ObjectData {
 public:
  enum InternalFields {
    kContextWrapSlot,
    kInternalFieldCount,
  };
  bool isGlobalObjectData() const override { return true; }
};

}  // namespace EscargotShim
