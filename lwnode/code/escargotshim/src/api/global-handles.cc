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

  void dispose() { weakValues_.clear(); }

 private:
  std::unordered_map<ValueWrap*, std::unique_ptr<GlobalHandles::NodeBlock>>
      weakValues_;
};

GlobalWeakHandler g_globalWeakHandler;

GlobalHandles::Node::Node(void* parameter,
                          v8::WeakCallbackInfo<void>::Callback callback)
    : parameter_(parameter), callback_(callback) {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "New Node(%p)", this);
}

GlobalHandles::Node::~Node() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Free Node(%p)", this);
}

GlobalHandles::NodeBlock::NodeBlock(v8::Isolate* isolate, uint32_t count)
    : isolate_(isolate), usedNodes_(count) {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "New NodeBlock(%p)", this);
}

GlobalHandles::NodeBlock::~NodeBlock() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Free NodeBlock(%p)", this);
  auto curNode = firstNode_;
  while (curNode) {
    auto nextNode = curNode->nextNode();
    delete curNode;
    curNode = nextNode;
  }
}

bool GlobalHandles::NodeBlock::increaseUsage() {
  return usedNodes_++ == 0;
}

bool GlobalHandles::NodeBlock::decreaseUsage() {
  LWNODE_DCHECK(usedNodes_ > 0);
  return --usedNodes_ == 0;
}

GlobalHandles::Node* GlobalHandles::NodeBlock::pushNode(ValueWrap* lwValue,
                                                        Node* node) {
  if (firstNode_ == nullptr) {
    firstNode_ = node;
    registerWeakCallback(lwValue);
  } else {
    // TODO
    LWNODE_DLOG_WARN("The weak callback is registered several times.")
  }

  return node;
}

void GlobalHandles::NodeBlock::registerWeakCallback(ValueWrap* lwValue) {
  MemoryUtil::gcRegisterFinalizer(lwValue, [](void* self) {
    LWNODE_CALL_TRACE_GC_START();
    LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Call weak callback");

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
        LWNODE_CALL_TRACE_ID(
            GLOBALHANDLES, "Call v8 callback: parm(%p)", curNode->parameter());
        curNode->callback()(info);
      }

      if (curNode->nextNode()) {
        LWNODE_UNIMPLEMENT;  // TODO
      }
    }
    LWNODE_CALL_TRACE_GC_END();
  });
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
    LWNODE_DLOG_WARN("Persistent value was created multiple times.")
  }
}

void GlobalHandles::Destroy(ValueWrap* lwValue) {
  for (auto globalHandles : g_globalHandlesVector) {
    if (globalHandles->destroy(lwValue)) {
      return;
    }
  }
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot destroy: %p", lwValue)
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

void GlobalHandles::MakeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  for (auto globalHandles : g_globalHandlesVector) {
    if (globalHandles->makeWeak(lwValue, parameter, callback)) {
      return;
    }
  }
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "Cannot make weak value: %p", lwValue)
}

bool GlobalHandles::makeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    return false;
  }

  auto block =
      std::make_unique<GlobalHandles::NodeBlock>(isolate_, iter->second);
  block->pushNode(lwValue, new Node(parameter, callback));
  g_globalWeakHandler.pushBlock(lwValue, std::move(block));

  persistentValues_.erase(iter);
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "MakeWeak: %p", lwValue)
  return true;
}

size_t GlobalHandles::handles_count() {
  return persistentValues_.size();
}

}  // namespace EscargotShim
