/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __ESCARGOTSHIM_CCTEST__
#define __ESCARGOTSHIM_CCTEST__

#include "gtest/gtest.h"
#include "v8.h"

#define TEST_CAST_NAME CCTest

#define UNINITIALIZED_TEST(Name) GTEST_TEST(TEST_CAST_NAME, Name)

#define TEST(Name) TEST_F(InitializeTest, Name)

#define THREADED_TEST(Name) TEST_F(InitializeTest, Name)

#define THREADED_PROFILED_TEST(Name) TEST_F(InitializeTest, Name)

#define DISABLED_TEST(Name)

#define CHECK(condition) EXPECT_TRUE(condition)
#define CHECK_EQ(lhs, rhs) EXPECT_EQ(lhs, rhs)
#define CHECK_NE(lhs, rhs) EXPECT_NE(lhs, rhs)
#define CHECK_STR(lhs, rhs) EXPECT_STREQ(lhs, rhs)
#define CHECK_LE(lhs, rhs) EXPECT_LE(lhs, rhs)
#define CHECK_LT(lhs, rhs) EXPECT_LT(lhs, rhs)
#define CHECK_GE(lhs, rhs) EXPECT_GE(lhs, rhs)
#define CHECK_GT(lhs, rhs) EXPECT_GT(lhs, rhs)

class CcTest {
 public:
  static v8::Isolate* isolate();
  static v8::Isolate* disposeScope();
  static v8::Isolate* disposeIsolate();
  static v8::Context::Scope* contextScope_;

 private:
  static v8::Isolate* isolate_;
};

static inline v8::Local<v8::Value> v8_num(double x) {
  return v8::Number::New(v8::Isolate::GetCurrent(), x);
}

static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::NewFromUtf8(
             v8::Isolate::GetCurrent(), x, v8::NewStringType::kNormal)
      .ToLocalChecked();
}

static inline v8::Local<v8::String> v8_str(v8::Isolate* isolate,
                                           const char* x) {
  return v8::String::NewFromUtf8(isolate, x, v8::NewStringType::kNormal)
      .ToLocalChecked();
}

static inline v8::Local<v8::Symbol> v8_symbol(const char* name) {
  return v8::Symbol::New(v8::Isolate::GetCurrent(), v8_str(name));
}

static inline v8::Local<v8::Script> v8_compile(v8::Local<v8::String> x) {
  v8::Local<v8::Script> result;
  if (v8::Script::Compile(v8::Isolate::GetCurrent()->GetCurrentContext(), x)
          .ToLocal(&result)) {
    return result;
  }
  return v8::Local<v8::Script>();
}

static inline v8::Local<v8::Script> v8_compile(const char* x) {
  return v8_compile(v8_str(x));
}

static inline int32_t v8_run_int32value(v8::Local<v8::Script> script) {
  v8::Local<v8::Context> context = CcTest::isolate()->GetCurrentContext();
  return script->Run(context).ToLocalChecked()->Int32Value(context).FromJust();
}

static inline v8::Local<v8::Value> CompileRun(v8::Local<v8::String> source) {
  v8::Local<v8::Value> result;
  if (v8_compile(source)
          ->Run(v8::Isolate::GetCurrent()->GetCurrentContext())
          .ToLocal(&result)) {
    return result;
  }
  return v8::Local<v8::Value>();
}

static inline v8::Local<v8::Value> CompileRun(const char* source) {
  return CompileRun(v8_str(source));
}

static inline void ExpectString(const char* code, const char* expected) {
  v8::Local<v8::Value> result = CompileRun(code);
  CHECK(result->IsString());
  v8::String::Utf8Value utf8(result);
  CHECK_EQ(0, strcmp(expected, *utf8));
}

static inline void ExpectInt32(const char* code, int expected) {
  v8::Local<v8::Value> result = CompileRun(code);
  CHECK(result->IsInt32());
  CHECK_EQ(expected,
           result->Int32Value(v8::Isolate::GetCurrent()->GetCurrentContext())
               .FromJust());
}

static inline void ExpectBoolean(const char* code, bool expected) {
  v8::Local<v8::Value> result = CompileRun(code);
  CHECK(result->IsBoolean());
  CHECK_EQ(expected,
           result->BooleanValue(v8::Isolate::GetCurrent()->GetCurrentContext())
               .FromJust());
}

static inline void ExpectTrue(const char* code) {
  ExpectBoolean(code, true);
}

static inline void ExpectFalse(const char* code) {
  ExpectBoolean(code, false);
}

// static inline void ExpectObject(const char* code,
//                                 v8::Local<v8::Value> expected) {
//   v8::Local<v8::Value> result = CompileRun(code);
//   CHECK(result->SameValue(expected));
// }

static inline void ExpectUndefined(const char* code) {
  v8::Local<v8::Value> result = CompileRun(code);
  CHECK(result->IsUndefined());
}

static inline void ExpectNull(const char* code) {
  v8::Local<v8::Value> result = CompileRun(code);
  CHECK(result->IsNull());
}

static inline void CheckDoubleEquals(double expected, double actual) {
  const double kEpsilon = 1e-10;
  CHECK_LE(expected, actual + kEpsilon);
  CHECK_GE(expected, actual - kEpsilon);
}

// A LocalContext holds a reference to a v8::Context.
class LocalContext {
 public:
  LocalContext(v8::Isolate* isolate,
               v8::ExtensionConfiguration* extensions = 0,
               v8::Local<v8::ObjectTemplate> global_template =
                   v8::Local<v8::ObjectTemplate>(),
               v8::Local<v8::Value> global_object = v8::Local<v8::Value>()) {
    Initialize(isolate, extensions, global_template, global_object);
  }

  LocalContext(v8::ExtensionConfiguration* extensions = 0,
               v8::Local<v8::ObjectTemplate> global_template =
                   v8::Local<v8::ObjectTemplate>(),
               v8::Local<v8::Value> global_object = v8::Local<v8::Value>()) {
    Initialize(CcTest::isolate(), extensions, global_template, global_object);
  }

  virtual ~LocalContext();

  v8::Context* operator->() {
    return *reinterpret_cast<v8::Context**>(&context_);
  }
  v8::Context* operator*() { return operator->(); }
  bool IsReady() { return !context_.IsEmpty(); }

  v8::Local<v8::Context> local() {
    return v8::Local<v8::Context>::New(isolate_, context_);
  }

 private:
  void Initialize(v8::Isolate* isolate,
                  v8::ExtensionConfiguration* extensions,
                  v8::Local<v8::ObjectTemplate> global_template,
                  v8::Local<v8::Value> global_object);

  v8::Persistent<v8::Context> context_;
  v8::Isolate* isolate_;
};

class InitializeTest : public ::testing::Test {
 public:
  void SetUp();
  void TearDown();
};

#endif
