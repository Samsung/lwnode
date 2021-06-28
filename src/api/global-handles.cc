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

#include "api/isolate.h"
#include "base.h"
#include "global-handles.h"

namespace EscargotShim {

static std::unordered_map<ValueWrap*, std::unique_ptr<GlobalHandles::NodeBlock>>
    g_WeakValues;

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

    auto iter = g_WeakValues.find(VAL(self));
    if (iter == g_WeakValues.end()) {
      LWNODE_LOG_ERROR("Cannot find weakened value.");
    }

    auto curNode = iter->second->firstNode();
    if (!curNode) {
      LWNODE_LOG_ERROR();
      return;
    }

    void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {nullptr,
                                                               nullptr};
    v8::WeakCallbackInfo<void> info(
        iter->second->isolate(), curNode->parameter(), embedderFields, nullptr);
    curNode->callback()(info);

    if (curNode->nextNode()) {
      LWNODE_UNIMPLEMENT;  // TODO
    }

    g_WeakValues.erase(iter);

    LWNODE_CALL_TRACE_GC_END();
  });
}

void GlobalHandles::Dispose() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES);
  persistentValues_.clear();

  // TODO: need to free the remaining values in the weak map, not clear map.
  g_WeakValues.clear();
}

void GlobalHandles::Create(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    persistentValues_.emplace(lwValue,
                              std::make_unique<NodeBlock>(isolate_, 1));
  } else {
    iter->second->increaseUsage();
    // TODO:
    LWNODE_DLOG_WARN("Persistent value was created multiple times.")
  }
}

void GlobalHandles::Destroy(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter != persistentValues_.end()) {
    if (iter->second->usage() == 1) {
      persistentValues_.erase(iter);
    } else {
      iter->second->decreaseUsage();
    }
  } else {
    LWNODE_CALL_TRACE_ID(
        GLOBALHANDLES, "The value(%p) has already been removed.", lwValue)
  }
}

bool GlobalHandles::MakeWeak(ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    LWNODE_LOG_ERROR("Only Persistent value can be made weak.");
    return false;
  }

  iter->second->pushNode(lwValue, new Node(parameter, callback));
  g_WeakValues.emplace(lwValue, std::move(iter->second));
  persistentValues_.erase(iter);
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES, "The value(%p) was weakened.", lwValue)
  return true;
}

size_t GlobalHandles::handles_count() {
  return persistentValues_.size();
}

}  // namespace EscargotShim
