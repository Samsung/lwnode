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

namespace EscargotShim {

std::vector<GlobalHandles*> g_globalHandlesVector;

class GlobalWeakHandler {
 public:
  void pushBlock(ValueWrap* lwValue,
                 std::unique_ptr<GlobalHandles::NodeBlock> block) {
    auto iter = weakValues_.find(lwValue);
    if (iter != weakValues_.end()) {
      // TODO
      LWNODE_CHECK_NOT_REACH_HERE();
    }
    weakValues_.emplace(lwValue, std::move(block));
  }

  std::unique_ptr<GlobalHandles::NodeBlock> popBlock(ValueWrap* lwValue) {
    auto iter = weakValues_.find(lwValue);
    if (iter == weakValues_.end()) {
      return nullptr;
    }
    auto nodeBlock = std::move(iter->second);
    weakValues_.erase(iter);

    return nodeBlock;
  }

  size_t clearWeakValue() {
    auto weakValuesSize = weakValues_.size();
    if (weakValuesSize == 0) {
      return 0;
    }
    LWNODE_CALL_TRACE_ID(
        GLOBALHANDLES, "Clear weak values: %zu", weakValuesSize);
    for (auto& iter : weakValues_) {
      iter.second->releaseValue();
    }
    return weakValuesSize;
  }

  bool isWeak(ValueWrap* lwValue) {
    return weakValues_.find(lwValue) != weakValues_.end();
  }

  void dispose() { weakValues_.clear(); }

 private:
  std::unordered_map<ValueWrap*, std::unique_ptr<GlobalHandles::NodeBlock>>
      weakValues_;
};

GlobalWeakHandler g_globalWeakHandler;

GlobalHandles::Node::Node(void* parameter,
                          v8::WeakCallbackInfo<void>::Callback callback)
    : parameter_(parameter), callback_(callback) {}

GlobalHandles::Node::~Node() {}

GlobalHandles::NodeBlock::NodeBlock(v8::Isolate* isolate,
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
  MemoryUtil::gcRegisterFinalizer(value_, [](void* self) {
    LWNODE_CALL_TRACE_GC_START();
    LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Call weak callback: %p", self);

    auto block = g_globalWeakHandler.popBlock(VAL(self));
    if (!block) {
      LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot invoke callback: %p", self);
      return;
    }

    auto curNode = block->firstNode_;
    if (curNode) {
      if (curNode->callback()) {
        void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {nullptr,
                                                                   nullptr};
        v8::WeakCallbackInfo<void> info(
            block->isolate(), curNode->parameter(), embedderFields, nullptr);
        LWNODE_CHECK_NOT_NULL(block->isolate());
        curNode->callback()(info);
      }
      // TODO: The weak callback is registered several times.(create nextNode)
    }
    LWNODE_CALL_TRACE_GC_END();
  });
}

void GlobalHandles::NodeBlock::releaseValue() {
  registerWeakCallback();
  holder_.release();
}

GlobalHandles::GlobalHandles(v8::Isolate* isolate) : isolate_(isolate) {
  g_globalHandlesVector.push_back(this);
}

void GlobalHandles::Dispose() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES);
  persistentValues_.clear();
  auto it = std::find(
      g_globalHandlesVector.begin(), g_globalHandlesVector.end(), this);

  if (it != g_globalHandlesVector.end()) {
    g_globalHandlesVector.erase(it);
  }
  // TODO: consider multi isolate
  if (g_globalHandlesVector.size() == 0) {
    g_globalWeakHandler.dispose();
  }
}

void GlobalHandles::Create(ValueWrap* lwValue) {
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

void GlobalHandles::Destroy(ValueWrap* lwValue) {
  for (auto globalHandles : g_globalHandlesVector) {
    if (globalHandles->destroy(lwValue)) {
      return;
    }
  }
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot destroy: %p", lwValue);
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
  return g_globalWeakHandler.clearWeakValue();
#else
  return 0;
#endif
}

void GlobalHandles::MakeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  for (auto globalHandles : g_globalHandlesVector) {
    if (globalHandles->makeWeak(lwValue, parameter, callback)) {
      return;
    }
  }
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot make weak value: %p", lwValue);
}

bool GlobalHandles::makeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    return false;
  }

  if (g_globalWeakHandler.isWeak(lwValue)) {
    return false;
  }

  auto block = std::make_unique<GlobalHandles::NodeBlock>(
      isolate_, lwValue, iter->second);
  block->pushNode(new Node(parameter, callback));
  g_globalWeakHandler.pushBlock(lwValue, std::move(block));

  LWNODE_CALL_TRACE_ID(
      GLOBALHANDLES, "MakeWeak: %p(%zu)", lwValue, iter->second);
  persistentValues_.erase(iter);
  return true;
}

void* GlobalHandles::ClearWeakness(ValueWrap* lwValue) {
  for (auto globalHandles : g_globalHandlesVector) {
    if (globalHandles->clearWeak(lwValue)) {
      return lwValue;
    }
  }
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot clear weak value: %p", lwValue);
  return nullptr;
}

bool GlobalHandles::clearWeak(ValueWrap* lwValue) {
  auto block = g_globalWeakHandler.popBlock(lwValue);
  if (!block) {
    return false;
  }
  LWNODE_CHECK(persistentValues_.find(lwValue) == persistentValues_.end());
  persistentValues_.emplace(lwValue, block->usedNodes());
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "ClearWeak: %p", lwValue);
  return true;
}

size_t GlobalHandles::handles_count() {
  return persistentValues_.size();
}

}  // namespace EscargotShim
