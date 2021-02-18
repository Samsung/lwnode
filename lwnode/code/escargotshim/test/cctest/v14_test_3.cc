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
               ->Utf8Length(isolate), 6);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Length(),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Length(), 2);

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
  CHECK_EQ(7, array->Get(context, 2)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  // Local<Value> obj = CompileRun("[1, 2, 3]");
  // Local<v8::Array> arr = obj.As<v8::Array>();
  Local<v8::Array> arr = v8::Array::New(context->GetIsolate());
  CHECK(arr->Set(context, 0, v8_num(1)).FromJust());
  CHECK(arr->Set(context, 1, v8_num(2)).FromJust());
  CHECK(arr->Set(context, 2, v8_num(3)).FromJust());
  CHECK_EQ(3u, arr->Length());
  CHECK_EQ(1, arr->Get(context, 0)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  CHECK_EQ(2, arr->Get(context, 1)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  CHECK_EQ(3, arr->Get(context, 2)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  array = v8::Array::New(context->GetIsolate(), 27);
  CHECK_EQ(27u, array->Length());
  array = v8::Array::New(context->GetIsolate(), -27);
  CHECK_EQ(0u, array->Length());

  std::vector<Local<Value>> vector = {v8_num(1), v8_num(2), v8_num(3)};
  array = v8::Array::New(context->GetIsolate(), vector.data(), vector.size());
  CHECK_EQ(vector.size(), array->Length());
  CHECK_EQ(1, arr->Get(context, 0)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  CHECK_EQ(2, arr->Get(context, 1)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
  CHECK_EQ(3, arr->Get(context, 2)
                  .ToLocalChecked()
                  ->Int32Value(context)
                  .FromJust());
}
