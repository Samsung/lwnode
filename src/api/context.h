/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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
#include "utils/gc.h"

namespace EscargotShim {

class IsolateWrap;
class ValueWrap;

// typedef GCContainer<const ValueWrap*> EmbedderData;
typedef GCUnorderedMap<int, const ValueWrap*> EmbedderDataMap;

class ContextWrap : public gc {
 public:
  static ContextWrap* New(IsolateWrap* isolate);

  void Enter();
  void Exit();
  IsolateWrap* GetIsolate();

  Escargot::ContextRef* get() { return context_; }

  void SetEmbedderData(int index, const ValueWrap* value);
  const ValueWrap* GetEmbedderData(int index);

 private:
  static constexpr int kEmbedderDataSize = 2;
  EmbedderDataMap* embedder_data_ = nullptr;

  ContextWrap(IsolateWrap* isolate);

  IsolateWrap* isolate_ = nullptr;
  Escargot::ContextRef* context_ = nullptr;
};

}  // namespace EscargotShim
