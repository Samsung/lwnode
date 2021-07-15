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
#include <vector>

#include "utils/gc.h"

using namespace Escargot;

namespace EscargotShim {

class Platform : public PlatformRef {
 public:
  static Platform* GetInstance();
  static void Dispose();

  void markJSJobEnqueued(ContextRef* relatedContext) override;
  void* onMallocArrayBufferObjectDataBuffer(size_t sizeInByte) override;
  void onFreeArrayBufferObjectDataBuffer(void* buffer,
                                         size_t sizeInByte) override;

  LoadModuleResult onLoadModule(ContextRef* relatedContext,
                                ScriptRef* whereRequestFrom,
                                StringRef* moduleSrc) override;
  void didLoadModule(ContextRef* relatedContext,
                     OptionalRef<ScriptRef> referrer,
                     ScriptRef* loadedModule) override;
  void hostImportModuleDynamically(ContextRef* relatedContext,
                                   ScriptRef* referrer,
                                   StringRef* src,
                                   PromiseObjectRef* promise) override;

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

#define GC_HEAP_TRACE_ONLY

class GCHeap : public gc {
 public:
  void GlobalizeReference(void* address, void* data);
  void DisposeGlobal(void* address);
  void MakeWeak(void* address);
  void MakeWeak(void* location,
                void* parameter,
                v8::WeakCallbackInfo<void>::Callback weak_callback,
                v8::WeakCallbackType type);
  void ClearWeak(void* address);
  void disposePhantomWeak(void* address);
  bool isTraced(void* address);
  void* getPersistentData(void* address);
  void printStatus();

  typedef GC_word GC_heap_pointer;
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

 private:
  void notifyUpdate(void* address);

  GCUnorderedMap<GC_heap_pointer, AddressInfo> persistents_;
  GCUnorderedMap<GC_heap_pointer, AddressInfo> weakPhantoms_;
  bool isStatusPrinted = false;
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

 private:
  Engine() = default;
  void initialize();
  void dispose();
  void disposeExternalStrings();

  static std::unordered_set<v8::String::ExternalStringResourceBase*>
      s_externalStrings;

  PersistentRefHolder<GCHeap> gcHeap_;
};
}  // namespace EscargotShim
