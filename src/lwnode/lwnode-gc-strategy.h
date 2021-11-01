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

#pragma once

#include <v8.h>
#include <atomic>
#include <chrono>

namespace LWNode {

typedef std::chrono::system_clock::time_point TimePoint;
typedef std::chrono::duration<long double, std::ratio<1, 1000>> MilliSecondTick;

class GCStrategyInterface {
 public:
  virtual bool canScheduleGC() = 0;
  virtual void handle(v8::Isolate* isolate) = 0;
};

class DelayedGC : public GCStrategyInterface {
  /*
    @note Delayed GC Strategy:
    Garbage collection will be conducted :
      - (a) `delayedGCTimeout`ms after this function is last called
      - (b) every `periodicGCduration_`ms when the timer is running
  */

  enum class DelayedGCState {
    TIMER_START = 0,
    TASK_SCHEDULED,
    TIMER_END,
  };
  static constexpr unsigned DEFAULT_DELAYED_GC_TIMEOUT{1500};
  static constexpr unsigned DEFAULT_PERIODIC_GC_DURATION{5000};

 public:
  bool canScheduleGC() override;
  void handle(v8::Isolate* isolate) override;

 private:
  std::atomic<DelayedGCState> state_{DelayedGCState::TIMER_END};
  std::atomic<bool> isLastCallChecked_{true};
  const int periodicGCduration_{DEFAULT_PERIODIC_GC_DURATION};
  const unsigned delayedGCTimeout_{DEFAULT_DELAYED_GC_TIMEOUT};
  TimePoint lastCheckedTime_;
};

class EveryTickGC : public GCStrategyInterface {
 public:
  bool canScheduleGC() override { return true; };
  void handle(v8::Isolate* isolate) override;
};

}  // namespace LWNode
