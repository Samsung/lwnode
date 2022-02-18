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

#include <codecvt>
#include <locale>
#include <string>

#include "cctest.h"
#include "v8.h"

using namespace v8;

TEST(NumberValue) {
  LocalContext env;
  v8::Isolate* isolate = CcTest::isolate();
  v8::HandleScope scope(isolate);

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

bool functionTemplateCallbackFlag = false;
static void functionTemplateCallback(
    const v8::FunctionCallbackInfo<Value>& info) {
  functionTemplateCallbackFlag = true;
  CHECK_EQ(info.IsConstructCall(), false);
  info.GetReturnValue().Set(v8_num(22));
  CHECK_EQ(3, info.Length());
  CHECK_EQ(12,
           info[0]
               ->ToInt32(info.GetIsolate()->GetCurrentContext())
               .ToLocalChecked()
               ->Value());
  CHECK_EQ(24,
           info[1]
               ->ToInt32(info.GetIsolate()->GetCurrentContext())
               .ToLocalChecked()
               ->Value());
  CHECK_EQ(36,
           info[2]
               ->ToInt32(info.GetIsolate()->GetCurrentContext())
               .ToLocalChecked()
               ->Value());
}

TEST(CallFunctionTemplate) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> fun_templ =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback);
  Local<Function> fun = fun_templ->GetFunction(env.local()).ToLocalChecked();

  CHECK(env->Global()->Set(env.local(), v8_str("obj"), fun).FromJust());
  CompileRun("var funtionTemplateTest1 = obj(12, 24, 36);");
  CHECK(functionTemplateCallbackFlag);
  ExpectInt32("funtionTemplateTest1", 22);
  int32_t result = env->Global()
                       ->Get(env.local(), v8_str("funtionTemplateTest1"))
                       .ToLocalChecked()
                       ->Int32Value(env.local())
                       .FromJust();
  CHECK_EQ(result, 22);
}

bool functionTemplateConsCallbackFlag = false;
static void functionTemplateConsCallback(
    const v8::FunctionCallbackInfo<Value>& info) {
  functionTemplateConsCallbackFlag = true;

  auto context = info.GetIsolate()->GetCurrentContext();

  CHECK_EQ(info.Length(), 1);
  CHECK_EQ(25, info[0]->ToInt32(context).ToLocalChecked()->Value());
  CHECK_EQ(info.IsConstructCall(), true);
  CHECK(info.Data()->Equals(context, v8_num(7)).FromJust());
  Local<Object>::Cast(info.This())
      ->Set(context, v8_str("x"), v8_num(22))
      .FromJust();
}

TEST(CallFunctionTemplateCons) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> fun_templ = v8::FunctionTemplate::New(
      isolate, functionTemplateConsCallback, v8::Integer::New(isolate, 7));

  Local<Function> fun = fun_templ->GetFunction(env.local()).ToLocalChecked();
  CHECK(env->Global()->Set(env.local(), v8_str("ClassA"), fun).FromJust());
  CompileRun("var aObject = new ClassA(25);");
  Local<Value> v = CompileRun("aObject.x");
  CHECK(v->IsNumber());
  CHECK_EQ(22, static_cast<int>(v->NumberValue(env.local()).FromJust()));
  CHECK(functionTemplateConsCallbackFlag);
}

TEST(ObjectTemplateSimple) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::ObjectTemplate> object_template =
      v8::ObjectTemplate::New(isolate);

  Local<Data> value =
      String::NewFromUtf8(isolate, "Hello", NewStringType::kNormal)
          .ToLocalChecked();
  object_template->Set(isolate, "message", value);

  // create an instance
  v8::Local<v8::Object> object =
      object_template->NewInstance(env.local()).ToLocalChecked();

  CHECK((*env)
            ->Global()
            ->Set(env.local(), v8_str("object_from_template"), object)
            .FromJust());

  ExpectString("object_from_template.message", "Hello");
}

static bool onread_flag = false;
static void GetOnread(Local<Name> name,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CHECK_EQ(info.This()->InternalFieldCount(), 2);
  onread_flag = !onread_flag;
}

THREADED_TEST(MultiSetInternalFields) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  Local<v8::ObjectTemplate> instance_templ = templ->InstanceTemplate();
  instance_templ->SetInternalFieldCount(3);
  Local<v8::ObjectTemplate> proto_templ = templ->PrototypeTemplate();
  proto_templ->SetAccessor(v8_str("onread"), GetOnread);
  instance_templ->SetInternalFieldCount(2);

  Local<v8::Function> fn = templ->GetFunction(env.local()).ToLocalChecked();
  CHECK((*env)->Global()->Set(env.local(), v8_str("ClassA"), fn).FromJust());
  CompileRun("var a = new ClassA();");
  CompileRun("a.onread");
  CHECK(onread_flag);

  instance_templ->SetInternalFieldCount(5);
  Local<v8::Function> fn2 = templ->GetFunction(env.local()).ToLocalChecked();
  CHECK((*env)->Global()->Set(env.local(), v8_str("ClassB"), fn2).FromJust());
  CompileRun("var b = new ClassB();");
  CompileRun("b.onread");
  CHECK(!onread_flag);
}

static bool calledInternalFieldsErrorCallback = false;

void InternalFieldsErrorCallback(const char* location, const char* message) {
  calledInternalFieldsErrorCallback = true;
}

THREADED_TEST(InternalFieldsErrorOnTemplate) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  isolate->SetFatalErrorHandler(InternalFieldsErrorCallback);
  v8::HandleScope scope(isolate);

  calledInternalFieldsErrorCallback = false;
  Local<v8::ObjectTemplate> otpl = ObjectTemplate::New(isolate);
  otpl->SetInternalFieldCount(-1);  // error!
  CHECK(calledInternalFieldsErrorCallback);
}

THREADED_TEST(InternalFieldsErrorOnObject) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  isolate->SetFatalErrorHandler(InternalFieldsErrorCallback);
  v8::HandleScope scope(isolate);

  calledInternalFieldsErrorCallback = false;
  Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  Local<v8::ObjectTemplate> instance_templ = templ->InstanceTemplate();
  instance_templ->SetInternalFieldCount(1);
  Local<v8::Object> object = templ->GetFunction(env.local())
                                 .ToLocalChecked()
                                 ->NewInstance(env.local())
                                 .ToLocalChecked();
  object->SetInternalField(5, v8_num(1));  // error!
  CHECK(calledInternalFieldsErrorCallback);
}

THREADED_TEST(Shebang) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::ScriptCompiler::Source script_source(v8_str("#!/usr/bin/node"));
  v8::TryCatch try_catch(CcTest::isolate());
  v8::Local<v8::Function> fun =
      v8::ScriptCompiler::CompileFunctionInContext(
          env.local(), &script_source, 0, nullptr, 0, nullptr)
          .ToLocalChecked();
  CHECK(!fun.IsEmpty());
  CHECK(!try_catch.HasCaught());
}

static void TryCatchOnManyCalldepsCallback(
    const v8::FunctionCallbackInfo<Value>& info) {
  CHECK(info[0]->IsFunction());
  info[0].As<Function>()->Call(
      info.GetIsolate()->GetCurrentContext(), info.This(), 0, nullptr);
}

TEST(TryCatchOnManyCalldeps) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> fun_templ =
      v8::FunctionTemplate::New(isolate, TryCatchOnManyCalldepsCallback);
  Local<Function> fun = fun_templ->GetFunction(env.local()).ToLocalChecked();

  CHECK(env->Global()->Set(env.local(), v8_str("parse"), fun).FromJust());

  const char* source =
      R"(
        var result = false;
        function OnError(condition, message) {
          throw new Error();
        }

        try {
          parse(OnError);
        } catch {
          result = true;
        }
        result;
      )";

  v8::Local<v8::Value> result = CompileRun(source);
  CHECK(result->BooleanValue(isolate));

  const char* sourceNotTry =
      R"(
        function OnError(condition, message) {
          throw new Error();
        }

        parse(OnError);
      )";

  v8::TryCatch try_catch(isolate);
  result = CompileRun(sourceNotTry);
  CHECK(try_catch.HasCaught());
}

static void TryCatchOnFunctionCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetIsolate()->ThrowException(v8_str("error"));
}

TEST(TryCatchOnFunction) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::Function> function =
      Function::New(env.local(), TryCatchOnFunctionCallback).ToLocalChecked();
  env.local()
      ->Global()
      ->Set(env.local(), v8_str("tryCatchOnFunction"), function)
      .FromJust();

  const char* source =
      R"(
        var result = false;
        try {
          tryCatchOnFunction();
        } catch {
          result = true;
        }
        result;
      )";

  v8::Local<v8::Value> result = CompileRun(source);
  CHECK(result->BooleanValue(isolate));
}

namespace {

bool calledHandlerCallback_ = false;
void initCalledHandlerCallback() {
  calledHandlerCallback_ = false;
}
void calledHandlerCallback() {
  calledHandlerCallback_ = true;
}
bool checkCalledHandlerCallback() {
  bool result = calledHandlerCallback_;
  calledHandlerCallback_ = false;
  return result;
}

TEST(ObjectTemplateSetHandlerSetter) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  templ->InstanceTemplate()->SetHandler(v8::IndexedPropertyHandlerConfiguration(
      nullptr,
      [](uint32_t index,
         Local<Value> value,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();

        CHECK_EQ(index, 1);
        CHECK(
            info.Data()
                ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("data"))
                .FromJust());
        CHECK(value->Equals(info.GetIsolate()->GetCurrentContext(), v8_num(22))
                  .FromJust());
      },
      nullptr,
      nullptr,
      nullptr,
      v8_str("data")));

  templ->InstanceTemplate()->SetHandler(v8::NamedPropertyHandlerConfiguration(
      nullptr,
      [](Local<Name> property,
         Local<Value> value,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();

        CHECK(property
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("y"))
                  .FromJust());
        CHECK(
            info.Data()
                ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("data"))
                .FromJust());
        CHECK(value->Equals(info.GetIsolate()->GetCurrentContext(), v8_num(32))
                  .FromJust());
      },
      nullptr,
      nullptr,
      nullptr,
      v8_str("data")));

  auto object = templ->GetFunction(env.local())
                    .ToLocalChecked()
                    ->NewInstance(env.local())
                    .ToLocalChecked();

  env->Global()->Set(env.local(), v8_str("obj"), object).FromJust();

  initCalledHandlerCallback();
  CompileRun("obj[1] = 22");
  CHECK(checkCalledHandlerCallback());
  CompileRun("obj.y = 32");
  CHECK(checkCalledHandlerCallback());
}

TEST(ObjectTemplateSetHandlerDeleter) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  templ->InstanceTemplate()->SetHandler(v8::IndexedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      [](uint32_t index, const v8::PropertyCallbackInfo<v8::Boolean>& info) {
        calledHandlerCallback();

        CHECK_EQ(index, 0);
        info.GetReturnValue().Set(true);
      },
      nullptr,
      nullptr));

  templ->InstanceTemplate()->SetHandler(v8::NamedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      [](Local<Name> property,
         const v8::PropertyCallbackInfo<v8::Boolean>& info) {
        calledHandlerCallback();

        CHECK(property
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("x"))
                  .FromJust());
        info.GetReturnValue().Set(true);
      },
      nullptr,
      nullptr));

  auto object = templ->GetFunction(env.local())
                    .ToLocalChecked()
                    ->NewInstance(env.local())
                    .ToLocalChecked();

  env->Global()->Set(env.local(), v8_str("obj"), object).FromJust();

  initCalledHandlerCallback();
  CHECK(CompileRun("delete obj[0]")->BooleanValue(isolate));
  CHECK(checkCalledHandlerCallback());
  CHECK(CompileRun("delete obj.x")->BooleanValue(isolate));
  CHECK(checkCalledHandlerCallback());
}

TEST(ObjectTemplateSetHandlerDefiner) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  templ->InstanceTemplate()->SetHandler(v8::IndexedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      [](uint32_t index,
         const v8::PropertyDescriptor& desc,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();

        CHECK_EQ(index, 2);
        CHECK(desc.value()
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_num(10))
                  .FromJust());
      }));

  templ->InstanceTemplate()->SetHandler(v8::NamedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      [](Local<Name> property,
         const v8::PropertyDescriptor& desc,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();

        CHECK(property
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("z"))
                  .FromJust());
        CHECK(desc.value()
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_num(20))
                  .FromJust());
      }));

  auto object = templ->GetFunction(env.local())
                    .ToLocalChecked()
                    ->NewInstance(env.local())
                    .ToLocalChecked();

  env->Global()->Set(env.local(), v8_str("obj"), object).FromJust();

  initCalledHandlerCallback();
  CompileRun("Object.defineProperty(obj, 2, {value: 10});");
  CHECK(checkCalledHandlerCallback());
  CompileRun("Object.defineProperty(obj, 'z', {value: 20});");
  CHECK(checkCalledHandlerCallback());
}

TEST(ObjectTemplateSetHandlerDescriptor) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
  templ->InstanceTemplate()->SetHandler(v8::IndexedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      [](uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();
        CHECK_EQ(index, 2);
        Local<Value> descriptor = CompileRun("var desc = {value: 30}; desc;");
        info.GetReturnValue().Set(descriptor);
      }));

  templ->InstanceTemplate()->SetHandler(v8::NamedPropertyHandlerConfiguration(
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      [](Local<Name> property,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        calledHandlerCallback();
        CHECK(property
                  ->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("z"))
                  .FromJust());
        Local<Value> descriptor = CompileRun("var desc = {value: 60}; desc;");
        info.GetReturnValue().Set(descriptor);
      }));

  auto object = templ->GetFunction(env.local())
                    .ToLocalChecked()
                    ->NewInstance(env.local())
                    .ToLocalChecked();

  env->Global()->Set(env.local(), v8_str("obj"), object).FromJust();

  initCalledHandlerCallback();
  v8::Local<Value> valueForName =
      CompileRun("var descForName = Object.getOwnPropertyDescriptor(obj, 2); "
                 "descForName.value;");
  CHECK_EQ(30, valueForName->Int32Value(env.local()).FromJust());
  CHECK(checkCalledHandlerCallback());

  v8::Local<Value> valueForIdx =
      CompileRun("var descForIdx = Object.getOwnPropertyDescriptor(obj, 'z'); "
                 "descForIdx.value;");
  CHECK_EQ(60, valueForIdx->Int32Value(env.local()).FromJust());
  CHECK(checkCalledHandlerCallback());
}

}  // namespace
