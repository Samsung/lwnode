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
#include "handle.h"
#include "utils/gc.h"

namespace v8 {
namespace internal {
class GlobalHandles : public gc {
 public:
  GlobalHandles(Isolate* isolate) : isolate_(isolate) {}

  static void MakeWeak(EscargotShim::ValueWrap* lwValue,
                       void* parameter,
                       v8::WeakCallbackInfo<void>::Callback callback);

  // void Create(EscargotShim::ValueWrap* lwValue);
  static void Destroy(EscargotShim::ValueWrap* lwValue);
  static void* ClearWeakness(EscargotShim::ValueWrap* lwValue);

  virtual size_t handles_count() = 0;

 private:
  Isolate* const isolate_ = nullptr;
};

}  // namespace internal
}  // namespace v8

namespace EscargotShim {

class GlobalWeakHandler;

class GlobalHandles final : public v8::internal::GlobalHandles {
 public:
  GlobalHandles(IsolateWrap* isolate);

  void create(ValueWrap* lwValue);

  size_t PostGarbageCollectionProcessing(
      /*const v8::GCCallbackFlags gc_callback_flags*/);

  bool makeWeak(ValueWrap* lwValue,
                void* parameter,
                v8::WeakCallbackInfo<void>::Callback callback);
  bool clearWeakness(ValueWrap* lwValue);
  bool destroy(ValueWrap* lwValue);

  size_t handles_count() override;

  void Dispose();

  class Node {
   public:
    Node(void* parameter, v8::WeakCallbackInfo<void>::Callback callback);
    ~Node();

    Node(const Node&) = delete;

    void* parameter() { return parameter_; }
    v8::WeakCallbackInfo<void>::Callback callback() { return callback_; }

   private:
    void* parameter_{nullptr};
    v8::WeakCallbackInfo<void>::Callback callback_;
  };

  class NodeBlock {
   public:
    NodeBlock(IsolateWrap* isolate, ValueWrap* value, uint32_t count);

    ~NodeBlock();

    NodeBlock(const NodeBlock&) = delete;

    uint32_t usedNodes() { return usedNodes_; }

    Node* firstNode() { return firstNode_; }
    void setFirstNode(Node* node) { firstNode_ = node; }

    Node* pushNode(Node* node);

    void registerWeakCallback();

    void releaseValue();

    IsolateWrap* isolate() { return isolate_; }

   private:
    IsolateWrap* isolate_{nullptr};
    ValueWrap* value_{nullptr};
    uint32_t usedNodes_{0};
    Node* firstNode_{nullptr};
    Escargot::PersistentRefHolder<ValueWrap> holder_;
  };

 private:
  GCUnorderedMap<ValueWrap*, size_t> persistentValues_;
  IsolateWrap* isolate_{nullptr};
  GlobalWeakHandler* weakHandler_ = nullptr;
};

class GlobalWeakHandler {
 public:
  void pushBlock(ValueWrap* lwValue,
                 std::unique_ptr<GlobalHandles::NodeBlock> block) {
    if (isWeak(lwValue)) {
      // TODO
      LWNODE_CHECK_NOT_REACH_HERE();
    }

    weakValues_.emplace(lwValue, std::move(block));
  }

  std::unique_ptr<GlobalHandles::NodeBlock> popBlock(ValueWrap* lwValue) {
    auto iter = weakValues_.find(lwValue);
    if (!isWeak(lwValue)) {
      return nullptr;
    }

    auto nodeBlock = std::move(iter->second);
    weakValues_.erase(iter);

    return nodeBlock;
  }

  size_t clearWeakValue() {
    if (weakValues_.empty()) {
      return 0;
    }

    LWNODE_CALL_TRACE_ID(
        GLOBALHANDLES, "Clear weak values: %zu", weakValues_.size());
    for (auto& iter : weakValues_) {
      iter.second->releaseValue();
    }

    return weakValues_.size();
  }

  bool isWeak(ValueWrap* lwValue) {
    return weakValues_.find(lwValue) != weakValues_.end();
  }

  void dispose() { weakValues_.clear(); }

 private:
  std::unordered_map<ValueWrap*, std::unique_ptr<GlobalHandles::NodeBlock>>
      weakValues_;
};

}  // namespace EscargotShim
