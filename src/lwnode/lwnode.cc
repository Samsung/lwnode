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

#include "lwnode/lwnode.h"
#include <EscargotPublic.h>
#include <malloc.h>  // for malloc_trim
#include <codecvt>
#include <fstream>
#include "api.h"
#include "api/context.h"
#include "api/es-helper.h"
#include "api/isolate.h"
#include "api/utils/misc.h"
#include "api/utils/smaps.h"
#include "base.h"
#include "lwnode/lwnode-gc-strategy.h"
#include "lwnode/lwnode-loader.h"

using namespace v8;
using namespace EscargotShim;
using namespace std::literals;

namespace LWNode {

static void SetMethod(ContextRef* context,
                      ObjectRef* target,
                      std::string name,
                      NativeFunctionPointer nativeFunction) {
  Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* target,
         StringRef* name,
         NativeFunctionPointer nativeFunction) -> ValueRef* {
        target->defineDataProperty(
            state,
            name,
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          nativeFunction,
                                          0,
                                          true,
                                          false)),
            true,
            true,
            true);

        return ValueRef::createUndefined();
      },
      target,
      StringRef::createFromUTF8(name.c_str(), name.length()),
      nativeFunction);
}

constexpr auto kSmapCacheDuration = 600ms;

static std::vector<SmapContents>& getSelfSmaps() {
  static std::vector<SmapContents> s_cachedSmaps = parseSmaps("self");
  static auto s_lastUpdatedTime = std::chrono::steady_clock::now();

  if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - s_lastUpdatedTime) <
      kSmapCacheDuration) {
    return s_cachedSmaps;
  }

  s_cachedSmaps = parseSmaps("self");
  s_lastUpdatedTime = std::chrono::steady_clock::now();
  return s_cachedSmaps;
}

static std::string createDumpFilePath() {
  std::string appName;
  std::ifstream("/proc/self/comm") >> appName;
  std::string outputPath =
      "/tmp/smaps-" + appName + "-" + getCurrentTimeString() + ".csv";
  return outputPath;
}

bool dumpSelfMemorySnapshot() {
  auto& smaps = getSelfSmaps();
  return dumpMemorySnapshot(createDumpFilePath(), smaps);
}

static ValueRef* PssUsage(ExecutionStateRef* state,
                          ValueRef* thisValue,
                          size_t argc,
                          ValueRef** argv,
                          bool isConstructCall) {
  auto& smaps = getSelfSmaps();
  size_t total = calculateTotal(smaps, kPss);
  return ValueRef::create(total);
};

static ValueRef* PssSwapUsage(ExecutionStateRef* state,
                              ValueRef* thisValue,
                              size_t argc,
                              ValueRef** argv,
                              bool isConstructCall) {
  auto& smaps = getSelfSmaps();
  size_t total = calculateTotalPssSwap(smaps);
  return ValueRef::create(total);
};

static ValueRef* RssUsage(ExecutionStateRef* state,
                          ValueRef* thisValue,
                          size_t argc,
                          ValueRef** argv,
                          bool isConstructCall) {
  auto& smaps = getSelfSmaps();
  size_t total = calculateTotalRss(smaps);
  return ValueRef::create(total);
};

static ValueRef* MemSnapshot(ExecutionStateRef* state,
                             ValueRef* thisValue,
                             size_t argc,
                             ValueRef** argv,
                             bool isConstructCall) {
#ifdef PRODUCTION
  return ValueRef::create(false);
#endif
  auto outputPath = createDumpFilePath();
  auto& smaps = getSelfSmaps();
  if (dumpMemorySnapshot(outputPath, smaps)) {
    return StringRef::createFromUTF8(outputPath.c_str(), outputPath.length());
  }
  return ValueRef::createUndefined();
};

static ValueRef* CreateReloadableSourceFromFile(ExecutionStateRef* state,
                                                ValueRef* thisValue,
                                                size_t argc,
                                                ValueRef** argv,
                                                bool isConstructCall) {
  if (argc > 0) {
    if (argv[0]->isString()) {
      std::string fileName = argv[0]
                                 ->toStringWithoutException(state->context())
                                 ->toStdUTF8String();
      return Loader::CreateReloadableSourceFromFile(state, fileName);
    }
  }
  return ValueRef::createUndefined();
}

static ValueRef* getGCMemoryStats(ExecutionStateRef* state,
                                  ValueRef* thisValue,
                                  size_t argc,
                                  ValueRef** argv,
                                  bool isConstructCall) {
  auto context = state->context();
  auto object = ObjectRefHelper::create(context);

  // GC_heapsize - GC_unmapped_bytes
  ObjectRefHelper::setProperty(context,
                               object,
                               StringRef::createFromASCII("heapSize"),
                               ValueRef::create(GC_get_heap_size()))
      .check();

  ObjectRefHelper::setProperty(context,
                               object,
                               StringRef::createFromASCII("unmappedBytes"),
                               ValueRef::create(GC_get_unmapped_bytes()))
      .check();

  ObjectRefHelper::setProperty(context,
                               object,
                               StringRef::createFromASCII("bytesSinceGC"),
                               ValueRef::create(GC_get_bytes_since_gc()))
      .check();

  return ValueRef::create(object);
}

static ValueRef* checkIfHandledAsOneByteString(ExecutionStateRef* state,
                                               ValueRef* thisValue,
                                               size_t argc,
                                               ValueRef** argv,
                                               bool isConstructCall) {
  if (argc > 0 && argv[0]->isString()) {
    return ValueRef::create(argv[0]->asString()->has8BitContent());
  }
  return ValueRef::createUndefined();
}

static ValueRef* hasSystemInfo(ExecutionStateRef* state,
                               ValueRef* thisValue,
                               size_t argc,
                               ValueRef** argv,
                               bool isConstructCall) {
  if (argc <= 0 || !argv[0]->isString()) {
    return ValueRef::create(false);
  }

  auto info = argv[0]->asString()->toStdUTF8String();
  return ValueRef::create(SystemInfo::getInstance()->has(info));
}

void InitializeProcessMethods(Local<Object> target, Local<Context> context) {
  auto esContext = CVAL(*context)->context()->get();
  auto esTarget = CVAL(*target)->value()->asObject();

  EvalResultHelper::attachBuiltinPrint(esContext, esTarget);

  SetMethod(esContext, esTarget, "PssUsage", PssUsage);
  SetMethod(esContext, esTarget, "RssUsage", RssUsage);
  SetMethod(esContext, esTarget, "PssSwapUsage", PssSwapUsage);
  SetMethod(esContext, esTarget, "MemSnapshot", MemSnapshot);
  SetMethod(esContext,
            esTarget,
            "checkIfHandledAsOneByteString",
            checkIfHandledAsOneByteString);
#ifdef LWNODE_USE_RELOAD_SCRIPT
  SetMethod(esContext,
            esTarget,
            "CreateReloadableSourceFromFile",
            CreateReloadableSourceFromFile);
#endif
  SetMethod(esContext, esTarget, "getGCMemoryStats", getGCMemoryStats);
  SetMethod(esContext, esTarget, "hasSystemInfo", hasSystemInfo);
}

void IdleGC(v8::Isolate* isolate) {
  LWNODE_LOG_INFO("IdleGC");
  if (isolate) {
    IsolateWrap::fromV8(isolate)->vmInstance()->enterIdleMode();
  }
  Escargot::Memory::gc();
  malloc_trim(0);
}

void initDebugger() {
  auto lwIsolate = IsolateWrap::GetCurrent();
  if (lwIsolate) {
    lwIsolate->GetCurrentContext()->initDebugger();
  }
}

class MessageLoop::Internal {
 public:
  Internal() { gcStrategy_ = std::make_unique<DelayedGC>(); }
  void handleGC(v8::Isolate* isolate) { gcStrategy_->handle(isolate); }

 private:
  std::unique_ptr<GCStrategyInterface> gcStrategy_;
};

MessageLoop::MessageLoop() {
  internal_ = std::make_unique<Internal>();
}

MessageLoop* MessageLoop::GetInstance() {
  static MessageLoop instance_;
  return &instance_;
}

void MessageLoop::setWakeupMainloopOnceHandler(PlatformHandler handler) {
  platformHandler_ = handler;
}

void MessageLoop::wakeupMainloopOnce() {
  if (platformHandler_.wakeup) {
    platformHandler_.wakeup();
  }
}

void MessageLoop::onPrepare(v8::Isolate* isolate) {
  internal_->handleGC(isolate);
}

Escargot::ContextRef* Utils::ToEsContext(v8::Context* context) {
  return ContextWrap::fromV8(context)->get();
}

v8::Local<v8::Value> Utils::NewLocal(v8::Isolate* isolate,
                                     Escargot::ValueRef* ptr) {
  return v8::Utils::NewLocal<v8::Value>(isolate, ptr);
}

std::string Utils::trimLastNewLineIfNeeded(std::string&& str) {
  if (LogOption::getDefalutOutput()->hasAutoAppendEndOfLine()) {
    // NOTE: We don't consider other OSs except linux.
    static const std::string s_delimiter = "\n";
    str.erase(str.find_last_not_of(s_delimiter) + 1);
  }
  return std::move(str);
}

SystemInfo::SystemInfo() {
#ifdef HOST_TIZEN
  add("tizen");
#else
  add("linux");
#endif
}

SystemInfo* SystemInfo::getInstance() {
  static SystemInfo s_instance;
  return &s_instance;
}

void SystemInfo::add(const char* info, const char* value) {
  infos_.insert({info, value});
}

bool SystemInfo::has(const std::string& info) {
  return infos_.find(info) != infos_.end();
}

bool SystemInfo::get(const char* info, std::string& value) {
  auto iter = infos_.find(info);
  if (iter == infos_.end()) {
    return false;
  }

  value = iter->second;
  return true;
}

}  // namespace LWNode
