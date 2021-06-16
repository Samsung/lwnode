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

  auto flags = Flags::get();
  if (Flags::isTraceGCEnabled()) {
    MemoryUtil::gcStartStatsTrace();
  }
}

void Engine::dispose() {
  LWNODE_CALL_TRACE_GC_START();
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
