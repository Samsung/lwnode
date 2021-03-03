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

#include "backing-store.h"
#include "es-helper.h"
#include "isolate.h"

using namespace Escargot;

namespace EscargotShim {

BackingStoreWrap::BackingStoreWrap(void* data,
                                   size_t byte_length,
                                   SharedFlag shared,
                                   v8::ArrayBuffer::Allocator* allocator)
    : data_(data),
      byteLength_(byte_length),
      isShared_(shared == SharedFlag::kShared),
      allocator_(allocator) {
  LWNODE_DLOG_INFO("bs=%p mem=%p (length=%zuB)", this, data_, byteLength_);
}

BackingStoreWrap::~BackingStoreWrap() {
  allocator_->Free(data_, byteLength_);
  clear();
}

void BackingStoreWrap::clear() {
  data_ = nullptr;
  allocator_ = nullptr;
  byteLength_ = 0;
}

std::unique_ptr<BackingStoreWrap> BackingStoreWrap::create(
    IsolateWrap* isolate,
    size_t byteLength,
    SharedFlag shared,
    InitializedFlag initialized) {
  void* buffer = nullptr;

  auto allocator = isolate->array_buffer_allocator();

  if (initialized == InitializedFlag::kUninitialized) {
    buffer = allocator->AllocateUninitialized(byteLength);
  } else if (initialized == InitializedFlag::kZeroInitialized) {
    buffer = allocator->Allocate(byteLength);
  } else {
    LWNODE_DCHECK(false);
  }

  return std::make_unique<BackingStoreWrap>(
      buffer, byteLength, shared, allocator);
}

bool BackingStoreWrap::attachTo(ExecutionStateRef* state,
                                ArrayBufferObjectRef* arrayBuffer) {
  // attach the buffer of this backing store to the given arrary buffer
  arrayBuffer->attachBuffer(state, this->Data(), this->ByteLength());

  // since the ownership of the backing store is given to this
  // arraybuffer, it should manage destructing the backing store.
  auto holder = new BackingStoreWrapHolder(this);

  ObjectRefHelper::setExtraData(arrayBuffer, holder, [](void* self) {
    auto value = reinterpret_cast<ValueRef*>(self);
    auto holder = reinterpret_cast<BackingStoreWrapHolder*>(
        value->asObject()->extraData());

    LWNODE_DCHECK_NOT_NULL(holder);

    // the backing store the holder has will be deleted when no one uses it.
    delete holder;
  });

  return true;
}

// --- BackingStoreWrapHolder ---

std::map<BackingStoreWrap*, u_int8_t> BackingStoreWrapHolder::map_;

BackingStoreWrapHolder::BackingStoreWrapHolder(BackingStoreWrap* backingStore)
    : backingStore_(backingStore) {
  auto it = BackingStoreWrapHolder::map_.find(backingStore_);
  if (it == map_.end()) {
    map_.insert(std::make_pair(backingStore, 1));
  } else {
    it->second++;
  }
}

BackingStoreWrapHolder::~BackingStoreWrapHolder() {
  auto it = BackingStoreWrapHolder::map_.find(backingStore_);
  if (it != map_.end()) {
    if (it->second == 1) {
      map_.erase(it);
      delete backingStore_;
    } else {
      it->second--;
    }
  } else {
    LWNODE_DCHECK_MSG(false, "the target backing store is already destroyed");
  }
}

std::shared_ptr<BackingStoreWrapHolder> BackingStoreWrapHolder::clone() {
  return std::make_shared<BackingStoreWrapHolder>(backingStore_);
}

}  // namespace EscargotShim
