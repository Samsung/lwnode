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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "escargot-sample.h"
#include "include/v8.h"
// internals
#include "api/flags.h"
#include "api/utils/gc.h"

using namespace v8;
using namespace Escargot;
using namespace EscargotShim;
using namespace EscargotSample;

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
  Globals::initialize();

  auto platform = new EscargotSample::Platform();

  // Create a new VMInstance and make a Context.
  PersistentRefHolder<VMInstanceRef> instance = VMInstanceRef::create(platform);
  PersistentRefHolder<ContextRef> context = ContextRef::create(instance);

  instance->setOnVMInstanceDelete(
      [](VMInstanceRef* instance) { delete instance->platform(); });

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

int helloEscargotSample(int argc, char* argv[]) {
  App app;

  const char* const source =
      R"(
        print('Hello' + ', Escargot!');
      )";
  app.evalScript(source, "", false, false);
  return 0;
}

int main(int argc, char* argv[]) {
  Flags::add(FlagType::TraceGC);

  helloV8(argc, argv);
  // helloEscargot(argc, argv);
  helloEscargotSample(argc, argv);

  return 0;
}
