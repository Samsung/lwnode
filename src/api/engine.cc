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

#include "engine.h"
#include <iomanip>
#include <sstream>
#include "utils/logger.h"
#include "utils/misc.h"
#include "utils/string.h"

using namespace Escargot;

namespace EscargotShim {

static Platform* s_platform;

Platform* Platform::GetInstance() {
  if (s_platform == nullptr) {
    s_platform = new Platform();
  }
  return s_platform;
}

void Platform::Dispose() {
  LWNODE_CALL_TRACE_GC_START();
  delete s_platform;
  s_platform = nullptr;
  LWNODE_CALL_TRACE_GC_END();
}

// --- P l a t f o r m ---

void Platform::markJSJobEnqueued(ContextRef* relatedContext) {
  // @note the timing to handle pending jobs depends on clients
}

void* Platform::onMallocArrayBufferObjectDataBuffer(size_t sizeInByte) {
  return allocator_->Allocate(sizeInByte);
}

void Platform::onFreeArrayBufferObjectDataBuffer(void* buffer,
                                                 size_t sizeInByte) {
  return allocator_->Free(buffer, sizeInByte);
}

PlatformRef::LoadModuleResult Platform::onLoadModule(
    ContextRef* relatedContext,
    ScriptRef* whereRequestFrom,
    StringRef* moduleSrc) {
  LWNODE_DLOG_INFO("onLoadModule: %s", moduleSrc->toStdUTF8String().c_str());
  LWNODE_UNIMPLEMENT;

  return LoadModuleResult(nullptr);
}

void Platform::didLoadModule(ContextRef* relatedContext,
                             OptionalRef<ScriptRef> referrer,
                             ScriptRef* loadedModule) {
  LWNODE_DLOG_INFO("didModule");
  LWNODE_UNIMPLEMENT;
}

void Platform::hostImportModuleDynamically(ContextRef* relatedContext,
                                           ScriptRef* referrer,
                                           StringRef* src,
                                           PromiseObjectRef* promise) {
  LWNODE_UNIMPLEMENT;
}

// --- G C H e a p ---
#if !defined(GC_HEAP_TRACE_ONLY)
#define GC_WRAP_PERSISTENT_POINTER(p) (GC_heap_pointer)(p)
#define GC_UNWRAP_POINTER(p) ((void*)p)
#else
#define GC_WRAP_PERSISTENT_POINTER(p) GC_HIDE_POINTER(p)
#define GC_UNWRAP_POINTER(p) ((void*)GC_HIDE_POINTER(p))
#endif

/*
  @note gc heap tracing lifetime

  a) The types of heap tracing are strong, weak and phantom weak.
  b) Every persitent is created as strong type at the registration.
     Following flow is considered:

     i)   strong <-> weak <-> phantom weak -> (finalizer) -> nullptr
     ii)  strong -> nullptr
     iii) phantom weak -> strong

  If weak counter is 1, then the pointer will be phantom weak which is
  collectable for GC.
*/

void GCHeap::GlobalizeReference(void* address, void* data) {
  LWNODE_CALL_TRACE("address %p, data %p", address, data);
  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    iter->second.strong++;
    iter->second.weak--;
    iter->second.weak = std::max(iter->second.weak, 0);
    // no-progress handling weak phantoms
  } else {
    auto iter = weakPhantoms_.find(GC_HIDE_POINTER(address));
    if (iter != weakPhantoms_.end()) {
      // move phantom weak to strong
      AddressInfo info = iter->second;

      LWNODE_CHECK(info.strong == 0);
      LWNODE_CHECK(info.weak == 1);
      info.strong++;
      info.weak--;

      persistents_.emplace(GC_WRAP_PERSISTENT_POINTER(address), info);
      weakPhantoms_.erase(iter);
      // no-progress handling weak phantoms
    } else {
      persistents_.emplace(GC_WRAP_PERSISTENT_POINTER(address),
                           AddressInfo(1, 0, data));
    }
  }
  notifyUpdate(address);
}

void GCHeap::DisposeGlobal(void* address) {
  LWNODE_CALL_TRACE("address %p", address);
  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    iter->second.strong--;
    iter->second.strong = std::max(iter->second.strong, 0);

    // progress handling weak phantoms
    if (iter->second.strong == 0) {
      if (iter->second.weak == 0) {
        persistents_.erase(iter);
      } else if (iter->second.weak <= 1) {
        weakPhantoms_.emplace(GC_HIDE_POINTER(address), iter->second);
        persistents_.erase(iter);
      }
    }
  }
  notifyUpdate(address);
}

void GCHeap::MakeWeak(void* address) {
  LWNODE_CALL_TRACE("address %p", address);
  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    iter->second.strong--;
    iter->second.strong = std::max(iter->second.strong, 0);
    iter->second.weak++;

    // progress handling weak phantoms
    if (iter->second.strong == 0 && iter->second.weak <= 1) {
      weakPhantoms_.emplace(GC_HIDE_POINTER(address), iter->second);
      persistents_.erase(iter);
    }
  } else {
    auto iter = weakPhantoms_.find(GC_HIDE_POINTER(address));
    if (iter != weakPhantoms_.end()) {
      // move phantom weak to weak
      AddressInfo info = iter->second;

      LWNODE_CHECK(info.strong == 0);
      LWNODE_CHECK(info.weak == 1);
      info.weak++;

      persistents_.emplace(GC_WRAP_PERSISTENT_POINTER(address), info);
      weakPhantoms_.erase(iter);
    } else {
      LWNODE_CHECK(false);  // assumes this doesn't happen. let's see.
    }
  }

  notifyUpdate(address);
}

void GCHeap::ClearWeak(void* address) {
  LWNODE_CALL_TRACE("address %p", address);
  // 1. handle persistents_ and weakPhantoms_
  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    // guard: ClearWeak can be called even if no weak reference exist.
    if (iter->second.weak > 0) {
      iter->second.weak--;
    }

    // progress handling weak phantoms
    if (iter->second.strong <= 0 && iter->second.weak <= 1) {
      weakPhantoms_.emplace(GC_HIDE_POINTER(address), iter->second);
      persistents_.erase(iter);
    }
  }

  // 2. ensure clearing finalizer bound to this address
  MemoryUtil::gcRegisterFinalizer(address, nullptr, nullptr);

  notifyUpdate(address);
}

void GCHeap::disposePhantomWeak(void* address) {
  LWNODE_CALL_TRACE("address %p", address);
  auto iter = weakPhantoms_.find(GC_HIDE_POINTER(address));
  if (iter != weakPhantoms_.end()) {
    weakPhantoms_.erase(iter);
  }
  notifyUpdate(address);
}

bool GCHeap::isTraced(void* address) {
  if (persistents_.find(GC_WRAP_PERSISTENT_POINTER(address)) !=
      persistents_.end()) {
    return true;
  }

  if (weakPhantoms_.find(GC_HIDE_POINTER(address)) != weakPhantoms_.end()) {
    return true;
  }
  return false;
}

void* GCHeap::getPersistentData(void* address) {
  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    return iter->second.data;
  }

  auto iterWeak = weakPhantoms_.find(GC_HIDE_POINTER(address));
  if (iterWeak != weakPhantoms_.end()) {
    return iterWeak->second.data;
  }
  return nullptr;
}

static void printAddress(
    const GCUnorderedMap<GCHeap::GC_heap_pointer, GCHeap::AddressInfo>& map) {
  std::vector<std::string> vector;
  std::stringstream ss;
  const int row = 5;
  int count = 0;
  for (auto iter : map) {
    ss << std::setw(15) << std::right << GC_UNWRAP_POINTER(iter.first) << " ("
       << "S" << std::setw(3) << iter.second.strong << " W" << std::setw(3)
       << iter.second.weak << ") ";
    if (++count % row == 0) {
      vector.push_back(ss.str());
      ss.str("");
    }
  }
  if (count % row) {
    vector.push_back(ss.str());
  }
  for (auto it : vector) {
    LWNODE_LOG_INFO("%s", it.c_str());
  }
}

void GCHeap::printStatus() {
  if (isStatusPrinted) {
    return;
  }
  isStatusPrinted = true;

  if (persistents_.size() == 0 && weakPhantoms_.size() == 0) {
    return;
  }

  LWNODE_LOG_INFO(COLOR_GREEN "----- GCHEAP -----" COLOR_RESET);
  LWNODE_LOG_INFO("[HOLD]");
  printAddress(persistents_);
  LWNODE_LOG_INFO(COLOR_GREEN "------------------" COLOR_RESET);
  LWNODE_LOG_INFO("[PHANTOM]");
  printAddress(weakPhantoms_);
  LWNODE_LOG_INFO(COLOR_GREEN "------------------" COLOR_RESET);
}

void GCHeap::notifyUpdate(void* address) {
  isStatusPrinted = false;
}

void GCHeap::MakeWeak(void* location,
                      void* parameter,
                      v8::WeakCallbackInfo<void>::Callback weak_callback,
                      v8::WeakCallbackType type) {
  LWNODE_CALL_TRACE("address %p", location);

  if (type != v8::WeakCallbackType::kParameter) {
    LWNODE_CHECK(false);
  }

#if !defined(GC_HEAP_TRACE_ONLY)
  // 1. register the given finalizer
  struct Params {
    v8::Isolate* isolate;
    void* parameter;
    v8::WeakCallbackInfo<void>::Callback weak_callback;
  };

  Params* params = new Params();

  LWNODE_CHECK(isTraced(location));

  v8::Isolate* v8Isolate =
      reinterpret_cast<v8::Isolate*>(getPersistentData(location));

  LWNODE_CHECK_NOT_NULL(v8Isolate);

  params->isolate = v8Isolate;
  params->parameter = parameter;
  params->weak_callback = weak_callback;

  MemoryUtil::gcRegisterFinalizer(
      location,
      [](void* address, void* data) {
        Engine::current()->gcHeap()->disposePhantomWeak(address);
        Params* params = (Params*)data;
        void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {};
        v8::WeakCallbackInfo<void> info(
            params->isolate, params->parameter, embedderFields, nullptr);
        params->weak_callback(info);
        delete params;
      },
      params);
#endif

  // 2. make this location as weak type
  MakeWeak(location);
}

// --- E n g i n e ---

static Engine* s_engine;
std::unordered_set<v8::String::ExternalStringResourceBase*>
    Engine::s_externalStrings;

bool Engine::Initialize() {
  if (s_engine == nullptr) {
    s_engine = new Engine();
    s_engine->initialize();
  }
  return true;
}

bool Engine::Dispose() {
  LWNODE_CALL_TRACE_GC_START();
  if (s_engine) {
    s_engine->dispose();
    delete s_engine;
    s_engine = nullptr;
  }

  LWNODE_CALL_TRACE_GC_END();
  return true;
}

#define GC_FREE_SPACE_DIVISOR 24

void Engine::initialize() {
#ifndef NDEBUG
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
#endif

#ifdef M_MMAP_THRESHOLD
  mallopt(M_MMAP_THRESHOLD, 2048);
#endif
#ifdef M_MMAP_MAX
  mallopt(M_MMAP_MAX, 1024 * 1024);
#endif

  Globals::initialize();
  Memory::setGCFrequency(GC_FREE_SPACE_DIVISOR);
  gcHeap_.reset(new GCHeap());

  if (Flags::isTraceCallEnabled("HEAP")) {
    Memory::setGCEventListener([]() {
      // this is invoked at GC_EVENT_RECLAIM_END phase
      Engine::current()->gcHeap()->printStatus();
    });
  }

  auto flags = Flags::get();
  if (Flags::isTraceGCEnabled()) {
    MemoryUtil::gcStartStatsTrace();
  }
}

void Engine::dispose() {
  LWNODE_CALL_TRACE_GC_START();

  Memory::setGCEventListener(nullptr);

  gcHeap_.release();
  MemoryUtil::gc();
  GC_invoke_finalizers();

  Globals::finalize();
  disposeExternalStrings();
  LWNODE_CALL_TRACE_GC_END();
}

Engine* Engine::current() {
  LWNODE_CHECK(s_engine);
  return s_engine;
}

void Engine::registerExternalString(
    v8::String::ExternalStringResourceBase* v8Str) {
  s_externalStrings.emplace(v8Str);
}

void Engine::unregisterExternalString(
    v8::String::ExternalStringResourceBase* v8Str) {
  s_externalStrings.erase(v8Str);
}

void Engine::disposeExternalStrings() {
  std::vector<v8::String::ExternalStringResourceBase*> strToDispose;

  for (auto str : s_externalStrings) {
    strToDispose.push_back(str);
  }
  for (auto str : strToDispose) {
    str->Dispose();
  }

  s_externalStrings.clear();
}

}  // namespace EscargotShim
