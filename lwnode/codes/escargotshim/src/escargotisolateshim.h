/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __ESCARGOT_ISOLATESHIM__
#define __ESCARGOT_ISOLATESHIM__

#include "escargotbase.h"
#include "escargotregistry.h"
#include "escargotshim.h"

#include "v8.h"

namespace Escargot {
class VMInstanceRef;
}  // namespace Escargot

namespace EscargotShim {

class IsolateShim;
class ContextShim;

typedef Escargot::Evaluator::EvaluatorResult EvaluatorResult;

enum class CachedStringId : std::size_t {
#define DEF(x, ...) x,
#include "escargotproperyid.inc"
  Count
};

enum class CachedSymbolId : std::size_t {
#define DEFSYMBOL(x, ...) x,
#include "escargotproperyid.inc"
  SymbolCount
};

typedef Registry<CachedStringId, JsStringRef> StringRegistry;
typedef Registry<CachedSymbolId, JsSymbolRef> SymbolRegistry;

// struct JsErrorInfo {
//   std::string scriptResourceName;
//   std::string stack;
//   std::string source;
//   int lineNumber{0};
//   int columnNumber{0};
// };

class AtomicValue : public gc {
 public:
  AtomicValue();
  void Dispose();

  DEFINE_GETTER(JsValueRef, Undefined);
  DEFINE_GETTER(JsValueRef, Null);
  DEFINE_GETTER(JsValueRef, True);
  DEFINE_GETTER(JsValueRef, False);
  DEFINE_GETTER(StringRegistry*, strings);
  // Note: GetCachedPropertyIdRef
  JsStringRef Strings(CachedStringId key) { return m_strings->get(key); }
  JsStringRef Strings(JsValueRef value);
  JsSymbolRef Symbols(CachedSymbolId key) { return m_symbols->get(key); }

 private:
  JsValueRef m_Undefined{JS_INVALID_REFERENCE};
  JsValueRef m_Null{JS_INVALID_REFERENCE};
  JsValueRef m_True{JS_INVALID_REFERENCE};
  JsValueRef m_False{JS_INVALID_REFERENCE};

  StringRegistry* m_strings{nullptr};
  SymbolRegistry* m_symbols{nullptr};
};  // namespace EscargotShim

class ScriptException : public gc {
 public:
  DEFINE_GETTER(v8::TryCatch*, tryCatchStackTop);
  void SetTryCatchStackTop(v8::TryCatch* section);
  bool HasException() { return m_exception != JS_INVALID_REFERENCE; }
  void SetException(const EvaluatorResult& result,
                    const JsEvaluator::StackTraceData* data = nullptr);
  void SetException(JsObjectRef exception) { m_exception = exception; }
  bool IsThrownException() { return m_isThrownException; }
  void ThrownException() { m_isThrownException = true; }
  void ClearException() {
    m_exception = JS_INVALID_REFERENCE;
    m_isThrownException = false;
  }
  JsObjectRef Exception() { return m_exception; }
  void ThrowExceptionToStateIfHasError(JsExecutionStateRef state);

 private:
  v8::TryCatch* m_tryCatchStackTop{JS_INVALID_REFERENCE};
  JsObjectRef m_exception{JS_INVALID_REFERENCE};
  bool m_isThrownException{false};
};

class IsolateShim : public gc {
 public:
  struct CreateParams {
    CreateParams() : array_buffer_allocator(nullptr) {}
    v8::ArrayBuffer::Allocator* array_buffer_allocator;
  };

  virtual ~IsolateShim();

  class LocalThreadData : public gc {
   public:
    LocalThreadData(IsolateShim* isolateShim) : m_isolateShim(isolateShim) {}
    IsolateShim* m_isolateShim{nullptr};
  };

  class LocalScopeData : public gc {
   public:
    LocalScopeData() {}
    void addGcObjectRef(void* gcObject) { m_gcObjects.push_back(gcObject); }
    void clear() { m_gcObjects.clear(); }

    GCList<void*> m_gcObjects;
  };

  /**
   * Allocates a new isolate but does not initialize it. Does not change the
   * currently entered isolate.
   *
   * Only Isolate::GetData() and Isolate::SetData(), which access the
   * embedder-controlled parts of the isolate, are allowed to be called on the
   * uninitialized isolate. To initialize the isolate, call
   * Isolate::Initialize().
   *
   * When an isolate is no longer used its resources should be freed
   * by calling Dispose().  Using the delete operator is not allowed.
   *
   * V8::Initialize() must have run prior to this.
   */
  static IsolateShim* Allocate();

  /**
   * Initialize an Isolate previously allocated by Isolate::Allocate().
   */
  static void Initialize(IsolateShim* isolate, const CreateParams& params);

  /**
   * Creates a new isolate.  Does not change the currently entered
   * isolate.
   *
   * When an isolate is no longer used its resources should be freed
   * by calling Dispose().  Using the delete operator is not allowed.
   *
   * V8::Initialize() must have run prior to this.
   */
  static IsolateShim* New(const CreateParams& params);

  /**
   * Returns the entered isolate for the current thread or NULL in
   * case there is no current isolate.
   *
   * This method must not be invoked before V8::Initialize() was invoked.
   */
  static IsolateShim* GetCurrent();

  /**
   * Sets this isolate as the entered one for the current thread.
   * Saves the previously entered one (if any), so that it can be
   * restored when exiting.  Re-entering an isolate is allowed.
   */
  void Enter();

  /**
   * Exits this isolate by restoring the previously entered one in the
   * current thread.  The isolate may still stay the same, if it was
   * entered more than once.
   *
   * Requires: this == Isolate::GetCurrent().
   */
  void Exit();

  /**
   * Disposes the isolate.  The isolate must not be entered by any
   * thread to be disposable.
   */
  void Dispose();

  static IsolateShim* ToIsolateShim(v8::Isolate* isolate);
  v8::Isolate* asIsolate();

  void pushContext(ContextShim* context);
  void popContext();

  ContextShim* newContextShim(bool exposeGC,
                              JsObjectRef globalObjectTemplateInstance);
  JsVMInstanceRef vmInstanceRef() { return m_instance; }

  ContextShim* currentContext();
  ContextShim* GetContextShimFromJsContext(JsContextRef context);

  bool HasPendingPromiseJob() { return m_instance->hasPendingPromiseJob(); }
  EvaluatorResult ExecutePendingPromiseJob() {
    return m_instance->executePendingPromiseJob();
  }

  ScriptException* GetScriptException() { return m_scriptException; }
  inline void SetScriptExecuted() { m_jsScriptExecuted = true; }
  inline void ResetScriptExecuted() { m_jsScriptExecuted = false; }

  bool AddMessageListener(void* that);
  void RemoveMessageListeners(void* that);

  template <typename Fn>
  void ForEachMessageListener(Fn fn) {
    for (auto i = m_messageListeners.begin(); i != m_messageListeners.end();
         i++) {
      fn(*i);
    }
  }

  void pushLocalScopeData();
  void popLocalScopeData();
  LocalScopeData* currentLocalScopeData();

  void addToLocalScopeData(void* jsObject);
  void addToPersistentStorage(void* jsObject);
  void removeFromPersistentStorage(void* jsObject);

  AtomicValue* Cached() { return m_atomicValue; }

  static JsValueRef GetEscargotShimJsArrayBuffer(JsContextRef context);

  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocator) {
    array_buffer_allocator_ = allocator;
  }
  v8::ArrayBuffer::Allocator* array_buffer_allocator() const {
    return array_buffer_allocator_;
  }

 private:
  IsolateShim();
  static IsolateShim* New();
  void clearPointerPersistentMap();
  void clearContextMap();

  Escargot::PersistentRefHolder<Escargot::VMInstanceRef> m_instance;
  GCList<ContextShim*> m_contextStack;
  GCList<LocalThreadData*> m_localThreadData;
  GCList<LocalScopeData*> m_localScopeDataStack;

  bool m_jsScriptExecuted{false};

  static THREAD_LOCAL IsolateShim* s_currentIsolate;

  std::vector<void*> m_messageListeners;
  AtomicValue* m_atomicValue{nullptr};
  ScriptException* m_scriptException{nullptr};

  // GC collectors
  GCUnorderedMap<void*, size_t> m_persistentMap;
  GCUnorderedMap<JsContextRef, ContextShim*> m_contextMap;
  // static inline void fillGCDescriptor(GC_word* desc) {
  //   GC_set_bit(desc, GC_WORD_OFFSET(IsolateShim, m_persistentMap));
  // }

  v8::ArrayBuffer::Allocator* array_buffer_allocator_ = nullptr;
};

}  // namespace EscargotShim

#endif
