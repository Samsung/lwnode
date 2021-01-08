/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

#include "isolate.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

Isolate::Isolate() {}
Isolate::~Isolate() {}

Isolate* Isolate::New() {
  Isolate* isolate = new Isolate();
  return isolate;
}

void Isolate::Initialize(const v8::Isolate::CreateParams& params) {
  Isolate* isolate = this;

  LWNODE_CHECK(params.code_event_handler == nullptr ||
               params.snapshot_blob == nullptr ||
               params.counter_lookup_callback == nullptr ||
               params.create_histogram_callback == nullptr ||
               params.add_histogram_sample_callback == nullptr ||
               params.external_references == nullptr ||
               params.allow_atomics_wait == true ||
               params.only_terminate_in_safe_scope == false ||
               params.embedder_wrapper_type_index == -1 ||
               params.embedder_wrapper_object_index == -1);

  if (auto allocator = params.array_buffer_allocator_shared) {
    LWNODE_CHECK(params.array_buffer_allocator == nullptr ||
                 params.array_buffer_allocator == allocator.get());
    isolate->set_array_buffer_allocator(allocator.get());
    isolate->set_array_buffer_allocator_shared(std::move(allocator));
  } else {
    LWNODE_CHECK_NOT_NULL(params.array_buffer_allocator);
    isolate->set_array_buffer_allocator(params.array_buffer_allocator);
  }
}

}  // namespace EscargotShim
