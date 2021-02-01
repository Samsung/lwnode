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

#include "cctest.h"

// internals
#include "api/flags.h"

LocalContext::~LocalContext() {
  v8::HandleScope scope(isolate_);
  v8::Local<v8::Context>::New(isolate_, context_)->Exit();
  context_.Reset();
}

void LocalContext::Initialize(v8::Isolate* isolate,
                              v8::ExtensionConfiguration* extensions,
                              v8::Local<v8::ObjectTemplate> global_template,
                              v8::Local<v8::Value> global_object) {
  isolate->Enter();
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context =
      v8::Context::New(isolate, extensions, global_template, global_object);

  context_.Reset(isolate, context);
  context->Enter();
  // We can't do this later perhaps because of a fatal error.
  isolate_ = isolate;
}

v8::Isolate* CcTest::isolate_ = nullptr;
v8::Context::Scope* CcTest::contextScope_ = nullptr;

v8::Isolate* CcTest::isolate() {
  if (isolate_ == nullptr) {
    v8::Isolate::CreateParams create_params;
    CcTest::isolate_ = v8::Isolate::New(create_params);
  }
  return isolate_;
}

void CcTest::disposeScope() {
  if (contextScope_ != nullptr) {
    delete contextScope_;
    contextScope_ = nullptr;
  }
}

void CcTest::disposeIsolate() {
  if (isolate_ != nullptr) {
    isolate_->Exit();
    isolate_->Dispose();
    isolate_ = nullptr;
  }
}

static inline bool startsWith(const std::string& string,
                              const std::string& prefix) {
  return (string.size() >= prefix.size()) &&
         (string.compare(0, prefix.size(), prefix) == 0);
}

void InitializeTest::SetUp() {
  // v8::V8::Initialize();
}

void InitializeTest::TearDown() {
  // CcTest::disposeScope();
  // CcTest::disposeIsolate();
  // v8::V8::Dispose();
}

int main(int argc, char* argv[]) {
  printf("============= Start EscargotShim Test ============= \n");

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);

    if (startsWith(arg, std::string("-f="))) {
      std::string f = std::string("*") + arg.substr(strlen("-f="));
      ::testing::GTEST_FLAG(filter) = f.c_str();
    } else if(startsWith(arg, std::string("--trace-gc"))) {
      EscargotShim::Flags::add(EscargotShim::FlagType::TraceGC);
    }
    else {
      printf("unknown options: %s\n", argv[i]);
    }
  }

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
