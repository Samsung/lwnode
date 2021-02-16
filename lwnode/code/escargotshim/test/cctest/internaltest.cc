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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cctest.h"
#include "include/v8.h"
using namespace v8;

TEST(CreateContextAndScope) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    Local<Context> context = Context::New(isolate);
    NESCARGOT_ASSERT(*context != nullptr);

    Context::Scope context_scope(context);
  }
  isolate->Dispose();
  CHECK(true);
}

TEST(CreateValuesAndCastLocalToEscargot) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    Local<Context> context = Context::New(isolate);
    CHECK(*context != nullptr);

    Context::Scope context_scope(context);

    // create String
    Local<String> source =
        String::NewFromUtf8(
            isolate, "'Hello' + ', World!'", NewStringType::kNormal)
            .ToLocalChecked();

    // Cast Local<String> to JsValueRef
    // JsValueRef v1 = reinterpre_cast<JsValueRef>(*source);
    // JsValueRef v2 = reinterpre_cast<Escargot::ValueRef*>(*source);
    JsValueRef valueRef = source->asJsValueRef();
    CHECK(valueRef->isString());  // call Escargot public function

    // Cast Local<Context> to JsContextRef
    // JsContextRef v1 = reinterpre_cast<JsContextRef>(*context);
    // JsContextRef v2 = reinterpre_cast<Escargot::ContextRef*>(*context);
    // JsContextRef contextRef =
    // EscargotShim::ContextShim::GetJsContextRef(*context);

    // create Number
    Local<Number> local1 = Number::New(isolate, 0.1234f);
    JsNumberRef ref1 = local1->asJsValueRef();
    CHECK(ref1->isNumber());
    CHECK(local1->Value() == ref1->asNumber());

    // contextRef->destroy();  // call Escargot public function
  }
  isolate->Dispose();
}

TEST(ValueToUtf8String) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);

    const char* str = "tc_value_to_utf8string";
    Local<Value> result =
        String::NewFromUtf8(isolate, str, NewStringType::kNormal)
            .ToLocalChecked();

    String::Utf8Value utf8(result);
    CHECK_STR(str, *utf8);
  }
  isolate->Dispose();
}

TEST(Primitive) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope scope(isolate);

    Local<Context> context = Context::New(isolate);
    ASSERT_TRUE(*context != nullptr);

    Context::Scope context_scope(context);

    v8::Local<v8::Primitive> undef = v8::Undefined(isolate);
    CHECK(!undef.IsEmpty());
    CHECK(undef->IsUndefined());

    v8::Local<v8::Boolean> t = v8::True(isolate);
    CHECK(t->Value());
    CHECK(t->IsTrue());

    v8::Local<v8::Boolean> f = v8::False(isolate);
    CHECK(!f->Value());
    CHECK(f->IsFalse());

    v8::Local<v8::Primitive> n = v8::Null(isolate);
    CHECK(n->IsNull());
  }
  isolate->Dispose();
}

TEST(SymbolPropertiesInternal) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::Object> obj = v8::Object::New(isolate);
  v8::Local<v8::Symbol> sym1 = v8::Symbol::New(isolate);
  v8::Local<v8::Symbol> sym2 = v8::Symbol::New(isolate, v8_str("my-symbol"));
  v8::Local<v8::Symbol> sym3 = v8::Symbol::New(isolate, v8_str("sym3"));
  UNUSED(sym3);

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
  CHECK(sym2->Name()->Equals(env.local(), v8_str("my-symbol")).FromJust());
}

bool functionTemplateCallbackFlag = false;
static void functionTemplateCallback(
    const v8::FunctionCallbackInfo<Value>& info) {
  functionTemplateCallbackFlag = true;
  CHECK_EQ(info.IsConstructCall(), false);
  info.GetReturnValue().Set(v8_num(22));
}

TEST(CallFunctionTemplate) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> fun_templ =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback);
  Local<Function> fun = fun_templ->GetFunction(env.local()).ToLocalChecked();

  CHECK(env->Global()->Set(env.local(), v8_str("obj"), fun).FromJust());
  CompileRun("var funtionTemplateTest1 = obj();");
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
  int arg = info[0]->IntegerValue(context).FromJust();
  CHECK_EQ(25, arg);
  CHECK_EQ(info.IsConstructCall(), true);

  Local<Object>::Cast(info.This())
      ->Set(context, v8_str("num"), v8_num(arg))
      .FromJust();
}

static void setNumberCallback(const v8::FunctionCallbackInfo<Value>& info) {
  auto context = info.GetIsolate()->GetCurrentContext();
  int arg = info[0]->IntegerValue(context).FromJust();
  Local<Object>::Cast(info.This())
      ->Set(context, v8_str("num"), v8_num(arg))
      .FromJust();
}

TEST(CallFunctionTemplateCons) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> fun_templ =
      v8::FunctionTemplate::New(isolate, functionTemplateConsCallback);

  Local<v8::FunctionTemplate> setNumberFn =
      v8::FunctionTemplate::New(isolate, setNumberCallback);
  fun_templ->PrototypeTemplate()->Set(v8_str("setNumber"), setNumberFn);

  Local<Function> fun = fun_templ->GetFunction(env.local()).ToLocalChecked();
  CHECK(env->Global()->Set(env.local(), v8_str("ClassA"), fun).FromJust());
  CompileRun("ClassA.prototype.getNumber = function(){return this.num;};"
             "var aObject = new ClassA(25);"
             "console.log(aObject);");
  CHECK(functionTemplateConsCallbackFlag);
  ExpectInt32("aObject.getNumber()", 25);
  ExpectInt32("aObject.num", 25);
}

TEST(FunctionNewInstance) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> templ2 =
      v8::FunctionTemplate::New(isolate, functionTemplateConsCallback);
  int inputNum = 25;
  auto classObj2 = templ2->GetFunction(env.local()).ToLocalChecked();
  CHECK(env->Global()
            ->Set(env.local(), v8_str("ClassObj2"), classObj2)
            .FromJust());
  CompileRun("ClassObj2.prototype.getNum = function(){return this.num;};");

  v8::Local<Value> args1[] = {v8_num(inputNum)};
  Local<v8::Object> obj2 =
      classObj2->NewInstance(env.local(), 1, args1).ToLocalChecked();
  CHECK(env->Global()->Set(env.local(), v8_str("obj2"), obj2).FromJust());
  ExpectInt32("obj2.num", inputNum);
  ExpectInt32("obj2.getNum()", inputNum);
}

TEST(ExternalInternal) {
  LocalContext env;
  v8::HandleScope scope(CcTest::isolate());

  int x = 3;
  Local<v8::External> ext = v8::External::New(CcTest::isolate(), &x);

  CHECK(env->Global()->Set(env.local(), v8_str("ext"), ext).FromJust());

  Local<Value> reext_obj = CompileRun("this.ext");
  v8::Local<v8::External> reext = reext_obj.As<v8::External>();
  int* ptr = static_cast<int*>(reext->Value());
  CHECK_EQ(3, x);
  *ptr = 10;
  CHECK_EQ(x, 10);

  // Make sure unaligned pointers are wrapped properly.
  // char* data = i::StrDup("0123456789");
  char data[] = "0123456789";
  Local<v8::Value> zero = v8::External::New(CcTest::isolate(), &data[0]);
  Local<v8::Value> one = v8::External::New(CcTest::isolate(), &data[1]);
  Local<v8::Value> two = v8::External::New(CcTest::isolate(), &data[2]);
  Local<v8::Value> three = v8::External::New(CcTest::isolate(), &data[3]);

  char* char_ptr = reinterpret_cast<char*>(v8::External::Cast(*zero)->Value());
  CHECK_EQ('0', *char_ptr);
  char_ptr = reinterpret_cast<char*>(v8::External::Cast(*one)->Value());
  CHECK_EQ('1', *char_ptr);
  char_ptr = reinterpret_cast<char*>(v8::External::Cast(*two)->Value());
  CHECK_EQ('2', *char_ptr);
  char_ptr = reinterpret_cast<char*>(v8::External::Cast(*three)->Value());
  CHECK_EQ('3', *char_ptr);
}

TEST(PrivatePropertiesInternal) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::Object> obj = v8::Object::New(isolate);
  UNUSED(obj);
  v8::Local<v8::Private> priv1 = v8::Private::New(isolate);
  UNUSED(priv1);
  v8::Local<v8::Private> priv2 =
      v8::Private::New(isolate, v8_str("my-private"));

  // CcTest::CollectAllGarbage(i::Heap::kFinalizeIncrementalMarkingMask);

  CHECK(priv2->Name()
            ->Equals(env.local(),
                     v8::String::NewFromUtf8(
                         isolate, "my-private", v8::NewStringType::kNormal)
                         .ToLocalChecked())
            .FromJust());

  // Make sure delete of a non-existent private symbol property works.
  // obj->DeletePrivate(env.local(), priv1).FromJust();
  // CHECK(!obj->HasPrivate(env.local(), priv1).FromJust());

  // CHECK(obj->SetPrivate(env.local(), priv1, v8::Integer::New(isolate, 1503))
  //           .FromJust());
  // CHECK(obj->HasPrivate(env.local(), priv1).FromJust());
  // CHECK_EQ(1503, obj->GetPrivate(env.local(), priv1)
  //                    .ToLocalChecked()
  //                    ->Int32Value(env.local())
  //                    .FromJust());
  // CHECK(obj->SetPrivate(env.local(), priv1, v8::Integer::New(isolate, 2002))
  //           .FromJust());
  // CHECK(obj->HasPrivate(env.local(), priv1).FromJust());
  // CHECK_EQ(2002, obj->GetPrivate(env.local(), priv1)
  //                    .ToLocalChecked()
  //                    ->Int32Value(env.local())
  //                    .FromJust());
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

THREADED_TEST(ArrayBufferAndTypedArray) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);

  const int kHeapObjectTag = 1;
  uint8_t* store_ptr = reinterpret_cast<uint8_t*>(kHeapObjectTag);
  Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, store_ptr, 8);
  Local<v8::Uint32Array> ta = v8::Uint32Array::New(ab, 1, 1);

  CHECK_EQ(ab->GetContents().Data(), store_ptr);
  CHECK_EQ(ab->GetContents().ByteLength(), 8);
  CHECK_EQ(ta->Length(), 1);
  CHECK_EQ(ta->ByteLength(), 4);
  CHECK_EQ(ta->ByteOffset(), 1);
  CHECK_EQ(ta->Buffer()->GetContents().Data(), store_ptr);

  CHECK_EQ(ta->IsTypedArray(), true);
  CHECK_EQ(ab->IsTypedArray(), false);
}

static void functionTemplateCallback1(
    const v8::FunctionCallbackInfo<Value>& info) {
  info.GetReturnValue().Set(v8_str("callback1"));
}

static void functionTemplateCallback2(
    const v8::FunctionCallbackInfo<Value>& info) {
  info.GetReturnValue().Set(v8_str("callback2"));
}

TEST(SetMethod) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::Object> that = v8::Object::New(isolate);
  v8::Local<v8::Function> function =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback1)
          ->GetFunction(env.local())
          .ToLocalChecked();
  that->Set(env.local(), v8_str("f"), function).FromJust();

  CHECK(env->Global()->Set(env.local(), v8_str("that"), that).FromJust());
  ExpectString("that.f()", "callback1");
}

TEST(SetProtoMethod) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> that =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback1);

  v8::Local<v8::Signature> signature = v8::Signature::New(isolate, that);
  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(
      isolate, functionTemplateCallback2, Local<Value>(), signature);

  that->PrototypeTemplate()->Set(v8_str("f"), t);

  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("that"),
                  that->GetFunction(env.local()).ToLocalChecked())
            .FromJust());
  CompileRun("var a = new that(); console.log(a);");
  ExpectString("a.f()", "callback2");
}

TEST(SetTemplateMethod) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);
  Local<v8::FunctionTemplate> that =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback1);

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, functionTemplateCallback2);

  that->Set(v8_str("f"), t);
  CHECK(env->Global()
            ->Set(env.local(),
                  v8_str("that"),
                  that->GetFunction(env.local()).ToLocalChecked())
            .FromJust());
  CompileRun("var a = new that(); console.log(a);");

  // Todo: find out which test is right.
  // ExpectString("a.f()", "callback2");
  ExpectString("that.f()", "callback2");
}

static int functionTemplateAccessorCount = 0;
static void FunctionTemplateAccessorCallback(
    const v8::FunctionCallbackInfo<Value>& info) {}

static void ProtoGetter(Local<String> property,
                        const PropertyCallbackInfo<Value>& info) {
  int count = info.This()->InternalFieldCount();
  CHECK_GT(count, 0);
  functionTemplateAccessorCount++;
  CHECK_EQ(info.Data()
               ->ToInt32(info.GetIsolate()->GetCurrentContext())
               .ToLocalChecked()
               ->Value(),
           22);
  info.GetReturnValue().Set(v8_num(0));
}

static void InsGetter(Local<String> property,
                      const PropertyCallbackInfo<Value>& info) {
  int count = info.This()->InternalFieldCount();
  CHECK_GT(count, 0);
  functionTemplateAccessorCount++;
  CHECK_EQ(info.Data()
               ->ToInt32(info.GetIsolate()->GetCurrentContext())
               .ToLocalChecked()
               ->Value(),
           22);
  info.GetReturnValue().Set(v8_num(0));
}

TEST(FunctionTemplateAccessor) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, FunctionTemplateAccessorCallback);

  t->InstanceTemplate()->SetInternalFieldCount(1);
  const PropertyAttribute attributes =
      static_cast<PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
  UNUSED(attributes);

  Local<Value> count = v8_num(22);

  t->PrototypeTemplate()->SetAccessor(
      v8_str("test1"),
      ProtoGetter,
      nullptr,
      Handle<Value>(count),
      DEFAULT,
      None,
      Handle<AccessorSignature>(AccessorSignature::New(isolate, t)));

  t->InstanceTemplate()->SetAccessor(
      v8_str("test2"),
      InsGetter,
      nullptr,
      Handle<Value>(count),
      DEFAULT,
      None,
      Handle<AccessorSignature>(AccessorSignature::New(isolate, t)));

  Local<Object> o = t->GetFunction(env.local())
                        .ToLocalChecked()
                        ->NewInstance(env.local())
                        .ToLocalChecked();

  o->Get(env.local(), v8_str("test2"));
  o->Get(env.local(), v8_str("test1"));

  CHECK_EQ(functionTemplateAccessorCount, 2);
}

TEST(WriteOneByte) {
  LocalContext context;
  v8::HandleScope scope(context->GetIsolate());
  v8::Local<String> str = v8_str("abcde");

  char buf[100];
  int len;
  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 0, 4);
  CHECK_EQ(4, len);
  CHECK_EQ(0, strncmp("abcd\1", buf, 5));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 0, 5);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strncmp("abcde\1", buf, 6));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 0, 6);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 4, -1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 4, 6);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 4, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("e\1", buf, 2));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(reinterpret_cast<uint8_t*>(buf), 3, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("d\1", buf, 2));
}

int GetUtf8Length(Local<String> str) {
  int len = str->Utf8Length();
  if (len < 0) {
    NESCARGOT_LOG_ERROR("ERROR\n");
    //   i::Handle<i::String> istr(v8::Utils::OpenHandle(*str));
    //   i::String::Flatten(istr);
    //   len = str->Utf8Length();
  }
  return len;
}

THREADED_TEST(StringWrite) {
  LocalContext context;
  v8::HandleScope scope(context->GetIsolate());

  v8::Local<String> str = v8_str("abcde");
  UNUSED(str);

  // abc<Icelandic eth><Unicode snowman>.
  v8::Local<String> str2 = v8_str("abc\303\260\342\230\203");
  v8::Local<String> str3 =
      v8::String::NewFromUtf8(
          context->GetIsolate(), "abc\0def", v8::NewStringType::kNormal, 7)
          .ToLocalChecked();
  UNUSED(str3);

  // "ab" + lead surrogate + "cd" + trail surrogate + "ef"
  uint16_t orphans[8] = {0x61, 0x62, 0xd800, 0x63, 0x64, 0xdc00, 0x65, 0x66};
  v8::Local<String> orphans_str =
      v8::String::NewFromTwoByte(
          context->GetIsolate(), orphans, v8::NewStringType::kNormal, 8)
          .ToLocalChecked();
  UNUSED(orphans_str);

  // single lead surrogate
  uint16_t lead[1] = {0xd800};
  v8::Local<String> lead_str =
      v8::String::NewFromTwoByte(
          context->GetIsolate(), lead, v8::NewStringType::kNormal, 1)
          .ToLocalChecked();
  UNUSED(lead_str);

  // single trail surrogate
  uint16_t trail[1] = {0xdc00};
  UNUSED(trail);
  v8::Local<String> trail_str =
      v8::String::NewFromTwoByte(
          context->GetIsolate(), trail, v8::NewStringType::kNormal, 1)
          .ToLocalChecked();
  UNUSED(trail_str);

  // surrogate pair
  uint16_t pair[2] = {0xd800, 0xdc00};
  v8::Local<String> pair_str =
      v8::String::NewFromTwoByte(
          context->GetIsolate(), pair, v8::NewStringType::kNormal, 2)
          .ToLocalChecked();

  const int kStride = 4;  // Must match stride in for loops in JS below.
  CompileRun("var left = '';"
             "for (var i = 0; i < 0xd800; i += 4) {"
             "  left = left + String.fromCharCode(i);"
             "}");
  CompileRun("var right = '';"
             "for (var i = 0; i < 0xd800; i += 4) {"
             "  right = String.fromCharCode(i) + right;"
             "}");
  v8::Local<v8::Object> global = context->Global();
  Local<String> left_tree = global->Get(context.local(), v8_str("left"))
                                .ToLocalChecked()
                                .As<String>();
  Local<String> right_tree = global->Get(context.local(), v8_str("right"))
                                 .ToLocalChecked()
                                 .As<String>();

  CHECK_EQ(5, str2->Length());
  CHECK_EQ(0xd800 / kStride, left_tree->Length());
  CHECK_EQ(0xd800 / kStride, right_tree->Length());

  char buf[100];
  UNUSED(buf);
  char utf8buf[0xd800 * 3];
  uint16_t wbuf[100];
  UNUSED(wbuf);
  int len;
  int charlen;

  // str2: "abcÃ°â", len 8
  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, sizeof(utf8buf), &charlen);
  CHECK_EQ(9, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "abc\303\260\342\230\203"));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 8, &charlen);
  CHECK_EQ(8, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\342\230\203\1", 9));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 7, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 6, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 5, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 4, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\1", 4));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 3, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\1", 4));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(utf8buf, 2, &charlen);
  CHECK_EQ(2, len);
  CHECK_EQ(2, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "ab\1", 3));

  // TODO: check converting utf8 from a string created with two bytes string.
  // // allow orphan surrogates by default
  // memset(utf8buf, 0x1, 1000);
  // len = orphans_str->WriteUtf8(utf8buf, sizeof(utf8buf), &charlen);
  // CHECK_EQ(13, len);
  // CHECK_EQ(8, charlen);
  // CHECK_EQ(0, strcmp(utf8buf, "ab\355\240\200cd\355\260\200ef"));

  // do not replace / write anything if surrogate pair does not fit the
  // buffer space
  memset(utf8buf, 0x1, 1000);
  len = pair_str->WriteUtf8(utf8buf, 3, &charlen, String::REPLACE_INVALID_UTF8);
  CHECK_EQ(0, len);
  CHECK_EQ(0, charlen);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = GetUtf8Length(left_tree);
  int utf8_expected =
      (0x80 + (0x800 - 0x80) * 2 + (0xd800 - 0x800) * 3) / kStride;
  CHECK_EQ(utf8_expected, len);
  len = left_tree->WriteUtf8(utf8buf, utf8_expected, &charlen);
  CHECK_EQ(utf8_expected, len);
  CHECK_EQ(0xd800 / kStride, charlen);
  CHECK_EQ(0xed, static_cast<unsigned char>(utf8buf[utf8_expected - 3]));
  CHECK_EQ(0x9f, static_cast<unsigned char>(utf8buf[utf8_expected - 2]));
  CHECK_EQ(0xc0 - kStride,
           static_cast<unsigned char>(utf8buf[utf8_expected - 1]));
  CHECK_EQ(1, utf8buf[utf8_expected]);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = GetUtf8Length(right_tree);
  CHECK_EQ(utf8_expected, len);
  len = right_tree->WriteUtf8(utf8buf, utf8_expected, &charlen);
  CHECK_EQ(utf8_expected, len);
  CHECK_EQ(0xd800 / kStride, charlen);
  CHECK_EQ(0xed, static_cast<unsigned char>(utf8buf[0]));
  CHECK_EQ(0x9f, static_cast<unsigned char>(utf8buf[1]));
  CHECK_EQ(0xc0 - kStride, static_cast<unsigned char>(utf8buf[2]));
  CHECK_EQ(1, utf8buf[utf8_expected]);
}

THREADED_TEST(GlobalPrivatesInternal) {
  // i::FLAG_allow_natives_syntax = true;
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::Object> obj = v8::Object::New(isolate);

  v8::Local<String> name = v8_str("my-private");
  v8::Local<v8::Private> glob1 = v8::Private::ForApi(isolate, name);
  v8::Local<v8::Private> glob2 = v8::Private::ForApi(isolate, name);
  v8::Local<v8::Private> normal = v8::Private::New(isolate, name);

  // set `my-private:3` to obj
  CHECK(obj->SetPrivate(env.local(), glob1, v8::Integer::New(isolate, 3))
            .FromJust());

  // check `my-private` in obj
  CHECK(obj->HasPrivate(env.local(), glob2).FromJust());

  // check `my-private`:Symbol(not global) in obj
  CHECK(!obj->HasPrivate(env.local(), normal).FromJust());

  // CompileRun("var intern = %CreatePrivateSymbol('my-private')");
  // v8::Local<Value> intern =
  // env->Global()->Get(env.local(), v8_str("intern")).ToLocalChecked();
  // CHECK(!obj->Has(env.local(), intern).FromJust());
}

THREADED_TEST(StringConcatInternal) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    Local<Context> context = Context::New(isolate);
    CHECK(*context != nullptr);

    Context::Scope context_scope(context);

    // create String
    Local<String> source1 =
        String::NewFromUtf8(isolate, "Hello", NewStringType::kNormal)
            .ToLocalChecked();

    Local<String> source2 =
        String::NewFromUtf8(isolate, ", World!", NewStringType::kNormal)
            .ToLocalChecked();

    Local<String> result = String::Concat(source1, source2);

    String::Utf8Value utf8(result);
    CHECK_STR("Hello, World!", *utf8);
  }
  isolate->Dispose();
}

THREADED_TEST(TestObjectInternal) {
  Isolate::CreateParams create_params;
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    CHECK(*context != nullptr);

    // Test GetPrototype
    Context::Scope context_scope(context);
    Local<v8::Array> value =
        CompileRun("[\"a\", \"b\", \"c\"]").As<v8::Array>();
    CHECK(value->GetPrototype()->StrictEquals(CompileRun("Array.prototype")));

    // Test GetOwnPropertyNames : ["0", "1", "2"]
    v8::Local<v8::Array> props1 =
        value->GetOwnPropertyNames(context).ToLocalChecked();
    CHECK_EQ(3u, props1->Length());

    // Test GetPropertyNames : ["0", "1", "2"]
    v8::Local<v8::Array> props2 =
        value.As<v8::Object>()->GetPropertyNames(context).ToLocalChecked();
    CHECK_EQ(3u, props2->Length());
  }
  isolate->Dispose();
}

THREADED_TEST(ObjectGetConstructorNameInternal) {
  v8::Isolate* isolate = CcTest::isolate();
  LocalContext context;
  v8::HandleScope scope(isolate);
  v8_compile("function Parent() {};"
             "function Child() {};"
             "Child.prototype = new Parent();"
             "Child.prototype.constructor = Child;"
             "var outer = { inner: (0, function() { }) };"
             "var p = new Parent();"
             "var c = new Child();"
             "var x = new outer.inner();"
             "var proto = Child.prototype;")
      ->Run(context.local())
      .ToLocalChecked();

  Local<v8::Value> p =
      context->Global()->Get(context.local(), v8_str("p")).ToLocalChecked();
  CHECK(p->IsObject() && p->ToObject(context.local())
                             .ToLocalChecked()
                             ->GetConstructorName()
                             ->Equals(context.local(), v8_str("Parent"))
                             .FromJust());

  Local<v8::Value> c =
      context->Global()->Get(context.local(), v8_str("c")).ToLocalChecked();
  CHECK(c->IsObject() && c->ToObject(context.local())
                             .ToLocalChecked()
                             ->GetConstructorName()
                             ->Equals(context.local(), v8_str("Child"))
                             .FromJust());

  // NOTE: Chrome shows x:"" and proto:"Child", but the original tc expects
  // "outer.inner" and "Parent" respectively
  Local<v8::Value> x =
      context->Global()->Get(context.local(), v8_str("x")).ToLocalChecked();
  CHECK(x->IsObject() && x->ToObject(context.local())
                             .ToLocalChecked()
                             ->GetConstructorName()
                             ->Equals(context.local(), v8_str(""))
                             .FromJust());

  Local<v8::Value> child_prototype =
      context->Global()->Get(context.local(), v8_str("proto")).ToLocalChecked();
  CHECK(child_prototype->IsObject() &&
        child_prototype->ToObject(context.local())
            .ToLocalChecked()
            ->GetConstructorName()
            ->Equals(context.local(), v8_str("Child"))
            .FromJust());
}

static void WeakApiCallback(
    const v8::WeakCallbackInfo<Persistent<v8::Object>>& data) {
  data.GetParameter()->Reset();
  delete data.GetParameter();
}

TEST(WeakCallbackApiInternal) {
  LocalContext context;
  v8::Isolate* isolate = context->GetIsolate();
  // i::GlobalHandles* globals =
  //     reinterpret_cast<i::Isolate*>(isolate)->global_handles();
  // int initial_handles = globals->global_handles_count();

  {
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    CHECK(
        obj->Set(context.local(), v8_str("key"), v8::Integer::New(isolate, 231))
            .FromJust());

    // Convert Local object -> Persitent object
    v8::Persistent<v8::Object>* handle =
        new v8::Persistent<v8::Object>(isolate, obj);

    handle->SetWeak<v8::Persistent<v8::Object>>(
        handle, WeakApiCallback, v8::WeakCallbackType::kParameter);
  }

  // reinterpret_cast<i::Isolate*>(isolate)->heap()->CollectAllGarbage(
  //     i::Heap::kAbortIncrementalMarkingMask,
  //     i::GarbageCollectionReason::kTesting);

  // Verify disposed.
  // CHECK_EQ(initial_handles, globals->global_handles_count());
}

TEST(PrimitiveArray) {
  v8::Isolate* isolate = CcTest::isolate();
  v8::HandleScope scope(isolate);
  LocalContext env;

  int length = 5;
  Local<v8::PrimitiveArray> array(v8::PrimitiveArray::New(isolate, 5));
  CHECK_EQ(length, array->Length());

  for (int i = 0; i < length; i++) {
    Local<v8::Primitive> item = array->Get(isolate, i);
    CHECK(item->IsUndefined());
  }

  Local<v8::Symbol> symbol(v8::Symbol::New(isolate));
  array->Set(isolate, 0, symbol);
  CHECK(array->Get(isolate, 0)->IsSymbol());
  // Local<v8::String> string = v8::String::NewFromUtf8Literal(
  //     isolate, "test", v8::NewStringType::kInternalized);
  Local<v8::String> string =
      v8::String::NewFromUtf8(isolate, "test", v8::NewStringType::kNormal, 4)
          .ToLocalChecked();

  array->Set(isolate, 1, string);
  CHECK(array->Get(isolate, 0)->IsSymbol());
  CHECK(array->Get(isolate, 1)->IsString());

  Local<v8::Number> num = v8::Number::New(env->GetIsolate(), 3.1415926);
  array->Set(isolate, 2, num);
  CHECK(array->Get(isolate, 0)->IsSymbol());
  CHECK(array->Get(isolate, 1)->IsString());
  CHECK(array->Get(isolate, 2)->IsNumber());

  v8::Local<v8::Boolean> f = v8::False(isolate);
  array->Set(isolate, 3, f);
  CHECK(array->Get(isolate, 0)->IsSymbol());
  CHECK(array->Get(isolate, 1)->IsString());
  CHECK(array->Get(isolate, 2)->IsNumber());
  CHECK(array->Get(isolate, 3)->IsBoolean());

  v8::Local<v8::Primitive> n = v8::Null(isolate);
  array->Set(isolate, 4, n);
  CHECK(array->Get(isolate, 0)->IsSymbol());
  CHECK(array->Get(isolate, 1)->IsString());
  CHECK(array->Get(isolate, 2)->IsNumber());
  CHECK(array->Get(isolate, 3)->IsBoolean());
  CHECK(array->Get(isolate, 4)->IsNull());
}
