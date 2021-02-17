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
  CHECK(object->GetPrototype() == prototype);

  TEARDOWN();
}

// Helper functions that compile and run the source.
static inline Local<v8::Value> CompileRun_(v8::Local<v8::String> source) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  Local<v8::Context> context = isolate->GetCurrentContext();

  Local<UnboundScript> script;
  ScriptCompiler::Source script_source(source);

  if (!ScriptCompiler::CompileUnboundScript(isolate, &script_source)
           .ToLocal(&script)) {
    CHECK(false);
  }

  return script->BindToCurrentContext()->Run(context).ToLocalChecked();
}

#define CHECK_NOT_CAUGHT(__local_context__, try_catch, __op__)                 \
  do {                                                                         \
    const char* op = (__op__);                                                 \
    v8::Local<v8::Context> context = (__local_context__);                      \
    if (try_catch.HasCaught()) {                                               \
      v8::String::Utf8Value error(                                             \
          isolate, try_catch.Exception()->ToString(context).ToLocalChecked()); \
      printf("FATAL: Unexpected exception thrown during %s:\n\t%s\n",          \
             op,                                                               \
             *error);                                                          \
    }                                                                          \
  } while (false)

TEST(ScriptCompiler_CompileFunctionInContext) {
  SETUP();

  Context::Scope context_scope(context);

  v8::ScriptOrigin origin(v8_str("test"), v8_int(17), v8_int(31));
  v8::ScriptCompiler::Source script_source(v8_str("return 0"), origin);

  v8::TryCatch try_catch(isolate);
  v8::MaybeLocal<v8::Function> maybe_fun =
      v8::ScriptCompiler::CompileFunctionInContext(
          context, &script_source, 0, nullptr, 0, nullptr);

  // CHECK_NOT_CAUGHT(
  //     context, try_catch, "v8::ScriptCompiler::CompileFunctionInContext");

  v8::Local<v8::Function> fun = maybe_fun.ToLocalChecked();
  CHECK(!fun.IsEmpty());
  // CHECK(!try_catch.HasCaught());

  v8::Local<v8::String> result = fun->ToString(context).ToLocalChecked();
  v8::Local<v8::String> expected = v8_str("function anonymous(\n"
                                          ") {\n"
                                          "return 0\n"
                                          "}");

  CHECK(expected->Equals(context, result).FromJust());

  TEARDOWN();
}
