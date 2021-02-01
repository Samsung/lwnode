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

#define SETUP()                                                                \
  V8::Initialize();                                                            \
  Isolate::CreateParams create_params;                                         \
  create_params.array_buffer_allocator =                                       \
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();                       \
  Isolate* isolate = Isolate::New(create_params);                              \
  {                                                                            \
    Isolate::Scope isolate_scope(isolate);                                     \
    HandleScope handle_scope(isolate);                                         \
    Local<Context> context = Context::New(isolate);

#define TEARDOWN()                                                             \
  }                                                                            \
  isolate->Dispose();                                                          \
  V8::Dispose();                                                               \
  delete create_params.array_buffer_allocator;
