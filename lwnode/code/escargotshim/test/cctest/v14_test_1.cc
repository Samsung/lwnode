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
#include "v14_test.h"
#include "v8.h"

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

THREADED_TEST(ObjectPrototype) {
  SETUP();

  Context::Scope context_scope(context);

  Local<Object> object = Object::New(isolate);
  Local<Object> prototype = Object::New(isolate);

  object->SetPrototype(context, prototype).FromJust();
  CHECK(object->GetPrototype() == prototype );

  TEARDOWN();
}

TEST(StringNewFromUtf8Literal) {
  SETUP();

  v8::TryCatch try_catch(isolate);
  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               .IsEmpty(),
           false);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Utf8Length(isolate),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Utf8Length(isolate), 6);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Length(),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Length(), 2);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->ContainsOnlyOneByte(),
           true);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->ContainsOnlyOneByte(),
           false);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->IsOneByte(),
           true);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->IsOneByte(),
           false);

  TEARDOWN();
}
