/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
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
#include <EscargotPublic.h>
#include <string>
#include <thread>
#include "escargot-sample.h"
#include "include/v8.h"

using namespace v8;
using namespace Escargot;
using namespace EscargotShim;

int helloV8(int argc, char* argv[]) {
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
      return 1;
    }

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);

    // Run the script to get the result.
    Local<Value> result =
        script->BindToCurrentContext()->Run(context).ToLocalChecked();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(isolate, result);
    printf("%s\n", *utf8);
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  V8::Dispose();

  // V8::ShutdownPlatform();
  // delete platform;
  delete create_params.array_buffer_allocator;  // @check node do this?
  return 0;
}

int helloEscargot(int argc, char* argv[]) {
  auto platform = createPlatform();

  Globals::initialize(platform);

  // Create a new VMInstance and make a Context.
  PersistentRefHolder<VMInstanceRef> instance = VMInstanceRef::create();
  PersistentRefHolder<ContextRef> context = ContextRef::create(instance);

  // Create a string containing the JavaScript source code
  const char* rawsource = "'Hello' + ', Escargot!'";
  const char* rawfilename = "hello-world.js";
  StringRef* source = StringRef::createFromUTF8(rawsource, strlen(rawsource));
  StringRef* filename =
      StringRef::createFromUTF8(rawfilename, strlen(rawfilename));

  // Compile the source code
  auto scriptInitializeResult =
      context->scriptParser()->initializeScript(source, filename, false);

  // Run the script to get the result.
  auto evalResult = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      scriptInitializeResult.script.get());

  // Convert the result to an UTF8 string and print it.
  puts(evalResult.resultOrErrorToString(context)->toStdUTF8String().data());

  // Dispose the instance and tear down Escargot.
  context.release();
  instance.release();
  Globals::finalize();
  return 0;
}

Evaluator::EvaluatorResult executeSource(ContextRef* context, std::string src) {
  auto source = StringRef::createFromUTF8(src.c_str(), src.length());
  auto scriptInitializeResult = context->scriptParser()->initializeScript(
      source, StringRef::emptyString(), false);

  return Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      scriptInitializeResult.script.get());
}

int helloEscargotThread(int argc, char* argv[]) {
  Globals::initialize(createPlatform());

  auto instance = VMInstanceRef::create();
  auto context = ContextRef::create(instance);
  auto result = executeSource(context, std::string(R"(
        'Hello' + ', Escargot!';
      )"));
  puts(result.resultOrErrorToString(context)->toStdUTF8String().data());

  {
    std::thread worker([]() {
      Globals::initializeThread();

      auto instance = VMInstanceRef::create();
      auto context = ContextRef::create(instance);
      auto result = executeSource(context, std::string(R"(
        'Hello' + ', Thread!';
      )"));
      puts(result.resultOrErrorToString(context)->toStdUTF8String().data());

      Globals::finalizeThread();
    });
    worker.join();
  }

  Globals::finalize();

  return 0;
}

int helloEscargotSample(int argc, char* argv[]) {
  if (argc == 1) {
    const char* const source =
        R"(
          print('Hello' + ', Escargot!');
      )";

    std::vector<const char*> arguments(argv, argv + argc);
    arguments.push_back("-e");
    arguments.push_back(source);
    return entry((int)arguments.size(), (char**)arguments.data());
  }

  return entry(argc, argv);
}

int main(int argc, char* argv[]) {
  // helloV8(argc, argv);
  // helloEscargot(argc, argv);
  // helloEscargotSample(argc, argv);
  helloEscargotThread(argc, argv);

  return 0;
}
