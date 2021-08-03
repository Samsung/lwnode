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
#include "handle.h"
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
  // s_platform is freed in Escargot::PlatformBridge
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

#define GC_WRAP_PERSISTENT_POINTER(p) (GC_heap_pointer)(p)
#define GC_UNWRAP_PERSISTENT_POINTER(p) ((void*)p)
#define GC_WRAP_WEAK_POINTER(p) (GC_heap_pointer)(p)
#define GC_UNWRAP_WEAK_POINTER(p) ((void*)p)

/*
  state diagram:
  FREE -> STRONG <-> WEAK -> (Post GC Processing) -> { STRONG, WEAK, FREE }
*/

void GCHeap::acquire(void* address, Kind kind, void* data) {
  LWNODE_CALL_TRACE_ID(
      GCHEAP,
      "%s kind %u data %p",
      PersistentWrap::as(address)->getPersistentInfoString().c_str(),
      kind,
      data);

  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    if (kind == STRONG) iter->second.strong++;
    if (kind == WEAK) iter->second.weak++;
  } else {
    auto iter = weakPhantoms_.find(GC_WRAP_WEAK_POINTER(address));
    if (iter != weakPhantoms_.end()) {
      AddressInfo info = iter->second;

      LWNODE_CHECK(info.strong == 0);
      LWNODE_CHECK(info.weak != 0);

      if (kind == STRONG) iter->second.strong++;
      if (kind == WEAK) iter->second.weak++;

      persistents_.emplace(GC_WRAP_PERSISTENT_POINTER(address), info);
      weakPhantoms_.erase(iter);
    } else {
      persistents_.emplace(GC_WRAP_PERSISTENT_POINTER(address),
                           AddressInfo(1, 0, data));
    }
  }
  postUpdate(address);
}

void GCHeap::release(void* address, Kind kind) {
  LWNODE_CALL_TRACE_ID(
      GCHEAP,
      "%s kind %u",
      PersistentWrap::as(address)->getPersistentInfoString().c_str(),
      kind);

  auto iter = persistents_.find(GC_WRAP_PERSISTENT_POINTER(address));
  if (iter != persistents_.end()) {
    if (kind == STRONG) iter->second.strong--;
    if (kind == WEAK) iter->second.weak--;

    iter->second.strong = std::max(iter->second.strong, 0);
    iter->second.weak = std::max(iter->second.weak, 0);

    // progress handling weak phantoms
    if (iter->second.strong == 0) {
      if (iter->second.weak > 0) {
        weakPhantoms_.emplace(GC_WRAP_WEAK_POINTER(address), iter->second);
      }
      persistents_.erase(iter);
    }
  }
  postUpdate(address);
}

void GCHeap::postGarbageCollectionProcessing() {
  if (isOnPostGarbageCollectionProcessing_ ||
      ProcessingHoldScope::isSkipProcessing()) {
    return;
  }

  LWNODE_CALL_TRACE_ID(
      GCHEAP, "last: %zu, current: %zu", stat_.weak, weakPhantoms_.size());
  if (stat_.weak == weakPhantoms_.size()) {
    return;
  }
  isOnPostGarbageCollectionProcessing_ = true;

  // 1. move weaks to process post task.
  GCVector<HeapSegment> weaks;
  weaks.reserve(weakPhantoms_.size());
  for (const HeapSegment& it : weakPhantoms_) {
    weaks.push_back(it);
  }
  weakPhantoms_.clear();

  // 2. invoke finalizers
  for (const auto& iter : weaks) {
    PersistentWrap* persistent =
        PersistentWrap::as(GC_UNWRAP_PERSISTENT_POINTER(iter.first));
    persistent->invokeFinalizer();
  }

  stat_.weak = weakPhantoms_.size();
  isOnPostGarbageCollectionProcessing_ = false;
}

bool GCHeap::isTraced(void* address) {
  if (persistents_.find(GC_WRAP_PERSISTENT_POINTER(address)) !=
      persistents_.end()) {
    return true;
  }

  if (weakPhantoms_.find(GC_WRAP_WEAK_POINTER(address)) !=
      weakPhantoms_.end()) {
    return true;
  }

  return false;
}

void GCHeap::disposePhantomWeak(void* address) {
  LWNODE_CALL_TRACE_ID(
      GCHEAP,
      "%s",
      PersistentWrap::as(address)->getPersistentInfoString().c_str());
  stat_.freed++;
  auto iter = weakPhantoms_.find(GC_WRAP_WEAK_POINTER(address));
  if (iter != weakPhantoms_.end()) {
    weakPhantoms_.erase(iter);
  }
  postUpdate(address);
}

typedef void (*formatterFunction)(std::stringstream& stream,
                                  const GCHeap::HeapSegment& iter);

static void printAddress(
    const GCUnorderedMap<GCHeap::GC_heap_pointer, GCHeap::AddressInfo>& map,
    formatterFunction formatter,
    const int column = 4) {
  std::stringstream ss;
  std::vector<std::string> vector;
  int count = 0;
  for (const auto& iter : map) {
    formatter(ss, iter);
    if (++count % column == 0) {
      vector.push_back(ss.str());
      ss.str("");
    }
  }
  if (count % column) {
    vector.push_back(ss.str());
  }
  for (const auto& it : vector) {
    LWNODE_LOG_INFO("%s", it.c_str());
  }
}

void GCHeap::printStatus(bool forcePrint) {
  if (Flags::isTraceCallEnabled("GCHEAP") == false) {
    return;
  }

  if (!forcePrint && isStatePrinted_) {
    return;
  }

  isStatePrinted_ = true;

  if (!forcePrint || (persistents_.size() == 0 && weakPhantoms_.size() == 0)) {
    return;
  }

  LWNODE_DLOG_INFO(CLR_GREEN "----- GCHEAP -----" CLR_RESET);
  LWNODE_DLOG_INFO("[STAT]");
  LWNODE_DLOG_INFO("     freed: %zu", stat_.freed);
  LWNODE_DLOG_INFO("    strong: %zu", persistents_.size());
  LWNODE_DLOG_INFO("      weak: %zu", weakPhantoms_.size());
  LWNODE_DLOG_INFO("[HOLD]");
  printAddress(persistents_,
               [](std::stringstream& stream, const HeapSegment& iter) {
                 stream << std::setw(15) << std::right
                        << GC_UNWRAP_PERSISTENT_POINTER(iter.first) << " ("
                        << "S" << std::setw(3) << iter.second.strong << " W"
                        << std::setw(3) << iter.second.weak << ") ";
               });

  LWNODE_DLOG_INFO(CLR_GREEN "------------------" CLR_RESET);
  LWNODE_DLOG_INFO("[PHANTOM]");
  printAddress(weakPhantoms_,
               [](std::stringstream& stream, const HeapSegment& iter) {
                 stream << std::setw(15) << std::right
                        << GC_UNWRAP_WEAK_POINTER(iter.first) << " ("
                        << "S" << std::setw(3) << iter.second.strong << " W"
                        << std::setw(3) << iter.second.weak << ") ";
               });

  LWNODE_DLOG_INFO(CLR_GREEN "------------------" CLR_RESET);
}

void GCHeap::postUpdate(void* address) {
  isStatePrinted_ = false;
}

void GCHeap::processGCEvent() {
  auto gcHeap = Engine::current()->gcHeap();
  gcHeap->printStatus();
  gcHeap->postGarbageCollectionProcessing();
}

std::vector<GCHeap::ProcessingHoldScope*>
    GCHeap::ProcessingHoldScope::s_processingHoldScopes_;

GCHeap::ProcessingHoldScope::ProcessingHoldScope() {
  s_processingHoldScopes_.push_back(this);
}

GCHeap::ProcessingHoldScope::~ProcessingHoldScope() {
  s_processingHoldScopes_.pop_back();
}

bool GCHeap::ProcessingHoldScope::isSkipProcessing() {
  if (s_processingHoldScopes_.empty()) {
    return false;
  }
  return true;
}

// --- E n g i n e ---

static Engine* s_engine;
std::unordered_set<v8::String::ExternalStringResourceBase*>
    Engine::s_externalStrings;
static Engine::State s_state = Engine::Freed;

bool Engine::Initialize() {
  if (s_engine == nullptr) {
    s_engine = new Engine();
    s_engine->initialize();
    s_state = Running;
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
  s_state = Freed;
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

  Globals::initialize(Platform::GetInstance());
  Memory::setGCFrequency(GC_FREE_SPACE_DIVISOR);
  gcHeap_.reset(GCHeap::create());

#if defined(GC_HEAP_TRACE_ONLY)
  // this is invoked at GC_EVENT_RECLAIM_END phase
  Memory::setGCEventListener(GCHeap::processGCEvent);
#endif

  auto flags = Flags::get();
  if (Flags::isTraceGCEnabled()) {
    LWNODE_DLOG_WARN("temporary blocked for postGarbageCollectionProcessing");
    // MemoryUtil::gcStartStatsTrace();
  }
}

void Engine::dispose() {
  LWNODE_CALL_TRACE_GC_START();
  s_state = OnDestroy;

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

Engine::State Engine::getState() {
  return s_state;
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
