/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <functional>
#include "es.h"

template <typename T>
class Maybe {
 public:
  Maybe() : has_value_(false) {}
  Maybe(const T& value) : value_(value), has_value_(true) {}

  bool IsJust() const { return has_value_; }
  bool IsNothing() const { return !has_value_; }

  T FromJust() const {
    ES_ASSERT(has_value_);
    return value_;
  }

  T FromMaybe(const T& default_value) const {
    return has_value_ ? value_ : default_value;
  }

  bool To(T* out) const {
    if (ES_LIKELY(IsJust())) {
      *out = value_;
    }
    return IsJust();
  }

  void SetValue(const T& value) {
    value_ = value;
    has_value_ = true;
  }

  void Reset() { has_value_ = false; }

 private:
  T value_;
  bool has_value_;
};

template <class T>
inline Maybe<T> Nothing() {
  return Maybe<T>(T());
}

template <class T>
inline Maybe<T> Just(const T& value) {
  return value;
}

namespace Escargot {

ValueRef* BuiltinPrint(ExecutionStateRef* state,
                       ValueRef* this_value,
                       size_t argc,
                       ValueRef** argv,
                       bool is_construct_call);

OptionalRef<StringRef> BuiltinHelperFileRead(
    OptionalRef<ExecutionStateRef> state,
    const char* file_name,
    const char* builtin_name,
    bool should_throw_on_error = false);

ValueRef* BuiltinLoad(ExecutionStateRef* state,
                      ValueRef* this_value,
                      size_t argc,
                      ValueRef** argv,
                      bool is_construct_call);

ValueRef* BuiltinRead(ExecutionStateRef* state,
                      ValueRef* this_value,
                      size_t argc,
                      ValueRef** argv,
                      bool is_construct_call);

ValueRef* BuiltinRun(ExecutionStateRef* state,
                     ValueRef* this_value,
                     size_t argc,
                     ValueRef** argv,
                     bool is_construct_call);

ValueRef* BuiltinGc(ExecutionStateRef* state,
                    ValueRef* this_value,
                    size_t argc,
                    ValueRef** argv,
                    bool is_construct_call);

bool EvalScript(ContextRef* context,
                StringRef* source,
                StringRef* src_name,
                bool should_print_script_result,
                bool is_module);

void AttachBuiltinPrint(ContextRef* context, ObjectRef* target);

ExecResult CompileAndExecution(ContextRef* context,
                               StringRef* source,
                               StringRef* source_name,
                               bool isModule);

class ExecResultHelper {
 public:
  // If a function is provided, return `true` to exclue the call stack, `false`
  // to include it.
  typedef std::function<bool(std::string& codePath,
                             std::string& errorCodeLine,
                             const int errorLine,
                             const int errorColumn)>
      CallStackFilter;

  static void SetCallStackFilter(CallStackFilter filter);
  static void ShowInternalCode(bool visible);

  static void SetInternalSourcePatternGetter(
      std::function<std::string()> getter) {
    internalSourcePatternGetter = getter;
  }

  static std::string GetInternalSourcePattern();

  static std::string GetErrorString(
      ContextRef* context, const Evaluator::EvaluatorResult& eval_result);

  static std::string GetStackTraceString(
      const GCManagedVector<Evaluator::StackTraceData>& traceData,
      const std::string& reasonString,
      size_t max_stack_size = kDefaultMaxStackSize);

  static std::string GetCallStackString(
      const GCManagedVector<Evaluator::StackTraceData>& trace_data,
      size_t max_stack_size);

 private:
  static bool showInternalCode;
  static std::function<std::string()> internalSourcePatternGetter;
  static CallStackFilter callStackFilter;
  static const size_t kDefaultMaxStackSize = 30;
};

class ExceptionHelper {
 public:
  static ErrorObjectRef* CreateErrorObject(ContextRef* context,
                                           ErrorObjectRef::Code code,
                                           StringRef* errorMessage);
  static void AddDetailsToErrorObject(ExecutionStateRef* state,
                                      Escargot::ErrorObjectRef* maybe_error);
  static std::string GetErrorReason(ExecutionStateRef* state,
                                    Escargot::ValueRef* maybe_error);

  using ExceptionSetter = std::function<void(
      ContextRef* context, ValueRef* error, std::string& reason)>;
  using ExceptionReporter = std::function<void(ContextRef* context)>;

  static void SetExceptionHandlers(ExceptionSetter setter,
                                   ExceptionReporter reporter) {
    pending_exception_setter_ = setter;
    exception_reporter_ = reporter;
  }

  static void SetPendingException(ContextRef* context,
                                  ValueRef* error,
                                  std::string reason) {
    if (!pending_exception_setter_) return;
    pending_exception_setter_(context, error, reason);
  }

  static void ReportPendingException(ContextRef* context) {
    if (!exception_reporter_) return;
    exception_reporter_(context);
  }

 private:
  static ExceptionSetter pending_exception_setter_;
  static ExceptionReporter exception_reporter_;
};

class TryCatchScope {
 public:
  TryCatchScope(Escargot::ContextRef* context, bool report_exception = true)
      : context_(context),
        exception_(nullptr),
        prev_scope_(current_scope),
        report_exception_(report_exception) {
    current_scope = this;
  }

  ~TryCatchScope() {
    if (context_ && report_exception_ && exception_ != nullptr) {
      // context_ is only needed for ReportPendingException.
      ReportPendingException(context_);
      // Once exception is reported, then the exception is cleared.
      exception_ = nullptr;
      reason_.clear();
    }

    if (exception_ != nullptr && prev_scope_ != nullptr) {
      // This should be after handling report_exception_ above. Once the
      // exception is reported, then it's not passed to previous scopes.
      prev_scope_->Catch(exception_, reason_);
    }

    // Change the current scope
    current_scope = prev_scope_;
  }

  TryCatchScope(const TryCatchScope&) = delete;
  TryCatchScope(TryCatchScope&&) = delete;
  TryCatchScope& operator=(const TryCatchScope&) = delete;
  TryCatchScope& operator=(TryCatchScope&&) = delete;

  static bool HasScope() { return current_scope != nullptr; }

  static void Catch(Escargot::ValueRef* e, std::string reason) {
    if (current_scope) {
      current_scope->exception_ = e;
      current_scope->reason_ = reason;
    }
  }

  bool HasCaught() const { return exception_ != nullptr; }
  void ThrowIfHasException(Escargot::ExecutionStateRef* state) {
    if (HasCaught()) {
      state->throwException(exception_);
    }
  }
  Escargot::ValueRef* exception() const { return exception_; }

 private:
  void* operator new(size_t size) { return ::operator new(size); }
  void operator delete(void* ptr) { ::operator delete(ptr); }

  void ReportPendingException(Escargot::ContextRef* context);

  Escargot::ContextRef* context_;
  Escargot::ValueRef* exception_;
  TryCatchScope* prev_scope_;
  std::string reason_;
  bool report_exception_;
  static thread_local TryCatchScope* current_scope;
};

class Eval {
  using F = std::function<ValueRef*(ExecutionStateRef* state)>;

 public:
  explicit Eval(F&& f) : functor_(std::move(f)) {}
  ~Eval() = default;

  Eval() = delete;
  Eval(const Eval&) = delete;
  Eval(Eval&&) = delete;
  Eval& operator=(const Eval&) = delete;
  Eval& operator=(Eval&&) = delete;

  ExecResult execute(ContextRef* context) {
    ExecResult r = Escargot::Evaluator::execute(context, Eval::Evaluator, this);
    if (!r) {
      // If false in any TryCatchScope created, assign the error to the scope.
      TryCatchScope::Catch(r.error.value(),
                           ExecResultHelper::GetErrorString(context, r));
    }
    return r;
  }

  static ExecResult execute(ContextRef* context, F&& f) {
    return Eval(std::move(f)).execute(context);
  }

 private:
  void* operator new(size_t size) { return ::operator new(size); }
  void operator delete(void* ptr) { ::operator delete(ptr); }

  static ValueRef* Evaluator(ExecutionStateRef* state, const Eval* self) {
    return self->functor_(state) ?: ValueRef::createUndefined();
  }

  F functor_;
};

class StringHelper {
 public:
  static bool IsAsciiString(StringRef* str);
};

void SetMethod(ExecutionStateRef* state,
               ObjectRef* target,
               StringRef* name,
               FunctionObjectRef::NativeFunctionPointer nativeFunction);

void SetMethod(ExecutionStateRef* state,
               ObjectRef* target,
               const char* name,
               FunctionObjectRef::NativeFunctionPointer nativeFunction);

void SetMethod(ContextRef* context,
               ObjectRef* target,
               const char* name,
               FunctionObjectRef::NativeFunctionPointer nativeFunction);

void SetMethod(ObjectTemplateRef* target,
               const char* name,
               size_t argument_count,
               FunctionTemplateRef::NativeFunctionPointer fn);

// Accessor
void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 StringRef* name,
                 ValueRef* value,
                 bool isWritable = false,
                 bool isEnumerable = true,
                 bool isConfigurable = false);

void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 const char* name,
                 ValueRef* value,
                 bool isWritable = false,
                 bool isEnumerable = true,
                 bool isConfigurable = false);

void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 const char* name,
                 ObjectRef::NativeDataAccessorPropertyGetter getter,
                 ObjectRef::NativeDataAccessorPropertySetter setter,
                 bool isWritable = false,
                 bool isEnumerable = true,
                 bool isConfigurable = false,
                 void* data = nullptr);

void SetProperty(ObjectTemplateRef* target,
                 const char* name,
                 FunctionTemplateRef::NativeFunctionPointer getter,
                 FunctionTemplateRef::NativeFunctionPointer setter,
                 bool isEnumerable = true,
                 bool isConfigurable = false);

bool GetProperty(ContextRef* context,
                 ObjectRef* target,
                 ValueRef* key,
                 ValueRef*& out);

FunctionTemplateRef* NewFunctionTemplate(
    size_t argument_count = 0,
    FunctionTemplateRef::NativeFunctionPointer fn = nullptr,
    bool is_constructor = true);

Maybe<ValueRef*> CallFunction(ContextRef* context,
                              ValueRef* receiver,
                              FunctionObjectRef* callback,
                              const size_t argc,
                              ValueRef** argv);

Maybe<ValueRef*> CallFunction(ContextRef* context,
                              ObjectRef* receiver,
                              StringRef* functionName,
                              const size_t argc,
                              ValueRef** argv);

template <typename T>
class PersistentHolder {
 public:
  ~PersistentHolder() { DestoryHolder(); }
  PersistentHolder() { holder_ = nullptr; }
  PersistentHolder(T* ptr) { InitHolder(ptr); }
  PersistentHolder(PersistentHolder<T>&& src) {
    holder_ = src.holder_;
    src.holder_ = nullptr;
  }
  const PersistentHolder<T>& operator=(PersistentHolder<T>&& src) {
    holder_ = src.holder_;
    src.holder_ = nullptr;

    return *this;
  }
  PersistentHolder(const PersistentHolder<T>&) = delete;
  const PersistentHolder<T>& operator=(const PersistentHolder<T>&) = delete;

  void reset(T* ptr) {
    if (!ptr) {
      DestoryHolder();
      return;
    }
    if (holder_ == nullptr) {
      InitHolder(ptr);
    } else {
      *holder_ = ptr;
    }
  }

  T* get() {
    // NOTE: Reason why we need this: Escargot::PersistentRefHolder::get() has a
    // bug which doesn't handle dereferencing null pointer. So it can't be used
    // to know if the holder is empty.
    return holder_ ? *holder_ : nullptr;
  }

  T* value() { return get(); }

  T* release() {
    if (holder_) {
      T* ptr = *holder_;
      DestoryHolder();
      return ptr;
    }
    return nullptr;
  }

  bool isEmpty() { return holder_ == nullptr; }

  // operators

  operator T*() {
    // NOTE: Reason why we need this: Escargot::PersistentRefHolder::T*()
    // asserts holder != nullptr
    return holder_ ? *holder_ : nullptr;
  }

  T* operator->() { return *holder_; }

 private:
  void InitHolder(T* initial_value) {
    holder_ = reinterpret_cast<T**>(Memory::gcMallocUncollectable(sizeof(T*)));
    *holder_ = initial_value;
  }

  void DestoryHolder() {
    if (holder_) {
      Memory::gcFree(holder_);
    }
    holder_ = nullptr;
  }
  T** holder_;
};

/*
  The WeakHolder class is designed to manipulate pointers in a way that hides
  them from the garbage collector (GC) within a GC-managed heap. By adding and
  removing an offset to the original pointer, this class ensures that the
  pointer is not tracked by the GC, effectively making it invisible to the GC's
  management mechanisms. This can be useful in scenarios where you need to
  maintain a reference to an object without preventing it from being collected
  by the GC.

  Pointers which is not tracked or cross-reference GC objects doesn't need to
  use this class. However it can be used to clarify relationships.
*/

template <typename T>
class WeakHolder {
 public:
  WeakHolder() = default;
  WeakHolder(T* p) : value_(p ? AddOffset(p) : nullptr) {}
  WeakHolder(const WeakHolder& other) : value_(other.value_) {}

  T* value() const { return value_ ? RemoveOffset(value_) : nullptr; }

  void Reset(T* p) { value_ = p ? AddOffset(p) : nullptr; }

  PersistentHolder<T> Lock() const {
    return value_ ? PersistentHolder<T>(RemoveOffset(value_)) : nullptr;
  }

  // operators

  T* operator->() const { return RemoveOffset(value_); }
  T& operator*() const { return *RemoveOffset(value_); }

  WeakHolder& operator=(const WeakHolder& other) {
    if (this != &other) {
      value_ = other.value_;
    }
    return *this;
  }

  bool operator!=(const T& other) const { return !operator==(other); }
  bool operator==(const T& other) const { return value() == other; }

  bool operator!=(const std::nullptr_t other) const { return value() != other; }
  bool operator==(const std::nullptr_t other) const { return value() == other; }

 private:
  T* value_ = nullptr;
  static const std::uintptr_t kGCDerefOffset = 1;

  static T* AddOffset(T* ptr) {
    return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(ptr) +
                                kGCDerefOffset);
  }

  static T* RemoveOffset(T* ptr) {
    return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(ptr) -
                                kGCDerefOffset);
  }
};

}  // namespace Escargot
