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
#include "utils/gc-util.h"
#include "utils/misc.h"
#include "v8.h"

namespace EscargotShim {
class InternalFieldData;
class FunctionTemplateData;
class ObjectTemplateData;
class FunctionData;
class ObjectData;
class ExternalObjectData;
class GlobalObjectData;
class ExceptionObjectData;
class StackTraceData;
class ValueWrap;

using namespace Escargot;

class ExtraData : public gc {
 public:
  virtual bool isInternalFieldData() const { return false; }
  virtual bool isFunctionTemplateData() const { return false; }
  virtual bool isObjectTemplateData() const { return false; }
  virtual bool isObjectData() const { return false; }
  virtual bool isFunctionData() const { return false; }
  virtual bool isExternalObjectData() const { return false; }
  virtual bool isExceptionObjectData() const { return false; }
  virtual bool isStackTraceData() const { return false; }
  virtual bool isGlobalObjectData() const { return false; }

  InternalFieldData* asInternalFieldData() {
    LWNODE_CHECK(isInternalFieldData());
    return reinterpret_cast<InternalFieldData*>(this);
  }

  FunctionTemplateData* asFunctionTemplateData() {
    LWNODE_CHECK(isFunctionTemplateData());
    return reinterpret_cast<FunctionTemplateData*>(this);
  }

  ObjectTemplateData* asObjectTemplateData() {
    LWNODE_CHECK(isObjectTemplateData());
    return reinterpret_cast<ObjectTemplateData*>(this);
  }

  FunctionData* asFunctionData() {
    LWNODE_CHECK(isFunctionData());
    return reinterpret_cast<FunctionData*>(this);
  }

  ObjectData* asObjectData() {
    LWNODE_CHECK(isObjectData());
    return reinterpret_cast<ObjectData*>(this);
  }

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
};

class InternalFieldData : public ExtraData {
 public:
  InternalFieldData() = default;
  InternalFieldData(int count) { setInternalFieldCount(count); }
  bool isInternalFieldData() const override { return true; }

  virtual int internalFieldCount();
  virtual void setInternalFieldCount(int size);
  virtual void* internalField(int idx);
  virtual void setInternalField(int idx, void* lwValue);

 private:
  bool isValidIndex(int idx);
  GCContainer<void*>* internalFields_{nullptr};
};

class TemplateData : public InternalFieldData {
 public:
  virtual FunctionTemplateRef* functionTemplate() { return functionTemplate_; }

 protected:
  TemplateData() = default;
  TemplateData(FunctionTemplateRef* functionTemplate)
      : InternalFieldData(), functionTemplate_(functionTemplate) {}
  FunctionTemplateRef* functionTemplate_{nullptr};
};

class FunctionTemplateData : public TemplateData {
 public:
  FunctionTemplateData() = default;
  FunctionTemplateData(FunctionTemplateRef* functionTemplate)
      : TemplateData(functionTemplate) {}
  FunctionTemplateData(FunctionTemplateRef* functionTemplate,
                       v8::Isolate* isolate,
                       v8::FunctionCallback callback,
                       v8::Value* callbackData,
                       v8::Signature* signature)
      : TemplateData(functionTemplate),
        isolate_(isolate),
        callback_(callback),
        callbackData_(callbackData),
        signature_(signature) {}

  bool isFunctionTemplateData() const override { return true; }

  v8::Isolate* isolate() { return isolate_; }
  v8::FunctionCallback callback() { return callback_; }
  v8::Value* callbackData() { return callbackData_; }
  v8::Signature* signature() { return signature_; }

  void setCallback(v8::FunctionCallback callback) { callback_ = callback; }
  void setCallbackData(v8::Value* callbackData) {
    callbackData_ = callbackData;
  }

 private:
  v8::Isolate* isolate_{nullptr};
  v8::FunctionCallback callback_{nullptr};
  v8::Value* callbackData_{nullptr};
  v8::Signature* signature_{nullptr};
};

class ObjectTemplateData : public TemplateData {
 public:
  ObjectTemplateData() = default;
  ObjectTemplateData(FunctionTemplateRef* functionTemplate)
      : TemplateData(functionTemplate) {}

  bool isObjectTemplateData() const override { return true; }

  ObjectData* createObjectData(ObjectTemplateRef* objectTemplate);

 private:
};

class ObjectData : public TemplateData {
 public:
  ObjectData() = default;
  ObjectData(FunctionTemplateRef* functionTemplate)
      : TemplateData(functionTemplate) {}
  ObjectData(FunctionObjectRef* functionObject);
  ObjectData(ObjectTemplateRef* objectTemplate);

  bool isObjectData() const override { return true; }

  void setFunctionObject(FunctionObjectRef* functionObject) {
    LWNODE_CHECK(functionObject_ == nullptr);
    functionObject_ = functionObject;
  }

  ObjectTemplateRef* objectTemplate() { return objectTemplate_; }
  FunctionObjectRef* functionObject() { return functionObject_; }

 protected:
  ObjectTemplateRef* objectTemplate_{nullptr};
  FunctionObjectRef* functionObject_{nullptr};
};

class FunctionData : public ObjectData {
 public:
  FunctionData(FunctionTemplateRef* functionTemplate)
      : ObjectData(functionTemplate) {}

  bool isFunctionData() const override { return true; }

  v8::Isolate* isolate();
  v8::FunctionCallback callback();
  v8::Value* callbackData();
  v8::Signature* signature();

  bool checkSignature(Escargot::ExecutionStateRef* state, ObjectRef* thisValue);
  std::string toString();

 private:
  FunctionData() = default;
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
// NOTE: StackTraceData does not use any InternalFields.
// If InternalField is needed, make it inherit from "public InternalFieldData"
class StackTraceData : public ExtraData {
 public:
  StackTraceData(const Escargot::Evaluator::StackTraceData& data)
      : data_(data) {}

  bool isStackTraceData() const override { return true; }

  Evaluator::StackTraceData stackTraceData() { return data_; }
  StringRef* src() const { return data_.srcName; }
  StringRef* sourceCode() const { return data_.sourceCode; }
  Escargot::Evaluator::LOC loc() const { return data_.loc; }
  StringRef* functionName() const { return data_.functionName; }
  bool isFunction() const { return data_.isFunction; }
  bool isConstructor() const { return data_.isConstructor; }
  bool isAssociatedWithJavaScriptCode() const {
    return data_.isAssociatedWithJavaScriptCode;
  }
  bool isEval() const { return data_.isEval; }
  OptionalRef<FunctionObjectRef> callee() { return data_.callee; }

 private:
  Escargot::Evaluator::StackTraceData data_;
};

// NOTE: ExceptionObjectData does not use any InternalFields.
// If InternalField is needed, make it inherit from "public InternalFieldData"
class ExceptionObjectData : public ExtraData {
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
