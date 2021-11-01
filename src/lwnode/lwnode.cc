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
#include <atomic>
#include <chrono>
#include <codecvt>
#include <fstream>
#include <thread>
#include "api.h"
#include "api/context.h"
#include "api/es-helper.h"
#include "api/isolate.h"
#include "api/utils/misc.h"
#include "api/utils/smaps.h"
#include "base.h"
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

void InitializeProcessMethods(Local<Object> target, Local<Context> context) {
  auto esContext = CVAL(*context)->context()->get();
  auto esTarget = CVAL(*target)->value()->asObject();

#if !defined(NDEBUG)
  EvalResultHelper::attachBuiltinPrint(esContext, esTarget);
#endif

  SetMethod(esContext, esTarget, "PssUsage", PssUsage);
  SetMethod(esContext, esTarget, "RssUsage", RssUsage);
  SetMethod(esContext, esTarget, "PssSwapUsage", PssSwapUsage);
  SetMethod(esContext, esTarget, "MemSnapshot", MemSnapshot);
#ifdef LWNODE_USE_RELOAD_SCRIPT
  SetMethod(esContext,
            esTarget,
            "CreateReloadableSourceFromFile",
            CreateReloadableSourceFromFile);
#endif
}

static void IdleGC(v8::Isolate* isolate) {
  LWNODE_LOG_INFO("IdleGC");
  IsolateWrap::fromV8(isolate)->vmInstance()->enterIdleMode();
  Escargot::Memory::gc();
  malloc_trim(0);
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

typedef std::chrono::system_clock::time_point TimePoint;
typedef std::chrono::duration<long double, std::ratio<1, 1000>> MilliSecondTick;
static TimePoint getCurrentTime() {
  return std::chrono::system_clock::now();
}

void MessageLoop::handleDelayedGC(v8::Isolate* isolate) {
  enum class DelayedGCState {
    TIMER_START = 0,
    TASK_SCHEDULED,
    TIMER_END,
  };

  static std::atomic<DelayedGCState> s_state(DelayedGCState::TIMER_END);
  static std::atomic<bool> s_isLastCallChecked(true);
  static const int s_periodicGCduration = DEFAULT_PERIODIC_GC_DURATION;
  static TimePoint s_lastCheckedTime;

  static std::function<bool()> canScheduleGC = []() {
    // condition (a)
    if (s_isLastCallChecked == true) {
      return true;
    }

    // condition (b)
    if (MilliSecondTick(getCurrentTime() - s_lastCheckedTime).count() >
        s_periodicGCduration) {
      s_lastCheckedTime = getCurrentTime();
      return true;
    }

    return false;
  };

  /*
    @note Delayed GC Strategy:
    Garbage collection will be conducted :
      - (a) `delayedGCTimeout`ms after this function is last called
      - (b) every `s_periodicGCduration`ms when the timer is running

    @todo consider applying strategy pattern if needed
  */

  s_isLastCallChecked = false;

  if (s_state == DelayedGCState::TIMER_END) {
    // start a timer thread
    std::thread([]() {
      s_state = DelayedGCState::TIMER_START;
      s_lastCheckedTime = getCurrentTime();

      do {
        s_isLastCallChecked = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(
            MessageLoop::GetInstance()->delayedGCTimeout()));

        if (canScheduleGC()) {
          MessageLoop::GetInstance()->wakeupMainloopOnce();
          s_state = DelayedGCState::TASK_SCHEDULED;
          break;
        }
      } while (s_isLastCallChecked == false);
    }).detach();

  } else if (s_state == DelayedGCState::TASK_SCHEDULED) {
    IdleGC(isolate);
    s_state = DelayedGCState::TIMER_END;
  }
}

void MessageLoop::onPrepare(v8::Isolate* isolate) {
  handleDelayedGC(isolate);
}

}  // namespace LWNode
