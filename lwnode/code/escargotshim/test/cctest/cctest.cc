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
#if defined(CCTEST_ENGINE_ESCARGOT)
#include "internal-api.h"
#endif
#include "libplatform/libplatform.h"

v8::Isolate* CcTest::isolate_ = nullptr;
v8::Context::Scope* CcTest::contextScope_ = nullptr;
v8::ArrayBuffer::Allocator* CcTest::allocator_ = nullptr;
static bool disable_automatic_dispose_ = false;
v8::Isolate::CreateParams create_params_;

static CcTest g_cctest;

LocalContext::~LocalContext() {
  v8::HandleScope scope(isolate_);
  v8::Local<v8::Context>::New(isolate_, context_)->Exit();
  context_.Reset();
}

void LocalContext::Initialize(v8::Isolate* isolate,
                              v8::ExtensionConfiguration* extensions,
                              v8::Local<v8::ObjectTemplate> global_template,
                              v8::Local<v8::Value> global_object) {
  v8::HandleScope scope(isolate);
  v8::Local<v8::Context> context =
      v8::Context::New(isolate, extensions, global_template, global_object);
  context_.Reset(isolate, context);
  context->Enter();
  // We can't do this later perhaps because of a fatal error.
  isolate_ = isolate;
}

CcTest::CcTest() {
  allocator_ = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

CcTest::~CcTest() {
  delete allocator_;
}

v8::Isolate* CcTest::isolate() {
  if (isolate_ == nullptr) {
    create_params_.array_buffer_allocator = allocator_;
    CcTest::isolate_ = v8::Isolate::New(create_params_);
#if defined(CCTEST_ENGINE_ESCARGOT)
    e::IsolateWrap::fromV8(isolate_)->lock_gc_release();
#endif
  }
  return isolate_;
}

v8::Local<v8::Object> CcTest::global() {
  return isolate()->GetCurrentContext()->Global();
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
#if defined(CCTEST_ENGINE_ESCARGOT)
    e::IsolateWrap::fromV8(isolate_)->unlock_gc_release();
    CcTest::CollectAllGarbage();
#endif
    isolate_ = nullptr;
  }
}

void CcTest::TearDown() {
  if (isolate_ != nullptr) {
    disposeIsolate();
    delete create_params_.array_buffer_allocator;
  }
}

void CcTest::CollectGarbage() {
#if defined(CCTEST_ENGINE_ESCARGOT)
  MemoryUtil::gc();
#endif
}

void CcTest::CollectAllGarbage(v8::Isolate* isolate) {
#if defined(CCTEST_ENGINE_ESCARGOT)
  if (isolate) {
    printf("(!) gc per isolate isn't supported yet.\n");
  }
  MemoryUtil::gcFull();
#endif
}

void CcTest::PreciseCollectAllGarbage(v8::Isolate* isolate) {
#if defined(CCTEST_ENGINE_ESCARGOT)
  if (isolate) {
    printf("(!) gc per isolate isn't supported yet.\n");
    isolate->RequestGarbageCollectionForTesting(
        v8::Isolate::kFullGarbageCollection);
  }
  MemoryUtil::gcFull();
#endif
}

void CcTest::InitializeVM() {}

static inline bool startsWith(const std::string& string,
                              const std::string& prefix) {
  return (string.size() >= prefix.size()) &&
         (string.compare(0, prefix.size(), prefix) == 0);
}

void InitializeTest::SetUp() {
  CcTest::isolate()->Enter();
}

void InitializeTest::TearDown() {
  CcTest::disposeScope();
  CcTest::disposeIsolate();
}

int main(int argc, char* argv[]) {
  printf("============= Start EscargotShim Test ============= \n");

  EscargotShim::Flags::add(
      EscargotShim::FlagType::AllowCodeGenerationFromString);

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);

    if (startsWith(arg, std::string("-f="))) {
      std::string f =
          std::string("*") + arg.substr(strlen("-f=")) + std::string("*");
      ::testing::GTEST_FLAG(filter) = f.c_str();
#if defined(CCTEST_ENGINE_ESCARGOT)
    } else if (startsWith(arg, std::string("--trace-call"))) {
      EscargotShim::Flags::add(EscargotShim::FlagType::TraceCall);

      std::string str(arg);
      std::string::size_type pos = str.find_first_of('=');
      if (std::string::npos != pos) {
        std::stringstream ss(str.substr(pos + 1));  // +1 for skipping =
        std::string token;
        while (std::getline(ss, token, ',')) {
          if (token.find('-') == 0) {
            EscargotShim::Flags::setNagativeTraceCallId(token.substr(1));
          } else {
            EscargotShim::Flags::setTraceCallId(token);
          }
        }
      }
    } else if (startsWith(arg, std::string("--trace-gc"))) {
      EscargotShim::Flags::add(EscargotShim::FlagType::TraceGC);
#endif
    } else {
      printf("unknown options: %s\n", argv[i]);
    }
  }

  ::testing::InitGoogleTest(&argc, argv);

  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());

  v8::V8::Initialize();

  auto result = RUN_ALL_TESTS();

  CcTest::TearDown();
  if (!disable_automatic_dispose_) {
    v8::V8::Dispose();
  }
  v8::V8::ShutdownPlatform();

  return result;
}
