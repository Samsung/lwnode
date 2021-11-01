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

#include "lwnode-gc-strategy.h"
#include <thread>
#include "lwnode.h"

namespace LWNode {

static TimePoint getCurrentTime() {
  return std::chrono::system_clock::now();
}

bool DelayedGC::canScheduleGC() {
  // condition (a)
  if (isLastCallChecked_ == true) {
    return true;
  }

  // condition (b)
  if (MilliSecondTick(getCurrentTime() - lastCheckedTime_).count() >
      periodicGCduration_) {
    lastCheckedTime_ = getCurrentTime();
    return true;
  }

  return false;
}

void DelayedGC::handle(v8::Isolate* isolate) {
  isLastCallChecked_ = false;

  if (state_ == DelayedGCState::TIMER_END) {
    std::thread([&]() {
      state_ = DelayedGCState::TIMER_START;
      lastCheckedTime_ = getCurrentTime();

      do {
        isLastCallChecked_ = true;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(delayedGCTimeout_));

        if (canScheduleGC()) {
          MessageLoop::GetInstance()->wakeupMainloopOnce();
          state_ = DelayedGCState::TASK_SCHEDULED;
          break;
        }
      } while (isLastCallChecked_ == false);
    }).detach();

  } else if (state_ == DelayedGCState::TASK_SCHEDULED) {
    IdleGC(isolate);
    state_ = DelayedGCState::TIMER_END;
  }
}

void EveryTickGC::handle(v8::Isolate* isolate) {
  IdleGC(isolate);
}

}  // namespace LWNode
