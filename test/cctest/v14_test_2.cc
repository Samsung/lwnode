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
  Local<Value> input = v8_num(testNumber);
  serializer.WriteValue(env.local(), input);

  // String
  const char* testString = "Serialize!';";
  input = v8_str(testString);
  serializer.WriteValue(env.local(), input);

  // Boolean
  input = Boolean::New(isolate, false);
  serializer.WriteValue(env.local(), input);

  // Undefined
  input = v8::Undefined(isolate);
  serializer.WriteValue(env.local(), input);

  // Null
  input = v8::Null(isolate);
  serializer.WriteValue(env.local(), input);

  std::pair<uint8_t*, size_t> data = serializer.Release();
  CHECK(data.first);
  MallocedBuffer buffer(data.first, data.second);

  ValueDeserializer deserializer(isolate, buffer.data, buffer.size);

  // Number
  Local<Value> output;
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsNumber());
  CHECK_EQ(testNumber, output->NumberValue(env.local()).FromJust());

  // String
  CHECK(deserializer.ReadValue(env.local()).ToLocal(&output));
  CHECK(output->IsString());
  String::Utf8Value outputString(env->GetIsolate(), output);
  CHECK_EQ(0, strcmp(*outputString, testString));

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
