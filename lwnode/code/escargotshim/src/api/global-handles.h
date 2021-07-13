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

namespace EscargotShim {

class GlobalHandles final : public gc {
 public:
  GlobalHandles(v8::Isolate* isolate);

  void Create(ValueWrap* lwValue);
  static void Destroy(ValueWrap* lwValue);
  static void MakeWeak(ValueWrap* lwValue,
                       void* parameter,
                       v8::WeakCallbackInfo<void>::Callback callback);

  bool destroy(ValueWrap* lwValue);

  bool makeWeak(ValueWrap* lwValue,
                void* parameter,
                v8::WeakCallbackInfo<void>::Callback callback);

  size_t handles_count();

  void Dispose();

  class Node {
   public:
    Node(void* parameter, v8::WeakCallbackInfo<void>::Callback callback);
    ~Node();

    Node(const Node&) = delete;

    Node* nextNode() { return nextNode_; }
    void* parameter() { return parameter_; }
    v8::WeakCallbackInfo<void>::Callback callback() { return callback_; }

   private:
    Node* nextNode_{nullptr};
    void* parameter_{nullptr};
    v8::WeakCallbackInfo<void>::Callback callback_;
  };

  class NodeBlock {
   public:
    NodeBlock(v8::Isolate* isolate, uint32_t count);

    ~NodeBlock();

    NodeBlock(const NodeBlock&) = delete;

    bool increaseUsage();

    bool decreaseUsage();

    uint32_t usage() { return usedNodes_; }

    Node* firstNode() { return firstNode_; }
    void setFirstNode(Node* node) { firstNode_ = node; }

    Node* pushNode(ValueWrap* lwValue, Node* node);

    void registerWeakCallback(ValueWrap* lwValue);

    v8::Isolate* isolate() { return isolate_; }

   private:
    v8::Isolate* isolate_{nullptr};
    uint32_t usedNodes_{0};
    Node* firstNode_{nullptr};
  };

 private:
  GCUnorderedMap<ValueWrap*, size_t> persistentValues_;
  v8::Isolate* isolate_{nullptr};
};
}  // namespace EscargotShim
