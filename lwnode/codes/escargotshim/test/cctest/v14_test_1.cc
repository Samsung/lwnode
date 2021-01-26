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
#include "v14_test.h"

using namespace v8;

TEST(UnboundScript) {
  // Initialize V8.
  /* V8::InitializeICUDefaultLocation(argv[0]);
    V8::InitializeExternalStartupData(argv[0]); */
  // v8::Platform* platform = platform::CreateDefaultPlatform();
  // V8::InitializePlatform(platform);
  V8::Initialize();

  // Create a new Isolate and make it the current one.
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  // @check use create_params.array_buffer_allocator_shared

  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    // // Create a string containing the JavaScript source code.
    Local<String> source =
        String::NewFromUtf8(
            isolate, "'Hello' + ', V8!'", NewStringType::kNormal)
            .ToLocalChecked();

    // Create a new context.
    Local<Context> context = Context::New(isolate);

    // // Create a Source
    ScriptCompiler::Source script_source(source);

    Local<UnboundScript> script;
    if (!ScriptCompiler::CompileUnboundScript(isolate, &script_source)
             .ToLocal(&script)) {
      CHECK(false);
    }

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);

    // Run the script to get the result.
    Local<Value> result =
        script->BindToCurrentContext()->Run(context).ToLocalChecked();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(isolate, result);
    CHECK_EQ(std::string("Hello, V8!").compare(std::string(*utf8)), 0);
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  V8::Dispose();

  // V8::ShutdownPlatform();
  // delete platform;
  delete create_params.array_buffer_allocator;  // @check node do this?
}

TEST(StringCreateNewFromOneTwoByte) {
  SETUP()

  const int length = 4;
  const int buffer_size = length * sizeof(uint16_t);

  void* buffer = malloc(buffer_size);
  if (buffer == NULL) return;
  memset(buffer, 'A', buffer_size);

  Local<String> empty = String::Empty(isolate);
  {
    v8::TryCatch try_catch(isolate);
    uint8_t* data = reinterpret_cast<uint8_t*>(buffer);

    CHECK_EQ(v8::String::NewFromOneByte(
                 isolate, data, v8::NewStringType::kNormal, length)
                 .IsEmpty(),
             false);
  }
  {
    v8::TryCatch try_catch(isolate);
    uint16_t* data = reinterpret_cast<uint16_t*>(buffer);
    CHECK_EQ(v8::String::NewFromTwoByte(
                 isolate, data, v8::NewStringType::kNormal, length)
                 .IsEmpty(),
             false);
  }
  free(buffer);

  TEARDOWN()
}
