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
  Platform(v8::ArrayBuffer::Allocator* allocator);

  void didPromiseJobEnqueued(ContextRef* relatedContext,
                             PromiseObjectRef* obj) override;
  void* onArrayBufferObjectDataBufferMalloc(ContextRef* whereObjectMade,
                                            ArrayBufferObjectRef* obj,
                                            size_t sizeInByte) override;
  void onArrayBufferObjectDataBufferFree(ContextRef* whereObjectMade,
                                         ArrayBufferObjectRef* obj,
                                         void* buffer) override;
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

 private:
  v8::ArrayBuffer::Allocator* allocator_ = nullptr;
};

class Engine : public gc {
 public:
  Engine() = default;

  static bool Initialize();
  static bool Dispose();

  static bool evalScript(ContextRef* context,
                         const char* str,
                         const char* fileName,
                         bool shouldPrintScriptResult,
                         bool isModule);
  static bool evalScript(ContextRef* context,
                         StringRef* str,
                         StringRef* fileName,
                         bool shouldPrintScriptResult,
                         bool isModule);
  static bool createDefaultGlobals(ContextRef* context);

 private:
  void initialize();
  void finalize();
};
}  // namespace EscargotShim
