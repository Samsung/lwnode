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

class SerializerDelegate : public ValueSerializer::Delegate {
 public:
  explicit SerializerDelegate(Isolate* isolate) : isolate_(isolate) {}
  void ThrowDataCloneError(Local<String> message) override {
    isolate_->ThrowException(Exception::Error(message));
  }

 private:
  Isolate* isolate_;
};

class DeserializerDelegate : public ValueDeserializer::Delegate {
 public:
};

struct MallocedBuffer {
  uint8_t* data;
  size_t size;

  MallocedBuffer(uint8_t* data, size_t size) : data(data), size(size) {}

  ~MallocedBuffer() { free(data); }
};

THREADED_TEST(SerializeWriteReadValue) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  SerializerDelegate serializerdelegate(isolate);
  ValueSerializer serializer(isolate, &serializerdelegate);

  // Number
  double testNumber = 12.25;
  serializer.WriteValue(env.local(), v8_num(testNumber));

  // Integer
  unsigned int testUint = 12;
  serializer.WriteValue(env.local(), v8_int(testUint));
  int testInt = -22;
  serializer.WriteValue(env.local(), v8_int(testInt));

  // Boolean
  serializer.WriteValue(env.local(), Boolean::New(isolate, false));

  // Undefined
  serializer.WriteValue(env.local(), v8::Undefined(isolate));

  // Null
  serializer.WriteValue(env.local(), v8::Null(isolate));

  std::pair<uint8_t*, size_t> data = serializer.Release();
  CHECK(data.first);
  MallocedBuffer buffer(data.first, data.second);

  ValueDeserializer deserializer(isolate, buffer.data, buffer.size);

  // Number
  Local<Value> output;
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsNumber());
  CHECK_EQ(testNumber, output->NumberValue(env.local()).FromJust());

  // Integer
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsUint32());
  CHECK_EQ(testUint, output->Int32Value(env.local()).FromJust());

  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsInt32());
  CHECK_EQ(testInt, output->Int32Value(env.local()).FromJust());

  // Boolean
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsBoolean());
  CHECK(!output->BooleanValue(isolate));

  // Undefined
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsUndefined());

  // Null
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsNull());
}

THREADED_TEST(SerializeWriteReadString) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  SerializerDelegate serializerdelegate(isolate);
  ValueSerializer serializer(isolate, &serializerdelegate);

  // 8bit String
  const char* test8BitString = "Serialize!'; ~12";
  serializer.WriteValue(env.local(), v8_str(test8BitString));

  // 16bit String
  uint16_t test16BitString[2] = {0xD800, 0xDC00};
  Local<String> test16BitLocalString =
      String::NewFromTwoByte(
          env->GetIsolate(), test16BitString, v8::NewStringType::kNormal, 2)
          .ToLocalChecked();

  serializer.WriteValue(env.local(), test16BitLocalString);

  std::pair<uint8_t*, size_t> data = serializer.Release();
  CHECK(data.first);
  MallocedBuffer buffer(data.first, data.second);

  ValueDeserializer deserializer(isolate, buffer.data, buffer.size);

  Local<Value> output;

  // 8bit String
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsString());
  String::Utf8Value output8bitString(env->GetIsolate(), output);
  CHECK_EQ(0, strcmp(*output8bitString, test8BitString));

  // 16bit String
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsString());
  CHECK(output->Equals(isolate->GetCurrentContext(), test16BitLocalString)
            .FromJust());
}

THREADED_TEST(SerializeWriteObject) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  const char* sample = "var rv = {};"
                       "rv.alpha = 'hello';"
                       "rv.beta = 123;"
                       "rv;";

  Local<Value> val = CompileRun(sample);
  CHECK(val->IsObject());
  Local<v8::Object> obj = val.As<v8::Object>();
  obj->Set(env.local(), v8_str("gamma"), v8_bool(true)).FromJust();

  SerializerDelegate serializerdelegate(isolate);
  ValueSerializer serializer(isolate, &serializerdelegate);

  serializer.WriteValue(env.local(), obj);

  std::pair<uint8_t*, size_t> data = serializer.Release();
  CHECK(data.first);
  MallocedBuffer buffer(data.first, data.second);

  ValueDeserializer deserializer(isolate, buffer.data, buffer.size);

  Local<Value> output;
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsObject());
  Local<v8::Object> outputObject = output.As<v8::Object>();
  CHECK(
      v8_str("hello")
          ->Equals(
              env.local(),
              outputObject->Get(env.local(), v8_str("alpha")).ToLocalChecked())
          .FromJust());
  CHECK(v8_num(123)
            ->Equals(
                env.local(),
                outputObject->Get(env.local(), v8_str("beta")).ToLocalChecked())
            .FromJust());
  CHECK(
      v8_bool(true)
          ->Equals(
              env.local(),
              outputObject->Get(env.local(), v8_str("gamma")).ToLocalChecked())
          .FromJust());
}

THREADED_TEST(SerializeWriteReadTypedArray) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context(env.local());

  SerializerDelegate serializerdelegate(isolate);
  ValueSerializer serializer(isolate, &serializerdelegate);

  Local<Value> val = CompileRun("new Uint8Array([0, 11, 22, 33]);");
  CHECK(val->IsUint8Array());
  serializer.WriteValue(context, val);

  std::pair<uint8_t*, size_t> data = serializer.Release();
  CHECK(data.first);
  MallocedBuffer buffer(data.first, data.second);

  ValueDeserializer deserializer(isolate, buffer.data, buffer.size);

  Local<Value> output;
  CHECK(deserializer.ReadValue(context).ToLocal(&output));
  CHECK(output->IsUint8Array());
  Local<v8::Uint8Array> outputArray = output.As<v8::Uint8Array>();
  CHECK_EQ(4, outputArray->Length());
  CHECK_EQ(0,
           outputArray->Get(context, v8_str("0"))
               .ToLocalChecked()
               ->Int32Value(context)
               .FromJust());
  CHECK_EQ(11,
           outputArray->Get(context, v8_str("1"))
               .ToLocalChecked()
               ->Int32Value(context)
               .FromJust());
  CHECK_EQ(22,
           outputArray->Get(context, v8_str("2"))
               .ToLocalChecked()
               ->Int32Value(context)
               .FromJust());
  CHECK_EQ(33,
           outputArray->Get(context, v8_str("3"))
               .ToLocalChecked()
               ->Int32Value(context)
               .FromJust());
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
