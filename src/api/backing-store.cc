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
#include "extra-data.h"
#include "isolate.h"

using namespace Escargot;

namespace EscargotShim {

BackingStoreWrap::BackingStoreWrap(
    void* data,
    size_t byte_length,
    SharedFlag shared,
    std::unique_ptr<BufferDeleter> buffer_deleter)
    : data_(data),
      byteLength_(byte_length),
      isShared_(shared == SharedFlag::kShared),
      bufferDeleter_(std::move(buffer_deleter)) {}

BackingStoreWrap::~BackingStoreWrap() {
  bufferDeleter_->Free(data_, byteLength_);
}


std::unique_ptr<BackingStoreWrap> BackingStoreWrap::create(
    void* data,
    size_t byte_length,
    v8::BackingStore::DeleterCallback deleter,
    void* deleter_data) {
  return std::make_unique<BackingStoreWrap>(
      data,
      byte_length,
      SharedFlag::kNotShared,
      std::make_unique<ExternalBufferDeleter>(deleter, deleter_data));
}

}  // namespace EscargotShim
