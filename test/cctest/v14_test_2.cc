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

#include "cctest.h"
#include "v8.h"

using namespace v8;

TEST(NumberValue) {
  V8::Initialize();

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();

  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);

    Context::Scope context_scope(context);

    double pi_val = 3.1415926;
    Local<v8::Number> pi_obj = v8::Number::New(isolate, pi_val);
    CHECK(pi_obj->IsNumber());
    CHECK_EQ(pi_val, pi_obj->Value());

    int32_t int32_val = 128;
    Local<v8::Integer> int32_obj = v8::Int32::New(isolate, int32_val);
    CHECK(int32_obj->IsInt32());
    CHECK_EQ(int32_val, int32_obj->Value());

    uint32_t uint32_val = 256;
    Local<v8::Integer> uint32_obj = v8::Int32::New(isolate, uint32_val);
    CHECK(uint32_obj->IsUint32());
    CHECK_EQ(uint32_val, uint32_obj->Value());

  }

  isolate->Dispose();
  V8::Dispose();
  delete create_params.array_buffer_allocator;
}
