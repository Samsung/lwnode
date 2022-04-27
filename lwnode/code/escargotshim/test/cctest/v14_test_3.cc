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

#include <EscargotPublic.h>

#include "api/isolate.h"
#include "base.h"

using namespace v8;
using namespace EscargotShim;

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

THREADED_TEST(Int32Number) {
  // Ref: test/parallel/test-buffer-indexof.js
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context = env.local();
  Context::Scope context_scope(context);

  v8::Local<v8::Number> num = v8::Number::New(isolate, 1);
  CHECK(num->IsUint32());

  num = v8::Number::New(isolate, 1);
  CHECK(num->IsInt32());

  num = v8::Number::New(isolate, -1);
  CHECK(num->IsInt32());

  num = v8::Number::New(isolate, -1);
  CHECK(!num->IsUint32());
}

class PromiseHookDataCustom {
 public:
  int init = 0;
  int resolve = 0;
  int before = 0;
  int after = 0;
  int parent = 0;
};

PromiseHookDataCustom* promiseHookData = nullptr;

void PromiseHookCallback(v8::PromiseHookType type,
                         v8::Local<v8::Promise> promise,
                         v8::Local<v8::Value> parentPromise) {
  switch (type) {
    case v8::PromiseHookType::kInit:
      promiseHookData->init++;
      if (!parentPromise->IsUndefined()) {
        promiseHookData->parent++;
      }
      break;
    case v8::PromiseHookType::kResolve:
      promiseHookData->resolve++;
      break;
    case v8::PromiseHookType::kBefore:
      promiseHookData->before++;
      break;
    case v8::PromiseHookType::kAfter:
      promiseHookData->after++;
      break;
  }
}

THREADED_TEST(PromiseHookCustom) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  promiseHookData = new PromiseHookDataCustom();
  isolate->SetPromiseHook(PromiseHookCallback);

  Local<v8::Promise::Resolver> p1 =
      v8::Promise::Resolver::New(context.local()).ToLocalChecked();
  Local<v8::Promise::Resolver> p2 =
      v8::Promise::Resolver::New(context.local()).ToLocalChecked();
  CHECK_EQ(promiseHookData->init, 2);

  Local<v8::Promise> p = p1->GetPromise();
  Local<v8::Promise> r = p2->GetPromise();

  p1->Resolve(context.local(), v8::Integer::New(isolate, 1)).FromJust();
  CHECK_EQ(promiseHookData->resolve, 1);

  p2->Reject(context.local(), v8::Integer::New(isolate, 2)).FromJust();
  CHECK_EQ(promiseHookData->resolve, 2);

  delete promiseHookData;
}

static void ThrowingGetterCustom(
    Local<String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
  // info.GetIsolate()->ThrowException(Local<Value>());
  info.GetReturnValue().SetUndefined();
}

THREADED_TEST(VariousGetPropertiesAndThrowingCallbacksCustom) {
  LocalContext context;
  HandleScope scope(context->GetIsolate());

  Local<FunctionTemplate> templ = FunctionTemplate::New(context->GetIsolate());
  Local<ObjectTemplate> instance_templ = templ->InstanceTemplate();
  instance_templ->SetAccessor(v8_str("f"), ThrowingGetterCustom);

  Local<Object> instance = templ->GetFunction(context.local())
                               .ToLocalChecked()
                               ->NewInstance(context.local())
                               .ToLocalChecked();

  Local<Object> another = Object::New(context->GetIsolate());
  CHECK(another->SetPrototype(context.local(), instance).FromJust());

  Local<Object> with_js_getter =
      CompileRun("o = {};\n"
                 "o.__defineGetter__('f', function() { throw undefined; });\n"
                 "o\n")
          .As<Object>();
  CHECK(!with_js_getter.IsEmpty());

  TryCatch try_catch(context->GetIsolate());

  v8::MaybeLocal<Value> result =
      instance->GetRealNamedProperty(context.local(), v8_str("f"));
  // CHECK(try_catch.HasCaught());
  // try_catch.Reset();
  CHECK(result.IsEmpty());

  Maybe<PropertyAttribute> attr =
      instance->GetRealNamedPropertyAttributes(context.local(), v8_str("f"));
  CHECK(!try_catch.HasCaught());
  CHECK(Just(None) == attr);

  result = another->GetRealNamedProperty(context.local(), v8_str("f"));
  // CHECK(try_catch.HasCaught());
  try_catch.Reset();
  CHECK(result.IsEmpty());

  attr = another->GetRealNamedPropertyAttributes(context.local(), v8_str("f"));
  CHECK(!try_catch.HasCaught());
  CHECK(Just(None) == attr);

  // result = another->GetRealNamedPropertyInPrototypeChain(context.local(),
  //                                                        v8_str("f"));
  // CHECK(try_catch.HasCaught());
  // try_catch.Reset();
  // CHECK(result.IsEmpty());

  // attr = another->GetRealNamedPropertyAttributesInPrototypeChain(
  //     context.local(), v8_str("f"));
  // CHECK(!try_catch.HasCaught());
  // CHECK(Just(None) == attr);

  // result = another->Get(context.local(), v8_str("f"));
  // CHECK(try_catch.HasCaught());
  // try_catch.Reset();
  // CHECK(result.IsEmpty());

  // result = with_js_getter->GetRealNamedProperty(context.local(),
  // v8_str("f")); CHECK(try_catch.HasCaught()); try_catch.Reset();
  // CHECK(result.IsEmpty());

  // attr = with_js_getter->GetRealNamedPropertyAttributes(context.local(),
  //                                                       v8_str("f"));
  // CHECK(!try_catch.HasCaught());
  // CHECK(Just(None) == attr);

  // result = with_js_getter->Get(context.local(), v8_str("f"));
  // CHECK(try_catch.HasCaught());
  // try_catch.Reset();
  // CHECK(result.IsEmpty());

  Local<Object> target = CompileRun("({})").As<Object>();
  Local<Object> handler = CompileRun("({})").As<Object>();
  Local<v8::Proxy> proxy =
      v8::Proxy::New(context.local(), target, handler).ToLocalChecked();

  result = target->GetRealNamedProperty(context.local(), v8_str("f"));
  CHECK(!try_catch.HasCaught());
  CHECK(result.IsEmpty());

  result = proxy->GetRealNamedProperty(context.local(), v8_str("f"));
  CHECK(!try_catch.HasCaught());
  CHECK(result.IsEmpty());
}

THREADED_TEST(ThrowUndefinedExceptionCrashTest) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  TryCatch try_catch(context->GetIsolate());
  CompileRun("function f() { throw undefined; }; f();");
}

THREADED_TEST(OneByteStringTest) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  { ExpectString("function f() { return 'twø'; }; f();", "twø"); }

  {
    Local<String> s =
        CompileRun("function f() { return 'twø'; }; f();").As<String>();
    char buf[10] = {0};
    int len = s->WriteUtf8(isolate, buf);
    CHECK_EQ(0, strcmp(buf, "twø"));
  }

  {
    int flags = String::HINT_MANY_WRITES_EXPECTED |
                String::NO_NULL_TERMINATION | String::REPLACE_INVALID_UTF8;

    v8::Local<String> utf8 =
        v8::String::NewFromUtf8(
            context->GetIsolate(), "twø", v8::NewStringType::kNormal)
            .ToLocalChecked();

    char buf[5] = {0};
    int len = utf8->WriteUtf8(isolate, buf, flags);
    CHECK_EQ(0, strcmp(buf, "twø"));
  }
}

THREADED_TEST(SharedArrayBufferCustom) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  {
    v8::Local<SharedArrayBuffer> sa =
        v8::SharedArrayBuffer::New(context->GetIsolate(), 4);
    CHECK_EQ(4, sa->ByteLength());
  }

  {
    auto bs = v8::SharedArrayBuffer::NewBackingStore(isolate, 10);
    CHECK(bs);
  }

  {
    v8::Local<SharedArrayBuffer> sa;
    {
      auto bs = v8::SharedArrayBuffer::NewBackingStore(isolate, 10);
      CHECK(bs);
      sa = v8::SharedArrayBuffer::New(context->GetIsolate(), std::move(bs));
      CHECK_EQ(10, sa->ByteLength());
    }

    CHECK_EQ(10, sa->ByteLength());
    auto bs = sa->GetBackingStore();
    CHECK_EQ(10, bs->ByteLength());
  }

  {
    int data[10];
    int called = false;
    auto deleter = [](void* data, size_t, void* deleteData) {
      bool* called = (bool*)deleteData;
      *called = true;
    };
    {
      auto bs = v8::SharedArrayBuffer::NewBackingStore(
          &data, sizeof(data), deleter, &called);
      CHECK(bs);
    }
    CcTest::CollectAllGarbage();
    CHECK(called);
  }
}

THREADED_TEST(InheritanceCustom) {
  LocalContext env;
  v8::Isolate* isolate = CcTest::isolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> funA = v8::FunctionTemplate::New(isolate);
  // funA->SetClassName(v8_str("A"));
  funA->InstanceTemplate()->SetInternalFieldCount(10);
  funA->PrototypeTemplate()->Set(
      v8::String::NewFromUtf8(isolate, "f").ToLocalChecked(),
      FunctionTemplate::New(isolate,
                            [](const FunctionCallbackInfo<Value>& info) {
                              info.GetReturnValue().Set(v8_num(1));
                            }));

  Local<v8::FunctionTemplate> funB = v8::FunctionTemplate::New(isolate);
  // funB->SetClassName(v8_str("B"));
  funB->InstanceTemplate()->SetInternalFieldCount(10);
  funB->PrototypeTemplate()->Set(
      v8::String::NewFromUtf8(isolate, "f").ToLocalChecked(),
      FunctionTemplate::New(isolate,
                            [](const FunctionCallbackInfo<Value>& info) {
                              info.GetReturnValue().Set(v8_num(2));
                            }));
  funB->Inherit(funA);

  Local<v8::FunctionTemplate> funC = v8::FunctionTemplate::New(isolate);
  // funC->SetClassName(v8_str("C"));
  funC->InstanceTemplate()->SetInternalFieldCount(10);
  funC->PrototypeTemplate()->Set(
      v8::String::NewFromUtf8(isolate, "f").ToLocalChecked(),
      FunctionTemplate::New(isolate,
                            [](const FunctionCallbackInfo<Value>& info) {
                              info.GetReturnValue().Set(v8_num(3));
                            }));
  funC->Inherit(funB);

  funA->InstanceTemplate()->SetAccessor(
      v8_str("knurd"),
      [](Local<String> property,
         const v8::PropertyCallbackInfo<v8::Value>& info) {
        info.GetReturnValue().Set(v8_num(15.2));
      });

  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("A"),
                  funA->GetFunction(env.local()).ToLocalChecked())
            .FromJust());
  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("B"),
                  funB->GetFunction(env.local()).ToLocalChecked())
            .FromJust());
  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("C"),
                  funC->GetFunction(env.local()).ToLocalChecked())
            .FromJust());

  CompileRun("A();");
  CompileRun("B();");
  CompileRun("C();");

  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("a"),
                  funA->GetFunction(env.local())
                      .ToLocalChecked()
                      ->NewInstance(env.local())
                      .ToLocalChecked())
            .FromJust());
  CHECK_EQ(1, CompileRun("a.f()")->NumberValue(env.local()).FromJust());

  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("b"),
                  funB->GetFunction(env.local())
                      .ToLocalChecked()
                      ->NewInstance(env.local())
                      .ToLocalChecked())
            .FromJust());
  CHECK_EQ(2, CompileRun("b.f()")->NumberValue(env.local()).FromJust());

  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("c"),
                  funC->GetFunction(env.local())
                      .ToLocalChecked()
                      ->NewInstance(env.local())
                      .ToLocalChecked())
            .FromJust());
  CHECK_EQ(3, CompileRun("c.f()")->NumberValue(env.local()).FromJust());

  CHECK_EQ(15.2, CompileRun("b.knurd")->NumberValue(env.local()).FromJust());
}

static void Returns42C(const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(42);
}

TEST(ChainSignatureCheckCustom) {
  LocalContext context;
  auto isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);
  auto global = context->Global();
  auto sig_obj = FunctionTemplate::New(isolate);
  auto sig = v8::Signature::New(isolate, sig_obj);
  for (int i = 0; i < 4; ++i) {
    auto temp = FunctionTemplate::New(isolate);
    temp->Inherit(sig_obj);
    sig_obj = temp;
  }

  auto x =
      FunctionTemplate::New(isolate,
                            Returns42C,
                            Local<Value>(),
                            v8::Signature::New(isolate, sig_obj));  // <-- super

  global
      ->Set(context.local(),
            v8_str("sig_obj"),
            sig_obj->GetFunction(context.local()).ToLocalChecked())  // <-- base
      .FromJust();

  global
      ->Set(context.local(),
            v8_str("x"),
            x->GetFunction(context.local()).ToLocalChecked())  // <-- super
      .FromJust();

  CompileRun("var s = new sig_obj();");  // <-- base

  {
    TryCatch try_catch(isolate);
    CompileRun("x()");
    CHECK(try_catch.HasCaught());
  }
  {
    TryCatch try_catch(isolate);
    CompileRun("x.call(1)");
    CHECK(try_catch.HasCaught());
  }
  {
    TryCatch try_catch(isolate);
    auto result = CompileRun("s.x = x; s.x()");
    CHECK(!try_catch.HasCaught());
    CHECK_EQ(42, result->Int32Value(context.local()).FromJust());
  }
  {
    TryCatch try_catch(isolate);
    auto result = CompileRun("x.call(s)");
    CHECK(!try_catch.HasCaught());
    CHECK_EQ(42, result->Int32Value(context.local()).FromJust());
  }
}

void templateHandlerCallbackGet(
    Local<Name> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  if (property->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("foo"))
          .FromJust()) {
    info.GetReturnValue().Set(v8_str("yes"));
  }
}

void templateHandlerCallbackSet(
    Local<Name> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  if (property->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("foo"))
          .FromJust()) {
    info.GetReturnValue().Set(v8_str("yes"));
  }
}

void templateHandlerCallbackQuery(
    Local<Name> property, const v8::PropertyCallbackInfo<v8::Integer>& info) {
  if (property->Equals(info.GetIsolate()->GetCurrentContext(), v8_str("foo"))
          .FromJust()) {
    info.GetReturnValue().Set(v8_int(0));
  }
}

THREADED_TEST(TemplateHandlerCallbackCustom) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  {
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
    templ->SetHandler(
        v8::NamedPropertyHandlerConfiguration(templateHandlerCallbackGet));
    Local<Object> instance = templ->NewInstance(env.local()).ToLocalChecked();
    CHECK(!instance->HasOwnProperty(env.local(), v8_str("1")).FromJust());
    CHECK(!instance->HasOwnProperty(env.local(), 1).FromJust());
    CHECK(instance->HasOwnProperty(env.local(), v8_str("foo")).FromJust());
    CHECK(!instance->HasOwnProperty(env.local(), v8_str("bar")).FromJust());
  }
  {
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
    templ->SetHandler(
        v8::NamedPropertyHandlerConfiguration(templateHandlerCallbackGet,
                                              templateHandlerCallbackSet,
                                              templateHandlerCallbackQuery));
    Local<Object> instance = templ->NewInstance(env.local()).ToLocalChecked();
    CHECK(!instance->HasOwnProperty(env.local(), v8_str("bar")).FromJust());
    CHECK(!instance->HasOwnProperty(env.local(), v8_str("bar")).FromJust());
  }
}

THREADED_TEST(SetGetFlagsCustom) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  std::string userOptions[] = {"program.exe",
                               "--expose-gc",
                               "--use-strict",
                               "--off-idlegc",
                               "--harmony-top-level-await",
                               "--allow-code-generation-from-strings",
                               "--trace-gc",
                               "--trace-call=",
                               "--internal-log",
                               "--trace-debug",
                               "--debug",
                               "--stack-size=",
                               "--nolazy"};
  int arrayLength = sizeof(userOptions) / sizeof(userOptions[0]);

  std::unique_ptr<char*[]> argv(new char*[arrayLength]);
  for (int i = 1; i < arrayLength; i++) {
    argv[i] = (char*)userOptions[i].c_str();
  }

  auto optionsBackup = EscargotShim::Global::flags()->get();

  v8::V8::SetFlagsFromCommandLine(&arrayLength, argv.get(), true);

  CHECK(
      EscargotShim::Global::flags()->isOn(EscargotShim::Flag::Type::ExposeGC));
  CHECK(
      EscargotShim::Global::flags()->isOn(EscargotShim::Flag::Type::UseStrict));
  CHECK(EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::DisableIdleGC));
  CHECK(EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::TopLevelWait));
  CHECK(EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::AllowCodeGenerationFromString));
  CHECK(EscargotShim::Global::flags()->isOn(EscargotShim::Flag::Type::TraceGC));
  CHECK(
      EscargotShim::Global::flags()->isOn(EscargotShim::Flag::Type::TraceCall));
  CHECK(EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::InternalLog));
  CHECK(!EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::LWNodeOther));

  EscargotShim::Global::flags()->set(optionsBackup);
}

static void simpleCallbackCustom(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(v8_num(10 + info.Length()));
}

template <typename Callback>
static void functionTemplateCallbackCustom(Callback callback) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::ObjectTemplate> object_template =
      v8::ObjectTemplate::New(isolate);
  object_template->Set(
      isolate, "callback", v8::FunctionTemplate::New(isolate, callback));
  v8::Local<v8::Object> object =
      object_template->NewInstance(env.local()).ToLocalChecked();
  CHECK((*env)->Global()->Set(env.local(), v8_str("obj"), object).FromJust());

  v8::Local<v8::Script> script;
  script = v8_compile("obj.callback(1)");
  for (int i = 0; i < 30; i++) {
    CHECK_EQ(11, v8_run_int32value(script));
  }
  script = v8_compile("obj.callback(1, 2)");
  for (int i = 0; i < 30; i++) {
    CHECK_EQ(12, v8_run_int32value(script));
  }
}

THREADED_PROFILED_TEST(FunctionTemplateCallbackCustom) {
  functionTemplateCallbackCustom(simpleCallbackCustom);
}

TEST(EsScopeTemplateCustom) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  {
    EsScopeTemplate scope;
    CHECK_EQ(scope.self(), nullptr);
  }

  {
    auto v8Isolate = env->GetIsolate();
    auto v8Context = v8Isolate->GetCurrentContext();
    auto lwIsolate = IsolateWrap::fromV8(v8Isolate);
    auto lwContext = IsolateWrap::fromV8(v8Isolate)->GetCurrentContext();

    {
      EsScopeTemplate scope(v8Context);
      CHECK_EQ(scope.self(), nullptr);
      CHECK_EQ(scope.context(), lwContext->get());
      CHECK_EQ(scope.v8Isolate(), v8Isolate);
    }

    {
      EsScopeFunctionTemplate scope(v8Context, nullptr);
      CHECK_EQ(scope.context(), lwContext->get());
      CHECK_EQ(scope.v8Isolate(), v8Isolate);
    }

    {
      EsScopeObjectTemplate scope(v8Context, nullptr);
      CHECK_EQ(scope.context(), lwContext->get());
      CHECK_EQ(scope.v8Isolate(), v8Isolate);
    }
  }
}

TEST(EsScopeCustom) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  {
    EsScope scope;
    CHECK_EQ(scope.self(), nullptr);
  }

  {
    auto v8Isolate = env->GetIsolate();
    auto v8Context = v8Isolate->GetCurrentContext();
    auto lwIsolate = IsolateWrap::fromV8(v8Isolate);
    auto lwContext = IsolateWrap::fromV8(v8Isolate)->GetCurrentContext();

    EsScope scope(v8Isolate);
    CHECK_EQ(scope.self(), nullptr);
    CHECK_EQ(scope.context(), lwContext->get());
    CHECK_EQ(scope.v8Isolate(), v8Isolate);
  }

  {
    auto v8Isolate = env->GetIsolate();
    auto lwContext = IsolateWrap::fromV8(v8Isolate)->GetCurrentContext();
    EscargotShim::EsScope scope(v8Isolate);
    CHECK_EQ(scope.context(), lwContext->get());

    auto esValue = ValueRef::create(-1);
    uint32_t index = ValueRef::InvalidIndex32Value;
    auto r = Evaluator::execute(
        scope.context(),
        [](ExecutionStateRef* state, ValueRef* self, uint32_t* index) {
          *index = self->toIndex32(state);
          return ValueRef::create(*index);
        },
        esValue,
        &index);

    CHECK_EQ(ValueRef::InvalidIndex32Value, index);
  }
}

TEST(ExtensionCustom) {
  auto optionsBackup = EscargotShim::Global::flags()->get();

  {
    LocalContext env;
    v8::Isolate* v8Isolate = env->GetIsolate();
    v8::HandleScope scope(v8Isolate);

    v8::TryCatch try_catch(v8Isolate);
    Local<String> s = CompileRun("externalizeString").As<String>();
    CHECK(try_catch.HasCaught());

    s = CompileRun("isOneByteString").As<String>();
    CHECK(try_catch.HasCaught());

    s = CompileRun("x").As<String>();
    CHECK(try_catch.HasCaught());
  }

  {
    EscargotShim::Global::flags()->add(Flag::Type::ExposeExternalizeString);

    LocalContext env;
    v8::Isolate* v8Isolate = env->GetIsolate();
    v8::HandleScope scope(v8Isolate);

    v8::TryCatch try_catch(v8Isolate);
    Local<String> s = CompileRun("externalizeString('ok')").As<String>();
    CHECK(!try_catch.HasCaught());

    char buf[10] = {0};
    int len = s->WriteUtf8(v8Isolate, buf);
    CHECK_EQ(0, strcmp(buf, "ok"));

    Local<Boolean> b = CompileRun("isOneByteString('ok')").As<Boolean>();
    CHECK(!try_catch.HasCaught());
    CHECK(b->Value());

    Local<Integer> r = CompileRun("x()").As<Integer>();
    CHECK(!try_catch.HasCaught());
    CHECK_EQ(r->Value(), 1);
  }

  EscargotShim::Global::flags()->set(optionsBackup);
}

class NativeFunctionExtensionCustom : public Extension {
 public:
  NativeFunctionExtensionCustom(const char* name,
                                const char* source,
                                v8::FunctionCallback fun = &Echo)
      : Extension(name, source), function_(fun) {}

  v8::Local<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) override {
    return v8::FunctionTemplate::New(isolate, function_);
  }

  static void Echo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() >= 1) args.GetReturnValue().Set(args[0]);
  }

 private:
  v8::FunctionCallback function_;
};

TEST(NativeFunctionDeclarationCustom) {
  v8::HandleScope handle_scope(CcTest::isolate());
  const char* name = "nativedecl";
  v8::RegisterExtension(std::make_unique<NativeFunctionExtensionCustom>(
      name, "native function xyz();"));
  const char* extension_names[] = {name};
  v8::ExtensionConfiguration extensions(1, extension_names);
  v8::Local<Context> context = Context::New(CcTest::isolate(), &extensions);
  Context::Scope lock(context);
  v8::Local<Value> result = CompileRun("xyz(42);");
  CHECK(result->Equals(context, v8::Integer::New(CcTest::isolate(), 42))
            .FromJust());
}

static void WeakApiCallbackCustom(
    const v8::WeakCallbackInfo<Persistent<v8::Object>>& data) {
  data.GetParameter()->Reset();
  delete data.GetParameter();
}

TEST(WeakCallbackApiCustom) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  i::GlobalHandles* globals =
      reinterpret_cast<i::Isolate*>(isolate)->global_handles();

  size_t initial_handles = globals->handles_count();
  {
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    CHECK(
        obj->Set(context.local(), v8_str("key"), v8::Integer::New(isolate, 231))
            .FromJust());
    v8::Persistent<v8::Object>* handle =
        new v8::Persistent<v8::Object>(isolate, obj);
    handle->SetWeak<v8::Persistent<v8::Object>>(
        handle, WeakApiCallbackCustom, v8::WeakCallbackType::kParameter);
  }
  CcTest::PreciseCollectAllGarbage(isolate);
  // Verify disposed.
  CHECK_EQ(initial_handles, globals->handles_count());
}

TEST(StackTraceCustom) {
  LocalContext context;
  Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::TryCatch try_catch(isolate);
  std::string source = "var a;"
                       "function f() {"
                       "  throw new Error();"
                       "};"
                       "try { f(); }"
                       "catch (e) { a = e.stack;}";
  v8::Local<Value> r = CompileRun(source.c_str());
  CHECK(!try_catch.HasCaught());

  std::string exception = *v8::String::Utf8Value(isolate, r);
  CHECK(exception.find("Error") != std::string::npos);
  CHECK(exception.find("Error") == 0);
  CHECK(exception.find("at") != std::string::npos);
  CHECK(exception.find("at") > exception.find("Error"));
  CHECK(exception.find("f") != std::string::npos);
  CHECK(exception.find("f") > exception.find("at"));
}

static int prepareStackTraceCallbackCount = 0;
static MaybeLocal<Value> prepareStackTraceCallback(Local<Context> context,
                                                   Local<Value> error,
                                                   Local<Array> sites) {
  prepareStackTraceCallbackCount++;
  return v8_str("");
}

TEST(StackTracePrepareStackTraceCallbackCustom) {
  LocalContext context;
  Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  isolate->SetPrepareStackTraceCallback(prepareStackTraceCallback);

  {
    v8::TryCatch try_catch(isolate);
    std::string source = "function f() {"
                         "  throw new Error();"
                         "};";
    v8::Local<Value> r = CompileRun(source.c_str());
    CHECK(!try_catch.HasCaught());
    std::string exception = *v8::String::Utf8Value(isolate, r);
    CHECK(exception == "undefined");
    CHECK(prepareStackTraceCallbackCount == 0);
  }

  {
    v8::TryCatch try_catch(isolate);
    std::string source = "var a;"
                         "function f() {"
                         "  throw new Error();"
                         "};"
                         "try { f(); }"
                         "catch (e) { a = e.stack;}";
    v8::Local<Value> r = CompileRun(source.c_str());
    CHECK(!try_catch.HasCaught());
    std::string exception = *v8::String::Utf8Value(isolate, r);
    CHECK(exception != "undefined");
    CHECK(prepareStackTraceCallbackCount > 0);
  }

  prepareStackTraceCallbackCount = 0;
}

TEST(GlobalObjectInternalFieldsCustom) {
  v8::Isolate* isolate = CcTest::isolate();
  v8::HandleScope scope(isolate);
  Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);
  global_template->SetInternalFieldCount(1);
  LocalContext env(nullptr, global_template);
  v8::Local<v8::Object> global_proxy = env->Global();
  v8::Local<v8::Object> global = global_proxy->GetPrototype().As<v8::Object>();
  CHECK_EQ(1, global->InternalFieldCount());
  CHECK(global->GetInternalField(0)->IsUndefined());
  global->SetInternalField(0, v8_num(17));
  CHECK_EQ(17, global->GetInternalField(0)->Int32Value(env.local()).FromJust());
}

THREADED_TEST(GlobalObjectHasRealIndexedPropertyCustom) {
  LocalContext env;
  v8::HandleScope scope(CcTest::isolate());

  v8::Local<v8::Object> global = env->Global();
  CHECK(global->Set(env.local(), 0, v8_str("value")).FromJust());
  CHECK(global->HasRealIndexedProperty(env.local(), 0).FromJust());

  CHECK(global->Set(env.local(), 1, v8_str("value1")).FromJust());
  CHECK(global->HasRealIndexedProperty(env.local(), 1).FromJust());

  CHECK(!global->HasRealIndexedProperty(env.local(), 2).FromJust());
}

THREADED_TEST(GlobalObjectHasRealNamedPropertyCustom) {
  LocalContext env;
  v8::HandleScope scope(CcTest::isolate());

  v8::Local<v8::Object> global = env->Global();
  CHECK(global->Set(env.local(), v8_str("foo"), v8_str("fooValue")).FromJust());
  CHECK(global->HasRealNamedProperty(env.local(), v8_str("foo")).FromJust());

  CHECK(global->Set(env.local(), v8_str("bar"), v8_str("barValue")).FromJust());
  CHECK(global->HasRealNamedProperty(env.local(), v8_str("bar")).FromJust());

  CHECK(!global->HasRealNamedProperty(env.local(), v8_str("thisKeyNotExist"))
             .FromJust());
}

THREADED_TEST(BindingDemoCustom1) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  Local<Value> value = CompileRun("function f() { return 10; } f();");
  CHECK_EQ(10, value->Int32Value(context.local()).FromJust());
}

static void BindingDemoCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  args.GetReturnValue().Set(v8_num(20));
}

THREADED_TEST(BindingDemoCustom2) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  Local<Object> obj = Object::New(context->GetIsolate());
  Local<Function> function =
      Function::New(context.local(), BindingDemoCallback).ToLocalChecked();

  CHECK(obj->Set(context.local(), v8_str("f"), function).FromJust());
  CHECK(context->Global()->Set(context.local(), v8_str("o"), obj).FromJust());

  Local<Value> value = CompileRun("o.f();");
  CHECK_EQ(20, value->Int32Value(context.local()).FromJust());
}

TEST(DISABLED_NativeFunctionDeclarationErrorCustom) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  const char* name = "nativedeclerr";
  // Syntax error in extension code.
  v8::RegisterExtension(std::make_unique<NativeFunctionExtensionCustom>(
      name, "native\n function abcd();"));
  const char* extension_names[] = {name};
  v8::ExtensionConfiguration extensions(1, extension_names);
  v8::Local<Context> context1 = Context::New(isolate, &extensions);
  Context::Scope lock(context1);

  v8::Local<Value> result =
      CompileRun("var a; try { abcd(10); } catch (e) { a = e.stack; }");
  std::string exception = *v8::String::Utf8Value(isolate, result);
  CHECK(exception.find("ReferenceError") != std::string::npos);
}
