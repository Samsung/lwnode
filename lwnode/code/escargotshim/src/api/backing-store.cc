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

BackingStoreWrap::BackingStoreWrap(void* buffer,
                                   size_t byte_length,
                                   SharedFlag shared)
    : buffer_(buffer),
      byteLength_(byte_length),
      isShared_(shared == SharedFlag::kShared) {
  LWNODE_DLOG_INFO("bs=%p mem=%p (length=%zuB)", this, buffer_, byteLength_);
}

BackingStoreWrap::~BackingStoreWrap() {
  clear();
}

void BackingStoreWrap::clear() {
  buffer_ = nullptr;
  byteLength_ = 0;
}

std::unique_ptr<BackingStoreWrap> BackingStoreWrap::create(
    IsolateWrap* isolate,
    size_t byteLength,
    SharedFlag shared,
    InitializedFlag initialized) {
  void* buffer = nullptr;

  v8::ArrayBuffer::Allocator* allocator = isolate->array_buffer_allocator();

  if (initialized == InitializedFlag::kUninitialized) {
    buffer = allocator->AllocateUninitialized(byteLength);
  } else if (initialized == InitializedFlag::kZeroInitialized) {
    buffer = allocator->Allocate(byteLength);
  } else {
    LWNODE_DCHECK(false);
  }

  return std::make_unique<BackingStoreWrap>(
      BackingStoreWrap(buffer, byteLength, shared));
}

bool BackingStoreWrap::attachTo(ExecutionStateRef* state,
                                ArrayBufferObjectRef* arrayBuffer) {
  // attach the buffer of this backing store to the given arrary buffer
  arrayBuffer->attachBuffer(state, buffer(), byteLength());

  // since the ownership of the backing store is given to this
  // arraybuffer, it should manage destructing the backing store.
  ObjectRefHelper::setExtraData(arrayBuffer, this, [](void* self) {
    auto value = reinterpret_cast<ValueRef*>(self);
    auto lw_backingStore =
        reinterpret_cast<BackingStoreWrap*>(value->asObject()->extraData());

    LWNODE_DCHECK_NOT_NULL(lw_backingStore);

    if (lw_backingStore->isShared()) {
      LWNODE_UNIMPLEMENT;
    } else {
      delete lw_backingStore;
    }
  });

  return true;
}

}  // namespace EscargotShim
