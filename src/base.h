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

#pragma once

#include "api.h"
#include "unimplemented.h"

#include <EscargotPublic.h>
#include <v8.h>

namespace EscargotShim {
class ValueWrap;
}

#define VAL(that) reinterpret_cast<EscargotShim::ValueWrap*>(that)
#define CVAL(that) reinterpret_cast<const EscargotShim::ValueWrap*>(that)

#define __TERMINATION_CHECK(lwIsolate, bailout_value)                          \
  if (lwIsolate->IsExecutionTerminating()) {                                   \
    LWNODE_DLOG_WARN(                                                          \
        "%s is ignored due to script execution being terminated.",             \
        __CODE_LOCATION__);                                                    \
    return bailout_value;                                                      \
  }

#if !defined(NDEBUG)
#define __DLOG_EVAL_EXCEPTION(eval_result)                                     \
  LWNODE_DLOG_RAW("Execute:\n  %s\n%s",                                        \
                  __CODE_LOCATION__,                                           \
                  EvalResultHelper::getErrorString(                            \
                      lwIsolate->GetCurrentContext()->get(), eval_result)      \
                      .c_str());
#else
#define __DLOG_EVAL_EXCEPTION(eval_result)
#endif

#define __CALL_TRACE_ID(id, ...) LWNODE_CALL_TRACE_ID(id)

#define API_ENTER(isolate, bailout_value, ...)                                 \
  __CALL_TRACE_ID(__VA_ARGS__ __VA_OPT__(, ) COMMON);                          \
  IsolateWrap* lwIsolate = IsolateWrap::fromV8(isolate);                       \
  __TERMINATION_CHECK(lwIsolate, bailout_value)

#define API_ENTER_NO_EXCEPTION(isolate, ...)                                   \
  __CALL_TRACE_ID(__VA_ARGS__ __VA_OPT__(, ) COMMON);                          \
  IsolateWrap* lwIsolate = IsolateWrap::fromV8(isolate);

#define API_ENTER_WITH_CONTEXT(local_context, bailout_value, ...)              \
  __CALL_TRACE_ID(__VA_ARGS__ __VA_OPT__(, ) COMMON);                          \
  IsolateWrap* lwIsolate = local_context.IsEmpty()                             \
                               ? IsolateWrap::GetCurrent()                     \
                               : VAL(*local_context)->context()->GetIsolate(); \
  __TERMINATION_CHECK(lwIsolate, bailout_value)

#define API_HANDLE_EXCEPTION(eval_result, lwIsolate, bailout_value)            \
  if (!eval_result.isSuccessful()) {                                           \
    __DLOG_EVAL_EXCEPTION(eval_result);                                        \
    lwIsolate->handleException(std::move(eval_result));                        \
    return bailout_value;                                                      \
  }

#define API_ENTER_AND_EXIT_IF_TERMINATING(ScopeType, context, returnValue)     \
  ScopeType scope(context, this);                                              \
  if (scope.isTerminating()) {                                                 \
    return returnValue;                                                        \
  }

#define API_ENTER_NO_TERMINATION_CHECK(ScopeType, isolate)                     \
  ScopeType scope(isolate, this);

#define API_EXIT_IF_EXCEPTION_OCCURRED(r, returnValue)                         \
  if (!scope.isSuccessful(r)) {                                                \
    scope.printDebug(r);                                                       \
    return returnValue;                                                        \
  }

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

namespace EscargotShim {

class EsScope {
 public:
  EsScope(const v8::Value* self = nullptr) : EsScope(nullptr, self) {}
  EsScope(v8::Isolate* isolate, const v8::Value* self = nullptr);
  EsScope(const v8::Local<v8::Context>& context,
          const v8::Value* self = nullptr);

  Escargot::ValueRef* self() { return self_; }
  virtual EscargotShim::IsolateWrap* lwIsolate() { return isolate_; }
  virtual v8::Isolate* v8Isolate() { return isolate_->toV8(); }
  virtual Escargot::ContextRef* context() {
    if (context_) {
      return context_->get();
    }
    return isolate_->GetCurrentContext()->get();
  }

  virtual ValueRef* asValue(const v8::Local<v8::Value>& value) {
    return CVAL(*value)->value();
  }

  virtual ValueRef* asValue(const v8::Local<v8::Name>& value) {
    return CVAL(*value)->value();
  }

  virtual StringRef* asValue(const v8::Local<v8::String>& value) {
    return CVAL(*value)->value()->asString();
  }

  void printDebug(Escargot::Evaluator::EvaluatorResult& r) {
#if !defined(NDEBUG)
    LWNODE_DLOG_RAW("Execute:\n  %s\n%s",
                    __CODE_LOCATION__,
                    EvalResultHelper::getErrorString(context(), r).c_str());
#endif
  }

  bool isTerminating() { return isolate_->IsExecutionTerminating(); }

  bool isSuccessful(Escargot::Evaluator::EvaluatorResult& r) {
    if (!r.isSuccessful()) {
      isolate_->SetPendingExceptionAndMessage(r.error.get(), r.stackTrace);
      isolate_->ReportPendingMessages();
      return false;
    }
    return true;
  }

 protected:
  EscargotShim::IsolateWrap* isolate_ = nullptr;
  EscargotShim::ContextWrap* context_ = nullptr;

 private:
  Escargot::ValueRef* self_ = nullptr;

  void initSelf(const v8::Value* self) {
    if (self) {
      self_ = CVAL(self)->value();
    }
  }
};

class EsScopeTemplate : public EsScope {
 public:
  EsScopeTemplate(const v8::Template* self = nullptr);
  EsScopeTemplate(const v8::Local<v8::Context>& context,
                  const v8::Template* self = nullptr);
  EscargotShim::TemplateRef* self() { return self_; }

  virtual FunctionTemplateRef* asFunctionTemplate(
      const v8::Local<v8::FunctionTemplate>& value) {
    return CVAL(*value)->ftpl();
  }

 protected:
  EscargotShim::TemplateRef* self_ = nullptr;

 private:
  void initSelf(const v8::Template* self) {
    if (self) {
      self_ = CVAL(self)->tpl();
    }
  }
};

class EsScopeFunctionTemplate : public EsScopeTemplate {
 public:
  EsScopeFunctionTemplate(const v8::FunctionTemplate* self)
      : EsScopeTemplate(self) {}
  EsScopeFunctionTemplate(const v8::Local<v8::Context>& context,
                          const v8::FunctionTemplate* self)
      : EsScopeTemplate(context, self) {}

  EscargotShim::FunctionTemplateRef* self() {
    LWNODE_CHECK(self_->isFunctionTemplate());
    return reinterpret_cast<EscargotShim::FunctionTemplateRef*>(self_);
  }

 private:
};

class EsScopeObjectTemplate : public EsScopeTemplate {
 public:
  EsScopeObjectTemplate(const v8::ObjectTemplate* self)
      : EsScopeTemplate(self) {}
  EsScopeObjectTemplate(const v8::Local<v8::Context>& context,
                        const v8::ObjectTemplate* self)
      : EsScopeTemplate(context, self) {}

  EscargotShim::ObjectTemplateRef* self() {
    LWNODE_CHECK(self_->isObjectTemplate());
    return reinterpret_cast<EscargotShim::ObjectTemplateRef*>(self_);
  }

 private:
};

}  // namespace EscargotShim
