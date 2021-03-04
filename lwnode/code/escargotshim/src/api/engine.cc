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
#include "flags.h"
#include "utils/logger.h"
#include "utils/string.h"

using namespace Escargot;

namespace EscargotShim {

// --- P l a t f o r m ---
Platform::Platform(v8::ArrayBuffer::Allocator* allocator) {
  LWNODE_CHECK_NOT_NULL(allocator);
  allocator_ = allocator;
}

void Platform::markJSJobEnqueued(ContextRef* relatedContext) {
  LWNODE_UNIMPLEMENT;
}

void* Platform::onArrayBufferObjectDataBufferMalloc(ContextRef* whereObjectMade,
                                                    ArrayBufferObjectRef* obj,
                                                    size_t sizeInByte) {
  return allocator_->Allocate(sizeInByte);
}

void Platform::onArrayBufferObjectDataBufferFree(ContextRef* whereObjectMade,
                                                 ArrayBufferObjectRef* obj,
                                                 void* buffer) {
  return allocator_->Free(buffer, obj->byteLength());
}

PlatformRef::LoadModuleResult Platform::onLoadModule(
    ContextRef* relatedContext,
    ScriptRef* whereRequestFrom,
    StringRef* moduleSrc) {
  LWNODE_UNIMPLEMENT;

  return LoadModuleResult(nullptr);
}

void Platform::didLoadModule(ContextRef* relatedContext,
                             OptionalRef<ScriptRef> referrer,
                             ScriptRef* loadedModule) {
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

bool Engine::Initialize() {
  if (s_engine == nullptr) {
    s_engine = new Engine();
    s_engine->initialize();
  }
  return true;
}

bool Engine::Dispose() {
  if (s_engine) {
    s_engine->finalize();
    s_engine = nullptr;
  }

  return true;
}

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
  Memory::setGCFrequency(24);

  auto flags = Flags::get();
  if (flags & FlagType::TraceGC) {
    MemoryUtil::startGCStatsTrace();
  }
}

void Engine::finalize() {
  Globals::finalize();
  MemoryUtil::collectAllGarbage();
}

}  // namespace EscargotShim
