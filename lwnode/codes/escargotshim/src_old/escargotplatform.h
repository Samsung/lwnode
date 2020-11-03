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

#ifndef __ESCARGOT_PLATFORM__
#define __ESCARGOT_PLATFORM__

#include <map>
#include <mutex>
#include <queue>
#include "include/v8-platform.h"

namespace EscargotShim {

class DefaultPlatform : public v8::Platform {
 public:
  DefaultPlatform();
  virtual ~DefaultPlatform();

  bool PumpMessageLoop(v8::Isolate* isolate);

  int NumberOfWorkerThreads() override;
  std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner(
      v8::Isolate* isolate) override;
  void CallOnWorkerThread(std::unique_ptr<v8::Task> task) override;

  void CallDelayedOnWorkerThread(std::unique_ptr<v8::Task> task,
                                 double delay_in_seconds) override;

  void CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task) override;

  void CallDelayedOnForegroundThread(v8::Isolate* isolate,
                                     v8::Task* task,
                                     double delay_in_seconds) override;
  double MonotonicallyIncreasingTime() override;
  double CurrentClockTimeMillis() override;
  v8::TracingController* GetTracingController() override;

 private:
  v8::Task* PopTaskInMainThreadQueue(v8::Isolate* isolate);
  std::mutex m_mutex;
  std::map<v8::Isolate*, std::queue<v8::Task*> > main_thread_queue_;
};

}  // namespace EscargotShim

#endif
