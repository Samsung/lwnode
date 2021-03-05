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

v8::Isolate* CcTest::isolate_ = nullptr;
v8::Context::Scope* CcTest::contextScope_ = nullptr;
static bool disable_automatic_dispose_ = false;
v8::Isolate::CreateParams create_params_;

// internals
#include "api/utils/flags.h"

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

v8::Isolate* CcTest::isolate() {
  if (isolate_ == nullptr) {
    create_params_.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    CcTest::isolate_ = v8::Isolate::New(create_params_);
  }
  isolate_->Enter();
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
    delete create_params_.array_buffer_allocator;
    isolate_ = nullptr;
  }
}

void CcTest::TearDown() {
  if (isolate_ != nullptr) {
    isolate_->Dispose();
    delete create_params_.array_buffer_allocator;
  }
}

static inline bool startsWith(const std::string& string,
                              const std::string& prefix) {
  return (string.size() >= prefix.size()) &&
         (string.compare(0, prefix.size(), prefix) == 0);
}

void InitializeTest::SetUp() {
}

void InitializeTest::TearDown() {
  CcTest::disposeScope();
  CcTest::disposeIsolate();
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

  v8::V8::Initialize();

  auto result = RUN_ALL_TESTS();

  CcTest::TearDown();
  if (!disable_automatic_dispose_) {
    v8::V8::Dispose();
  }
  v8::V8::ShutdownPlatform();

  return result;
}
