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

// New internal TC
TEST(StringNewFromUtf8Literal) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

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
               ->Utf8Length(isolate),
           6);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Length(),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Length(),
           2);

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
}

// from node v8 internal
TEST(WriteOneByte) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::TryCatch try_catch(isolate);

  v8::Local<String> str = v8_str("abcde");
  char buf[100];
  int len;

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 4);
  CHECK_EQ(4, len);
  CHECK_EQ(0, strncmp("abcd\1", buf, 5));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 5);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strncmp("abcde\1", buf, 6));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 6);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, -1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 6);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("e\1", buf, 2));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 3, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("d\1", buf, 2));
}

// from node v8's internal
TEST(SymbolPropertiesInternal) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();
  Context::Scope context_scope(context);

  // LocalContext env;
  // v8::Isolate* isolate = env->GetIsolate();
  // v8::HandleScope scope(isolate);

  v8::Local<v8::Object> obj = v8::Object::New(isolate);
  v8::Local<v8::Symbol> sym1 = v8::Symbol::New(isolate);
  v8::Local<v8::Symbol> sym2 = v8::Symbol::New(isolate, v8_str("my-symbol"));
  v8::Local<v8::Symbol> sym3 = v8::Symbol::New(isolate, v8_str("sym3"));

  CHECK(sym1->IsSymbol());
  CHECK(sym2->IsSymbol());
  CHECK(!obj->IsSymbol());

  CHECK(sym1->Equals(context, sym1).FromJust());
  CHECK(sym2->Equals(context, sym2).FromJust());
  CHECK(!sym1->Equals(context, sym2).FromJust());
  CHECK(!sym2->Equals(context, sym1).FromJust());
  CHECK(sym1->StrictEquals(sym1));
  CHECK(sym2->StrictEquals(sym2));
  CHECK(!sym1->StrictEquals(sym2));
  CHECK(!sym2->StrictEquals(sym1));
  CHECK(sym2->Name()->Equals(context, v8_str("my-symbol")).FromJust());
}

// Helper functions for Interceptor/Accessor interaction tests

void SimpleAccessorGetter1(Local<String> name,
                           const v8::PropertyCallbackInfo<v8::Value>& info) {
  Local<Object> self = Local<Object>::Cast(info.This());
  info.GetReturnValue().Set(
      self->Get(info.GetIsolate()->GetCurrentContext(),
                String::Concat(info.GetIsolate(), v8_str("accessor_"), name))
          .ToLocalChecked());
}

void SimpleAccessorSetter1(Local<String> name,
                           Local<Value> value,
                           const v8::PropertyCallbackInfo<void>& info) {
  Local<Object> self = Local<Object>::Cast(info.This());
  CHECK(self->Set(info.GetIsolate()->GetCurrentContext(),
                  String::Concat(info.GetIsolate(), v8_str("accessor_"), name),
                  value)
            .FromJust());
}

void SymbolAccessorGetter1(Local<Name> name,
                           const v8::PropertyCallbackInfo<v8::Value>& info) {
  CHECK(name->IsSymbol());
  Local<Symbol> sym = Local<Symbol>::Cast(name);
  if (sym->Description()->IsUndefined()) return;
  SimpleAccessorGetter1(Local<String>::Cast(sym->Description()), info);
}

void SymbolAccessorSetter1(Local<Name> name,
                           Local<Value> value,
                           const v8::PropertyCallbackInfo<void>& info) {
  CHECK(name->IsSymbol());
  Local<Symbol> sym = Local<Symbol>::Cast(name);
  if (sym->Description()->IsUndefined()) return;
  SimpleAccessorSetter1(Local<String>::Cast(sym->Description()), value, info);
}

THREADED_TEST(SymbolPropertiesInternal2) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();
  Context::Scope context_scope(context);

  v8::Local<v8::Object> obj = v8::Object::New(isolate);
  v8::Local<v8::Symbol> sym1 = v8::Symbol::New(isolate);
  v8::Local<v8::Symbol> sym2 = v8::Symbol::New(isolate, v8_str("my-symbol"));
  v8::Local<v8::Symbol> sym3 = v8::Symbol::New(isolate, v8_str("sym3"));
  v8::Local<v8::Symbol> sym4 = v8::Symbol::New(isolate, v8_str("native"));

  // Check basic symbol functionality.
  CHECK(sym1->IsSymbol());
  CHECK(sym2->IsSymbol());
  CHECK(!obj->IsSymbol());

  CHECK(sym1->Equals(env.local(), sym1).FromJust());
  CHECK(sym2->Equals(env.local(), sym2).FromJust());
  CHECK(!sym1->Equals(env.local(), sym2).FromJust());
  CHECK(!sym2->Equals(env.local(), sym1).FromJust());
  CHECK(sym1->StrictEquals(sym1));
  CHECK(sym2->StrictEquals(sym2));
  CHECK(!sym1->StrictEquals(sym2));
  CHECK(!sym2->StrictEquals(sym1));

  CHECK(
      sym2->Description()->Equals(env.local(), v8_str("my-symbol")).FromJust());

  v8::Local<v8::Value> sym_val = sym2;
  CHECK(sym_val->IsSymbol());
  CHECK(sym_val->Equals(env.local(), sym2).FromJust());
  CHECK(sym_val->StrictEquals(sym2));
  CHECK(v8::Symbol::Cast(*sym_val)->Equals(env.local(), sym2).FromJust());

  v8::Local<v8::Value> sym_obj = v8::SymbolObject::New(isolate, sym2);
  CHECK(sym_obj->IsSymbolObject());
  CHECK(!sym2->IsSymbolObject());
  CHECK(!obj->IsSymbolObject());
  CHECK(sym_obj->Equals(env.local(), sym2).FromJust());
  CHECK(!sym_obj->StrictEquals(sym2));
  CHECK(v8::SymbolObject::Cast(*sym_obj)
            ->Equals(env.local(), sym_obj)
            .FromJust());
  CHECK(v8::SymbolObject::Cast(*sym_obj)
            ->ValueOf()
            ->Equals(env.local(), sym2)
            .FromJust());

  // Make sure delete of a non-existent symbol property works.
  // CHECK(obj->Delete(env.local(), sym1).FromJust());
  CHECK(!obj->Has(env.local(), sym1).FromJust());

  CHECK(
      obj->Set(env.local(), sym1, v8::Integer::New(isolate, 1503)).FromJust());
  CHECK(obj->Has(env.local(), sym1).FromJust());
  CHECK_EQ(1503,
           obj->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK(
      obj->Set(env.local(), sym1, v8::Integer::New(isolate, 2002)).FromJust());
  CHECK(obj->Has(env.local(), sym1).FromJust());
  CHECK_EQ(2002,
           obj->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  // CHECK_EQ(v8::None, obj->GetPropertyAttributes(env.local(),
  // sym1).FromJust());

  CHECK_EQ(0u,
           obj->GetOwnPropertyNames(env.local()).ToLocalChecked()->Length());
  unsigned num_props =
      obj->GetPropertyNames(env.local()).ToLocalChecked()->Length();
  CHECK(obj->Set(env.local(), v8_str("bla"), v8::Integer::New(isolate, 20))
            .FromJust());
  CHECK_EQ(1u,
           obj->GetOwnPropertyNames(env.local()).ToLocalChecked()->Length());
  CHECK_EQ(num_props + 1,
           obj->GetPropertyNames(env.local()).ToLocalChecked()->Length());

  // CHECK(obj->SetAccessor(env.local(), sym3, SymbolAccessorGetter1,
  //                        SymbolAccessorSetter1)
  //           .FromJust());
  CHECK(obj->Get(env.local(), sym3).ToLocalChecked()->IsUndefined());
  CHECK(obj->Set(env.local(), sym3, v8::Integer::New(isolate, 42)).FromJust());
  CHECK(obj->Get(env.local(), sym3)
            .ToLocalChecked()
            ->Equals(env.local(), v8::Integer::New(isolate, 42))
            .FromJust());
  // CHECK(obj->Get(env.local(), v8_str("accessor_sym3"))
  //           .ToLocalChecked()
  //           ->Equals(env.local(), v8::Integer::New(isolate, 42))
  //           .FromJust());

  // CHECK(obj->SetNativeDataProperty(env.local(), sym4, SymbolAccessorGetter)
  //           .FromJust());
  CHECK(obj->Get(env.local(), sym4).ToLocalChecked()->IsUndefined());
  CHECK(obj->Set(env.local(),
                 v8_str("accessor_native"),
                 v8::Integer::New(isolate, 123))
            .FromJust());
  // CHECK_EQ(123, obj->Get(env.local(), sym4)
  //                   .ToLocalChecked()
  //                   ->Int32Value(env.local())
  //                   .FromJust());
  CHECK(obj->Set(env.local(), sym4, v8::Integer::New(isolate, 314)).FromJust());
  CHECK(obj->Get(env.local(), sym4)
            .ToLocalChecked()
            ->Equals(env.local(), v8::Integer::New(isolate, 314))
            .FromJust());
  // CHECK(obj->Delete(env.local(), v8_str("accessor_native")).FromJust());

  // Add another property and delete it afterwards to force the object in
  // slow case.
  CHECK(
      obj->Set(env.local(), sym2, v8::Integer::New(isolate, 2008)).FromJust());
  CHECK_EQ(2002,
           obj->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK_EQ(2008,
           obj->Get(env.local(), sym2)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK_EQ(2002,
           obj->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK_EQ(2u,
           obj->GetOwnPropertyNames(env.local()).ToLocalChecked()->Length());

  CHECK(obj->Has(env.local(), sym1).FromJust());
  CHECK(obj->Has(env.local(), sym2).FromJust());
  CHECK(obj->Has(env.local(), sym3).FromJust());
  // CHECK(obj->Has(env.local(), v8_str("accessor_sym3")).FromJust());
  CHECK(obj->Delete(env.local(), sym2).FromJust());
  CHECK(obj->Has(env.local(), sym1).FromJust());
  CHECK(!obj->Has(env.local(), sym2).FromJust());
  CHECK(obj->Has(env.local(), sym3).FromJust());
  // CHECK(obj->Has(env.local(), v8_str("accessor_sym3")).FromJust());
  CHECK_EQ(2002,
           obj->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK(obj->Get(env.local(), sym3)
            .ToLocalChecked()
            ->Equals(env.local(), v8::Integer::New(isolate, 42))
            .FromJust());
  // CHECK(obj->Get(env.local(), v8_str("accessor_sym3"))
  //           .ToLocalChecked()
  //           ->Equals(env.local(), v8::Integer::New(isolate, 42))
  //           .FromJust());
  CHECK_EQ(2u,
           obj->GetOwnPropertyNames(env.local()).ToLocalChecked()->Length());

  // Symbol properties are inherited.
  v8::Local<v8::Object> child = v8::Object::New(isolate);
  CHECK(child->SetPrototype(env.local(), obj).FromJust());
  CHECK(child->Has(env.local(), sym1).FromJust());
  CHECK_EQ(2002,
           child->Get(env.local(), sym1)
               .ToLocalChecked()
               ->Int32Value(env.local())
               .FromJust());
  CHECK(obj->Get(env.local(), sym3)
            .ToLocalChecked()
            ->Equals(env.local(), v8::Integer::New(isolate, 42))
            .FromJust());
  // CHECK(obj->Get(env.local(), v8_str("accessor_sym3"))
  //           .ToLocalChecked()
  //           ->Equals(env.local(), v8::Integer::New(isolate, 42))
  //           .FromJust());
  CHECK_EQ(0u,
           child->GetOwnPropertyNames(env.local()).ToLocalChecked()->Length());
}

// from node 14's test-api.cc
THREADED_TEST(Array2) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();
  Context::Scope context_scope(context);

  // LocalContext context;
  // v8::HandleScope scope(context->GetIsolate());
  Local<v8::Array> array = v8::Array::New(context->GetIsolate());
  CHECK_EQ(0u, array->Length());

  CHECK(array->Get(context, 0).ToLocalChecked()->IsUndefined());
  CHECK(!array->Has(context, 0).FromJust());
  CHECK(array->Get(context, 100).ToLocalChecked()->IsUndefined());
  CHECK(!array->Has(context, 100).FromJust());
  CHECK(array->Set(context, 2, v8_num(7)).FromJust());
  CHECK_EQ(3u, array->Length());

  CHECK(!array->Has(context, 0).FromJust());
  CHECK(!array->Has(context, 1).FromJust());
  CHECK(array->Has(context, 2).FromJust());
  CHECK_EQ(
      7,
      array->Get(context, 2).ToLocalChecked()->Int32Value(context).FromJust());
  // Local<Value> obj = CompileRun("[1, 2, 3]");
  // Local<v8::Array> arr = obj.As<v8::Array>();
  Local<v8::Array> arr = v8::Array::New(context->GetIsolate());
  CHECK(arr->Set(context, 0, v8_num(1)).FromJust());
  CHECK(arr->Set(context, 1, v8_num(2)).FromJust());
  CHECK(arr->Set(context, 2, v8_num(3)).FromJust());
  CHECK_EQ(3u, arr->Length());
  CHECK_EQ(
      1, arr->Get(context, 0).ToLocalChecked()->Int32Value(context).FromJust());
  CHECK_EQ(
      2, arr->Get(context, 1).ToLocalChecked()->Int32Value(context).FromJust());
  CHECK_EQ(
      3, arr->Get(context, 2).ToLocalChecked()->Int32Value(context).FromJust());
  array = v8::Array::New(context->GetIsolate(), 27);
  CHECK_EQ(27u, array->Length());
  array = v8::Array::New(context->GetIsolate(), -27);
  CHECK_EQ(0u, array->Length());

  std::vector<Local<Value>> vector = {v8_num(1), v8_num(2), v8_num(3)};
  array = v8::Array::New(context->GetIsolate(), vector.data(), vector.size());
  CHECK_EQ(vector.size(), array->Length());
  CHECK_EQ(
      1, arr->Get(context, 0).ToLocalChecked()->Int32Value(context).FromJust());
  CHECK_EQ(
      2, arr->Get(context, 1).ToLocalChecked()->Int32Value(context).FromJust());
  CHECK_EQ(
      3, arr->Get(context, 2).ToLocalChecked()->Int32Value(context).FromJust());
}

class TestResource : public String::ExternalStringResource {
 public:
  explicit TestResource(uint16_t* data,
                        int* counter = nullptr,
                        bool owning_data = true)
      : data_(data), length_(0), counter_(counter), owning_data_(owning_data) {
    while (data[length_]) ++length_;
  }

  ~TestResource() override {
    if (owning_data_) i::DeleteArray(data_);
    if (counter_ != nullptr) ++*counter_;
  }

  const uint16_t* data() const override { return data_; }

  size_t length() const override { return length_; }

 private:
  uint16_t* data_;
  size_t length_;
  int* counter_;
  bool owning_data_;
};

class TestOneByteResource : public String::ExternalOneByteStringResource {
 public:
  explicit TestOneByteResource(const char* data,
                               int* counter = nullptr,
                               size_t offset = 0)
      : orig_data_(data),
        data_(data + offset),
        length_(strlen(data) - offset),
        counter_(counter) {}

  ~TestOneByteResource() override {
    i::DeleteArray(orig_data_);
    if (counter_ != nullptr) ++*counter_;
  }

  const char* data() const override { return data_; }

  size_t length() const override { return length_; }

 private:
  const char* orig_data_;
  const char* data_;
  size_t length_;
  int* counter_;
};

THREADED_TEST(StringConcatInternal) {
  TestOneByteResource* testOneByteResource;
  TestResource* testResource;
  {
    LocalContext env;
    v8::Isolate* isolate = env->GetIsolate();
    v8::HandleScope scope(isolate);
    const char* one_byte_string_1 = "function a_times_t";
    const char* two_byte_string_1 = "wo_plus_b(a, b) {return ";
    const char* one_byte_extern_1 = "a * 2 + b;} a_times_two_plus_b(4, 8) + ";
    const char* two_byte_extern_1 = "a_times_two_plus_b(4, 8) + ";
    const char* one_byte_string_2 = "a_times_two_plus_b(4, 8) + ";
    const char* two_byte_string_2 = "a_times_two_plus_b(4, 8) + ";
    const char* two_byte_extern_2 = "a_times_two_plus_b(1, 2);";
    Local<String> left = v8_str(one_byte_string_1);

    uint16_t* two_byte_source = AsciiToTwoByteString(two_byte_string_1);
    Local<String> right =
        String::NewFromTwoByte(env->GetIsolate(), two_byte_source)
            .ToLocalChecked();
    i::DeleteArray(two_byte_source);

    Local<String> source = String::Concat(isolate, left, right);
    {
      uint16_t* str = AsciiToTwoByteString("function a_times_t"
                                           "wo_plus_b(a, b) {return ");
      Local<String> concat =
          String::NewFromTwoByte(env->GetIsolate(), str).ToLocalChecked();
      CHECK(source->StringEquals(concat));
      i::DeleteArray(str);
    }

    testOneByteResource = new TestOneByteResource(i::StrDup(one_byte_extern_1));
    right = String::NewExternalOneByte(env->GetIsolate(), testOneByteResource)
                .ToLocalChecked();
    source = String::Concat(isolate, source, right);
    {
      uint16_t* str =
          AsciiToTwoByteString("function a_times_t"
                               "wo_plus_b(a, b) {return "
                               "a * 2 + b;} a_times_two_plus_b(4, 8) + ");
      Local<String> concat =
          String::NewFromTwoByte(env->GetIsolate(), str).ToLocalChecked();
      CHECK(source->StringEquals(concat));
      i::DeleteArray(str);
    }
    delete testOneByteResource;

    testResource = new TestResource(AsciiToTwoByteString(two_byte_extern_1));
    right = String::NewExternalTwoByte(env->GetIsolate(), testResource)
                .ToLocalChecked();
    source = String::Concat(isolate, source, right);
    right = v8_str(one_byte_string_2);
    source = String::Concat(isolate, source, right);
    delete testResource;

    two_byte_source = AsciiToTwoByteString(two_byte_string_2);
    right = String::NewFromTwoByte(env->GetIsolate(), two_byte_source)
                .ToLocalChecked();
    i::DeleteArray(two_byte_source);

    testResource = new TestResource(AsciiToTwoByteString(two_byte_extern_2));
    source = String::Concat(isolate, source, right);
    right = String::NewExternalTwoByte(env->GetIsolate(), testResource)
                .ToLocalChecked();
    source = String::Concat(isolate, source, right);
    {
      uint16_t* str =
          AsciiToTwoByteString("function a_times_t"
                               "wo_plus_b(a, b) {return "
                               "a * 2 + b;} a_times_two_plus_b(4, 8) + "
                               "a_times_two_plus_b(4, 8) + "
                               "a_times_two_plus_b(4, 8) + "
                               "a_times_two_plus_b(4, 8) + "
                               "a_times_two_plus_b(1, 2);");
      Local<String> concat =
          String::NewFromTwoByte(env->GetIsolate(), str).ToLocalChecked();
      CHECK(source->StringEquals(concat));
      i::DeleteArray(str);
    }
    delete testResource;
    // FIXME: Check v8_compile()
    // Local<Script> script = v8_compile(source);
    // Local<Value> value = script->Run(env.local()).ToLocalChecked();
    // CHECK(value->IsNumber());
    // CHECK_EQ(68, value->Int32Value(env.local()).FromJust());
  }
}

THREADED_TEST(TwoByteStringToOneByteString) {
  // NOTE: See String::WriteOneByte() to check the purpose of this TC
  // Related to: test/parallel/test-buffer-write.js
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();
  Context::Scope context_scope(context);

  uint16_t twoByteString[9] = {'f', 'o', 'o', 0, 0, 0, 0, 0, 0};

  Local<String> s =
      String::NewFromTwoByte(isolate, twoByteString).ToLocalChecked();

  char buf[9];
  int len = s->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(len, 3);
  CHECK_EQ(0, strncmp("foo", buf, 3));
}
