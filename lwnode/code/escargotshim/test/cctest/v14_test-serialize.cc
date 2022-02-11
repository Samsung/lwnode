/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

class SerializerDelegate;
class DeserializerDelegate;

struct MallocedBuffer {
  uint8_t* data;
  size_t size;

  MallocedBuffer(uint8_t* data, size_t size) : data(data), size(size) {}

  ~MallocedBuffer() { free(data); }
};

class SerializeTest : public ::testing::Test {
 public:
  void SetUp() override {
    isolate_ = CcTest::isolate();
    isolate_->Enter();
    v8::HandleScope scope(isolate_);
    env_ = new LocalContext();
    context_ = env_->local();
  }

  void TearDown() override {
    delete env_;

    CcTest::disposeScope();
    CcTest::disposeIsolate();
  }

  template <typename... T>
  bool serializeValueTest(T... args) {
    SerializerDelegate serializerdelegate(isolate());
    ValueSerializer serializer(isolate(), &serializerdelegate);

    v8::Local<v8::Value> values[] = {args...};
    for (auto value : values) {
      serializer.WriteValue(context(), value);
    }

    std::pair<uint8_t*, size_t> data = serializer.Release();
    CHECK(data.first);
    MallocedBuffer buffer(data.first, data.second);

    ValueDeserializer deserializer(isolate(), buffer.data, buffer.size);

    for (v8::Local<v8::Value>& value : values) {
      Local<Value> output;
      CHECK(deserializer.ReadValue(context()).ToLocal(&output));

      if (value->IsObject() && output->IsObject()) {
        if (!equalsObject(value.As<Object>(), output.As<Object>() ) ) {
          return false;
        }
      }
      else if (!output->Equals(context(), value).FromJust()) {
        return false;
      }
    }

    return true;
  }

  // compare only property of object
  bool equalsObject(Local<Object> objectA, Local<Object> objectB) {
    auto propertyNamesOfObjectA =
        objectA->GetPropertyNames(context()).ToLocalChecked();
    auto propertyNamesOfObjectB =
        objectB->GetPropertyNames(context()).ToLocalChecked();
    if (propertyNamesOfObjectA->Length() != propertyNamesOfObjectB->Length()) {
      return false;
    }
    printf("size: %u\n", propertyNamesOfObjectA->Length());
    for (unsigned i = 0; i < propertyNamesOfObjectA->Length(); i++) {
      v8::Local<v8::Value> propertyName =
          propertyNamesOfObjectA->Get(context(), v8::Integer::New(isolate(), i))
              .ToLocalChecked();
      if (objectA->Get(context(), propertyName)
              .ToLocalChecked()
              ->Equals(context(),
                       objectB->Get(context(), propertyName).ToLocalChecked())
              .FromJust() == false) {
        return false;
      }
    }

    return true;
  }

  v8::Isolate* isolate() { return CcTest::isolate(); }

  Local<Context>& context() { return context_; }

 private:
  LocalContext* env_;
  Local<Context> context_;
  v8::Isolate* isolate_{nullptr};
};

#define SERIALIZE_TEST(Name) TEST_F(SerializeTest, Name)

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

SERIALIZE_TEST(WriteReadValue) {
  v8::HandleScope scope(isolate());

  CHECK(serializeValueTest(v8_num(12.25),
                           v8_int(12),
                           v8_int(-22),
                           v8_bool(true),
                           v8_bool(false),
                           v8::Undefined(isolate()),
                           v8::Null(isolate())));
}

SERIALIZE_TEST(WriteReadString) {
  v8::HandleScope scope(isolate());

  auto test8BitLocalString = v8_str("Serialize!'; ~12");  // 8bit String
  uint16_t test16BitString[2] = {0xD800, 0xDC00};         // 16bit String
  Local<String> test16BitLocalString =
      String::NewFromTwoByte(
          isolate(), test16BitString, v8::NewStringType::kNormal, 2)
          .ToLocalChecked();

  CHECK(serializeValueTest(test8BitLocalString, test16BitLocalString));
}

SERIALIZE_TEST(WriteObject) {
  v8::HandleScope scope(isolate());

  const char* sample = "var rv = {};"
                       "rv.alpha = 'hello';"
                       "rv.beta = 123;"
                       "rv;";

  Local<Value> val = CompileRun(sample);
  CHECK(val->IsObject());
  Local<v8::Object> obj = val.As<v8::Object>();
  obj->Set(context(), v8_str("gamma"), v8_bool(true)).FromJust();

  CHECK(serializeValueTest(obj));
}


SERIALIZE_TEST(WriteReadTypedArray) {
  v8::HandleScope scope(isolate());

  CHECK(serializeValueTest(CompileRun("new Uint8Array([0, 11, 22, 33]);")));
  CHECK(serializeValueTest(CompileRun("new Int8Array([10, 0, -20]);")));
  CHECK(serializeValueTest(CompileRun("new Uint16Array([42, 31]);")));
  CHECK(serializeValueTest(CompileRun("new Int16Array([20, -40]);")));
  CHECK(serializeValueTest(CompileRun("new Uint32Array([0, 10, 20, 30000]);")));
  CHECK(serializeValueTest(CompileRun("new Int32Array([-50000, 50000]);")));
  CHECK(serializeValueTest(CompileRun("new Float32Array([1.12, 10.23, 10]);")));
  CHECK(serializeValueTest(CompileRun("new Float64Array([61.1, 71.3, 20]);")));
  CHECK(serializeValueTest(CompileRun("new Uint8ClampedArray([21, 31]);")));
}
