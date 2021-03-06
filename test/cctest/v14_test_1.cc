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
#include "internal-api.h"
#include "v8.h"

using namespace v8;

TEST(UnboundScript) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  // Create a string containing the JavaScript source code.
  Local<String> source =
      String::NewFromUtf8(isolate, "'Hello' + ', V8!'", NewStringType::kNormal)
          .ToLocalChecked();

  // Create a new context.
  Local<Context> context = env.local();

  // Create a Source
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

TEST(StringCreateNewFromOneTwoByte) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

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
}

THREADED_TEST(ObjectPrototype) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Context::Scope context_scope(env.local());

  Local<Object> object = Object::New(isolate);
  Local<Object> prototype = Object::New(isolate);

  object->SetPrototype(env.local(), prototype).FromJust();
  CHECK(object->GetPrototype() == prototype);
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
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();

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
}

TEST(ContextEquals) {
  LocalContext env;
  Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  Local<v8::Context> context = isolate->GetCurrentContext();

  CHECK(env.local() == context);
}

// --- ArrayBuffer ---
TEST(ArrayBuffer_New) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, 1024);
  // CheckInternalFieldsAreZero(ab);
  CHECK_EQ(1024, (int)ab->ByteLength());
  CcTest::CollectAllGarbage();

  std::shared_ptr<v8::BackingStore> backing_store = ab->GetBackingStore();
  CHECK_EQ(1024, (int)backing_store->ByteLength());

  uint8_t* data = static_cast<uint8_t*>(backing_store->Data());
  CHECK_NOT_NULL(data);
  CHECK(env->Global()->Set(env.local(), v8_str("ab"), ab).FromJust());

  v8::Local<v8::Value> result = CompileRun("ab.byteLength");
  CHECK_EQ(1024, (int)result->Int32Value(env.local()).FromJust());

  result = CompileRun("var u8 = new Uint8Array(ab);"
                      "u8[0] = 0xFF;"
                      "u8[1] = 0xAA;"
                      "u8.length");
  CHECK_EQ(1024, result->Int32Value(env.local()).FromJust());
  CHECK_EQ(0xFF, data[0]);
  CHECK_EQ(0xAA, data[1]);
  data[0] = 0xCC;
  data[1] = 0x11;
  result = CompileRun("u8[0] + u8[1]");
  CHECK_EQ(0xDD, result->Int32Value(env.local()).FromJust());
}

static int s_array_buffer_release_pass;

TEST(ArrayBuffer_Release) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  [&isolate]() {
    v8::HandleScope handle_scope(isolate);
    Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, 1024);
    s_array_buffer_release_pass = 0;
    MemoryUtil::gcRegisterFinalizer(
        VAL(*ab)->value(), [](void* self) { s_array_buffer_release_pass++; });
  }();

  CcTest::PreciseCollectAllGarbage();

  CHECK_GT(s_array_buffer_release_pass, 0);

  // todo: verify if releasing context scopes works using context scope
}

TEST(ArrayBuffer_BackingStoreReused) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  Local<v8::ArrayBuffer> ab1 = v8::ArrayBuffer::New(isolate, 1024);
  std::shared_ptr<v8::BackingStore> bs1 = ab1->GetBackingStore();
  CHECK_EQ(1024, (int)bs1->ByteLength());

  CHECK_EQ(1024, (int)ab1->ByteLength());
  ab1->Detach();
  CHECK_EQ(0, (int)ab1->ByteLength());

  Local<v8::ArrayBuffer> ab2 = v8::ArrayBuffer::New(isolate, bs1);
  std::shared_ptr<v8::BackingStore> bs2 = ab2->GetBackingStore();
  CHECK_EQ(1024, (int)bs2->ByteLength());
  CHECK_EQ(bs1->Data(), bs2->Data());
}

static std::string print_result;

TEST(AddPrintFunctionUsingFunctionTemplate) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  static v8::FunctionCallback callback =
      [](const FunctionCallbackInfo<Value>& args) -> void {
    LWNODE_CHECK(args.Length() == 1 && args[0]->IsString());
    String::Utf8Value utf8(args.GetIsolate(), args[0]);
    printf("%s\n", *utf8);
    print_result = *utf8;
  };

  v8::Local<v8::Function> function =
      v8::FunctionTemplate::New(isolate, callback)
          ->GetFunction(context)
          .ToLocalChecked();

  v8::Local<v8::Object> target = context->Global();
  target->Set(context, v8_str("print"), function).Check();

  CompileRun("print('hello')");
  CHECK_EQ(print_result.compare("hello"), 0);
}

THREADED_TEST(IsExternal) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  int x = 3;
  Local<v8::External> ext = v8::External::New(isolate, &x);
  CHECK(ext->IsExternal());

  CHECK(env->Global()->Set(env.local(), v8_str("ext"), ext).FromJust());
  Local<Value> reext_obj = CompileRun("this.ext");
  v8::Local<v8::External> reext = reext_obj.As<v8::External>();
  int* ptr = static_cast<int*>(reext->Value());
  CHECK_EQ(3, x);
  *ptr = 10;
  CHECK_EQ(x, 10);
}

void TestArrayBufferViewContents(LocalContext* env) {
  v8::Local<v8::Object> obj_a = v8::Local<v8::Object>::Cast(
      (*env)
          ->Global()
          ->Get((*env)->GetIsolate()->GetCurrentContext(), v8_str("a"))
          .ToLocalChecked());
  CHECK(obj_a->IsArrayBufferView());

  v8::Local<v8::ArrayBufferView> array_buffer_view =
      v8::Local<v8::ArrayBufferView>::Cast(obj_a);

  /*
    @related:
      CHECK_EQ(array_buffer_view->HasBuffer(), should_use_buffer);

    @note
      v8 seemingly uses lazy-allocattion or optimized-allocation
      for ArrayBufferView's backing ArrayBuffer.

      Its HasBuffer() returns false if ArrayBufferObject, which
      should be returned when ArrayBufferView::Buffer() is invoked,
      isn't accessed before.

      However, Escargot allocates ArrayBufferObject when
      ArrayBufferView is created.
  */

  unsigned char contents[4] = {23, 23, 23, 23};
  CHECK_EQ(sizeof(contents),
           array_buffer_view->CopyContents(contents, sizeof(contents)));

  for (size_t i = 0; i < sizeof(contents); ++i) {
    CHECK_EQ(i, contents[i]);
  }
}

TEST(CopyContentsTypedArray) {
  LocalContext env;
  v8::HandleScope scope(env->GetIsolate());
  CompileRun("var a = new Uint8Array(4);"
             "a[0] = 0;"
             "a[1] = 1;"
             "a[2] = 2;"
             "a[3] = 3;");
  TestArrayBufferViewContents(&env);
}

TEST(CopyContentsArray) {
  LocalContext env;
  v8::HandleScope scope(env->GetIsolate());
  CompileRun("var a = new Uint8Array([0, 1, 2, 3]);");
  TestArrayBufferViewContents(&env);
}

TEST(CopyContentsView) {
  LocalContext env;
  v8::HandleScope scope(env->GetIsolate());
  CompileRun("var b = new ArrayBuffer(6);"
             "var c = new Uint8Array(b);"
             "c[0] = -1;"
             "c[1] = -1;"
             "c[2] = 0;"
             "c[3] = 1;"
             "c[4] = 2;"
             "c[5] = 3;"
             "var a = new DataView(b, 2);");
  TestArrayBufferViewContents(&env);
}

static void AddPrintFunction(Isolate* isolate, v8::Local<v8::Context> context) {
  v8::Local<v8::Function> function =
      v8::FunctionTemplate::New(
          isolate,
          [](const FunctionCallbackInfo<Value>& args) -> void {
            LWNODE_CHECK(args.Length() == 1);
            String::Utf8Value utf8(args.GetIsolate(), args[0]);
            printf("%s\n", *utf8);
          })
          ->GetFunction(context)
          .ToLocalChecked();

  context->Global()->Set(context, v8_str("print"), function).Check();
}

TEST(MapUsingIntegerKey) {
  LocalContext env;
  Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  Local<v8::Context> context = isolate->GetCurrentContext();

  AddPrintFunction(isolate, context);

  Local<Map> err_map = Map::New(isolate);

  err_map->Set(context, Integer::New(isolate, 1), v8_str(isolate, "positive"))
      .ToLocalChecked();
  err_map->Set(context, Integer::New(isolate, -1), v8_str(isolate, "negative"))
      .ToLocalChecked();

  v8::Local<v8::Object> global = context->Global();

  global->Set(context, v8_str("err_map"), err_map).Check();

  const char* source =
      R"(
        function assert(condition, message) {
          if (!condition) throw new Error(message || 'Assertion failed');
        }

        for (const element of err_map) {
          print(element);
        }

        assert(err_map.get(1) == 'positive');
        assert(err_map.get(-1) == 'negative');
      )";

  v8::TryCatch try_catch(isolate);
  CompileRun(source);

  CHECK(try_catch.HasCaught() == false);
}

static bool isWeakApiCallbackCalled = false;
static void WeakApiCallback(
    const v8::WeakCallbackInfo<Persistent<v8::Object>>& data) {
  data.GetParameter()->Reset();
  delete data.GetParameter();
  isWeakApiCallbackCalled = true;
}

TEST(DISABLED_WeakCallback_Invoked_by_CollectGarbage) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  {
    v8::HandleScope scope(isolate);

    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    CHECK(
        obj->Set(context.local(), v8_str("key"), v8::Integer::New(isolate, 231))
            .FromJust());

    v8::Persistent<v8::Object>* handle =
        new v8::Persistent<v8::Object>(isolate, obj);

    handle->SetWeak<v8::Persistent<v8::Object>>(
        handle, WeakApiCallback, v8::WeakCallbackType::kParameter);
  }

  CcTest::CollectGarbage();

  CHECK_EQ(isWeakApiCallbackCalled, true);
}

static void EnvGetter(Local<Name> name,
                      const PropertyCallbackInfo<Value>& info) {
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

  if (name->Equals(context, v8_str("p1")).FromJust()) {
    info.GetReturnValue().Set(v8_num(1));
  } else if (name->Equals(context, v8_str("p2")).FromJust()) {
    info.GetReturnValue().Set(v8_num(2));
  } else if (name->Equals(context, v8_str("p3")).FromJust()) {
    info.GetReturnValue().Set(v8_num(3));
  }
}

static void EnvQuery(Local<Name> name,
                     const PropertyCallbackInfo<Integer>& info) {
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  if (name->Equals(context, v8_str("p1")).FromJust()) {
    info.GetReturnValue().Set(1);
  } else if (name->Equals(context, v8_str("p2")).FromJust()) {
    info.GetReturnValue().Set(2);
  }
}

static void EnvEnumerator(const v8::PropertyCallbackInfo<v8::Array>& info) {
  v8::Local<v8::Array> result = v8::Array::New(info.GetIsolate());
  result->Set(info.GetIsolate()->GetCurrentContext(), 0, v8_str("p1"))
      .FromJust();
  result->Set(info.GetIsolate()->GetCurrentContext(), 1, v8_str("p2"))
      .FromJust();
  result->Set(info.GetIsolate()->GetCurrentContext(), 2, v8_str("p3"))
      .FromJust();
  info.GetReturnValue().Set(result);
}

TEST(EnvQueryHandler) {
  LocalContext env;
  Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  Local<v8::Context> context = isolate->GetCurrentContext();
  Local<ObjectTemplate> object_template = ObjectTemplate::New(isolate);
  object_template->SetHandler(NamedPropertyHandlerConfiguration(
      EnvGetter,
      nullptr,
      EnvQuery,
      nullptr,
      EnvEnumerator,
      Local<Value>(),
      PropertyHandlerFlags::kHasNoSideEffect));

  CHECK(context->Global()
            ->Set(context,
                  v8_str("env"),
                  object_template->NewInstance(context).ToLocalChecked())
            .FromJust());

  const char* source =
      R"(
        function assert(condition, message) {
          if (!condition) throw new Error(message || 'Assertion failed');
        }
        function getKeys(value, showHidden) {
          // from lib/internal/util/inspect.js
          let keys; const symbols = Object.getOwnPropertySymbols(value);
          if (showHidden) { keys = Object.getOwnPropertyNames(value);
            if (symbols.length !== 0) keys.push(...symbols);
          } else { keys = Object.keys(value);
            if (symbols.length !== 0) {
              const filter = (key) => Object.prototype.propertyIsEnumerable(value, key);
              keys.push(...symbols.filter(filter));
            }
          }
          return keys;
        }
        assert(env.p1 == 1); assert(env.p2 == 2); assert(env.p3 == 3);
        assert(getKeys(env,false).length == 2);
        assert(getKeys(env,true).length == 3);
      )";

  v8::TryCatch try_catch(isolate);
  CompileRun(source);
  CHECK(try_catch.HasCaught() == false);
}
