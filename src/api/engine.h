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

#include <EscargotPublic.h>
#include <string.h>
#include <v8.h>
#include <thread>
#include <vector>

#include "utils/gc-util.h"

using namespace Escargot;

namespace EscargotShim {

class Platform : public PlatformRef {
 public:
  static Platform* GetInstance();
  static void Dispose();

  void markJSJobEnqueued(ContextRef* relatedContext) override;
  void markJSJobFromAnotherThreadExists(ContextRef* relatedContext) override;
  void* onMallocArrayBufferObjectDataBuffer(size_t sizeInByte) override;
  void onFreeArrayBufferObjectDataBuffer(void* buffer,
                                         size_t sizeInByte) override;
  void* onReallocArrayBufferObjectDataBuffer(void* oldBuffer,
                                             size_t oldSizeInByte,
                                             size_t newSizeInByte) override;

  LoadModuleResult onLoadModule(ContextRef* relatedContext,
                                ScriptRef* whereRequestFrom,
                                StringRef* moduleSrc,
                                ModuleType type) override;
  void didLoadModule(ContextRef* relatedContext,
                     OptionalRef<ScriptRef> referrer,
                     ScriptRef* loadedModule) override;
  void hostImportModuleDynamically(ContextRef* relatedContext,
                                   ScriptRef* referrer,
                                   StringRef* src,
                                   ModuleType type,
                                   PromiseObjectRef* promise) override;

  void customInfoLogger(const char* format, va_list args) override {}
  void customErrorLogger(const char* format, va_list args) override {}

  std::vector<std::tuple<std::string /* abs path */,
                         ContextRef*,
                         PersistentRefHolder<ScriptRef>>>
      loadedModules;

  void setAllocator(v8::ArrayBuffer::Allocator* allocator) {
    allocator_ = allocator;
  }

 private:
  Platform() = default;
  v8::ArrayBuffer::Allocator* allocator_ = nullptr;
};

class GCHeap : public gc {
 public:
  enum Kind {
    FREE = 0,
    STRONG,
    WEAK,
  };

  struct AddressInfo {
    AddressInfo(int strong_, int weak_, void* data_ = nullptr) {
      strong = strong_;
      weak = weak_;
      data = data_;
    }
    int strong = 0;
    int weak = 0;
    void* data = nullptr;
  };
  typedef GC_word GC_heap_pointer;
  typedef std::pair<GC_heap_pointer, AddressInfo> HeapSegment;

  void acquire(void* address, Kind kind, void* data);
  void release(void* address, Kind kind);
  void disposePhantomWeak(void* address);
  bool isTraced(void* address);
  void printStatus(bool forcePrint = false);

  class ProcessingHoldScope {
   public:
    ProcessingHoldScope();
    ~ProcessingHoldScope();

    static bool isSkipProcessing();

   private:
    static std::vector<ProcessingHoldScope*> s_processingHoldScopes_;
  };

  void postGarbageCollectionProcessing();
  static void processGCEvent(void* data);

  static GCHeap* create() { return new GCHeap(); }

 private:
  void postUpdate(void* address);

  GCUnorderedMap<GC_heap_pointer, AddressInfo> persistents_;
  GCUnorderedMap<GC_heap_pointer, AddressInfo> weakPhantoms_;
  bool isStatePrinted_ = false;
  bool isOnPostGarbageCollectionProcessing_ = false;
  struct Stat {
    size_t freed = 0;
    size_t weak = 0;
  };

  Stat stat_;
};

class Engine {
 public:
  static bool Initialize();
  static bool Dispose();

  static Engine* current();
  static void registerExternalString(
      v8::String::ExternalStringResourceBase* v8Str);
  static void unregisterExternalString(
      v8::String::ExternalStringResourceBase* v8Str);

  GCHeap* gcHeap() { return gcHeap_.get(); }

  void registerGCEventListeners();
  void unregisterGCEventListeners();

  enum State {
    Freed,
    Running,
    OnDestroy,
  };

  static State getState();

  void initializeThread();
  void finalizeThread();

 private:
  Engine() = default;
  void initialize();
  void dispose();
  void disposeExternalStrings();

  static std::unordered_set<v8::String::ExternalStringResourceBase*>
      s_externalStrings;

  PersistentRefHolder<GCHeap> gcHeap_;
  std::thread::id mainThreadId_;
};
}  // namespace EscargotShim
