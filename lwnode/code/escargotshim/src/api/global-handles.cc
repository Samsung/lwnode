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

#include <EscargotPublic.h>
#include <algorithm>

#include "api/isolate.h"
#include "base.h"
#include "global-handles.h"

namespace v8 {
namespace internal {
void GlobalHandles::Destroy(EscargotShim::ValueWrap* lwValue) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  isolate->global_handles()->destroy(lwValue);
}

void GlobalHandles::MakeWeak(EscargotShim::ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  isolate->global_handles()->makeWeak(lwValue, parameter, callback);
}

void* GlobalHandles::ClearWeakness(EscargotShim::ValueWrap* lwValue) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  isolate->global_handles()->clearWeakness(lwValue);
  return nullptr;
}

}  // namespace internal
}  // namespace v8

namespace EscargotShim {

GlobalHandles::Node::Node(void* parameter,
                          v8::WeakCallbackInfo<void>::Callback callback)
    : parameter_(parameter), callback_(callback) {}

GlobalHandles::Node::~Node() {}

GlobalHandles::NodeBlock::NodeBlock(IsolateWrap* isolate,
                                    ValueWrap* value,
                                    uint32_t count)
    : isolate_(isolate), value_(value), usedNodes_(count) {
  holder_.reset(value_);
}

GlobalHandles::NodeBlock::~NodeBlock() {
  delete firstNode_;
  holder_.release();
}

GlobalHandles::Node* GlobalHandles::NodeBlock::pushNode(Node* node) {
  if (firstNode_ == nullptr) {
    firstNode_ = node;
  } else {
    // TODO
    LWNODE_DLOG_WARN("The weak callback is registered several times.");
    if (node) {
      delete node;
      return nullptr;
    }
  }

  return node;
}

void GlobalHandles::NodeBlock::registerWeakCallback() {
  MemoryUtil::gcRegisterFinalizer(
      value_,
      [](void* self, void* data) {
        LWNODE_CALL_TRACE_GC_START();
        LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Call weak callback: %p", self);

        auto globalHandles = static_cast<GlobalHandles*>(data);
        auto weakHandler = globalHandles->weakHandler_;
        auto block = weakHandler->popBlock(VAL(self));
        if (!block) {
          LWNODE_CALL_TRACE_ID(
              GLOBALHANDLES, "Cannot invoke callback: %p", self);
          return;
        }

        auto curNode = block->firstNode_;
        if (curNode) {
          if (curNode->callback()) {
            void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {nullptr,
                                                                       nullptr};
            v8::WeakCallbackInfo<void> info(block->isolate()->toV8(),
                                            curNode->parameter(),
                                            embedderFields,
                                            nullptr);
            LWNODE_CHECK_NOT_NULL(block->isolate());
            curNode->callback()(info);
          }
          // TODO: The weak callback is registered several times.(create
          // nextNode)
        }
        LWNODE_CALL_TRACE_GC_END();
      },
      isolate_->GetCurrent()->global_handles());
}

void GlobalHandles::NodeBlock::releaseValue() {
  registerWeakCallback();
  holder_.release();
}

GlobalHandles::GlobalHandles(IsolateWrap* isolate)
    : v8::internal::GlobalHandles(isolate), isolate_(isolate) {
  weakHandler_ = new GlobalWeakHandler();
}

void GlobalHandles::dispose() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES);
  persistentValues_.clear();
  weakHandler_->dispose();
  delete weakHandler_;
}

void GlobalHandles::create(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    persistentValues_.emplace(lwValue, 1);
  } else {
    ++iter->second;
    // TODO:
    LWNODE_CALL_TRACE_ID(GLOBALHANDLES,
                         "Persistent value was created multiple times: %p",
                         lwValue);
  }
}

bool GlobalHandles::destroy(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter != persistentValues_.end()) {
    if (iter->second == 1) {
      persistentValues_.erase(iter);
    } else {
      --iter->second;
    }
    return true;
  }
  return false;
}

size_t GlobalHandles::PostGarbageCollectionProcessing(
    /*const v8::GCCallbackFlags gc_callback_flags*/) {
#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  return weakHandler_->clearWeakValue();
#else
  return 0;
#endif
}

bool GlobalHandles::makeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    return false;
  }

  if (weakHandler_->isWeak(lwValue)) {
    return false;
  }

  auto block = std::make_unique<GlobalHandles::NodeBlock>(
      isolate_, lwValue, iter->second);
  block->pushNode(new Node(parameter, callback));
  weakHandler_->pushBlock(lwValue, std::move(block));

  LWNODE_CALL_TRACE_ID(
      GLOBALHANDLES, "MakeWeak: %p(%zu)", lwValue, iter->second);
  persistentValues_.erase(iter);
  return true;
}

bool GlobalHandles::clearWeakness(ValueWrap* lwValue) {
  auto block = weakHandler_->popBlock(lwValue);
  if (!block) {
    return false;
  }
  LWNODE_CHECK(persistentValues_.find(lwValue) == persistentValues_.end());
  persistentValues_.emplace(lwValue, block->usedNodes());
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "clearWeakness: %p", lwValue);
  return true;
}

size_t GlobalHandles::handles_count() {
  return persistentValues_.size();
}

}  // namespace EscargotShim
