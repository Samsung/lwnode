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

#include "context.h"
#include "isolate.h"

namespace EscargotShim {

static bool createGlobals(ContextRef* context) {
#if defined(HOST_TIZEN)
// @todo setup device APIs
#endif
  return true;
}

ContextWrap::ContextWrap(IsolateWrap* isolate) {
  isolate_ = isolate;
  context_ = ContextRef::create(isolate->vmInstance());

  createGlobals(context_);
}

ContextWrap* ContextWrap::New(IsolateWrap* isolate) {
  LWNODE_CHECK_NOT_NULL(isolate);
  auto context = new ContextWrap(isolate);
  return context;
}

void ContextWrap::Enter() {
  isolate_->Enter();
  isolate_->pushContext(this);
}

void ContextWrap::Exit() {
  isolate_->popContext(this);
}

IsolateWrap* ContextWrap::GetIsolate() {
  return isolate_;
}

void ContextWrap::SetEmbedderData(int index, ValueWrap* value) {
  if (embedder_data_ == nullptr) {
    embedder_data_ = new EmbedderDataMap();
  }
  LWNODE_DLOG_INFO("set: EmbedderData: idx %d", index);
  embedder_data_->insert(std::make_pair(index, value));
}

ValueWrap* ContextWrap::GetEmbedderData(int index) {
  auto iter = embedder_data_->find(index);
  if (iter != embedder_data_->end()) {
    return iter->second;
  }
  return nullptr;
}

void ContextWrap::SetAlignedPointerInEmbedderData(int index, void* value) {
  LWNODE_DLOG_INFO("set: AlignedPointerInEmbedderData: idx %d", index);
  aligned_pointer_in_embedder_data_.insert(std::make_pair(index, value));
}

void* ContextWrap::GetAlignedPointerFromEmbedderData(int index) {
  auto iter = aligned_pointer_in_embedder_data_.find(index);
  if (iter != aligned_pointer_in_embedder_data_.end()) {
    return iter->second;
  }
  return nullptr;
}

}  // namespace EscargotShim

// namespace EscargotShim
