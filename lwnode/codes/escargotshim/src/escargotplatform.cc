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

#include "escargotplatform.h"
#include "escargotbase.h"

namespace EscargotShim {

DefaultPlatform::DefaultPlatform() {}

DefaultPlatform::~DefaultPlatform() {}

bool DefaultPlatform::PumpMessageLoop(v8::Isolate* isolate) {
  v8::Task* task = nullptr;

  {
    std::unique_lock<std::mutex> guard(m_mutex);
    task = PopTaskInMainThreadQueue(isolate);
  }

  if (task != nullptr) {
    task->Run();
    delete task;

    return true;
  }

  return false;
}

// TODO:

// void DefaultPlatform::CallOnBackgroundThread(
//     v8::Task* task, v8::Platform::ExpectedRuntime expected_runtime) {
//   // CHAKRA-TODO: Figure out what to do here.
//   NESCARGOT_ASSERT(false);
// }

void DefaultPlatform::CallOnForegroundThread(v8::Isolate* isolate,
                                             v8::Task* task) {
  std::unique_lock<std::mutex> guard(m_mutex);
  main_thread_queue_[isolate].push(task);
}

double DefaultPlatform::MonotonicallyIncreasingTime() {
  // CHAKRA-TODO: Figure out what to do here.
  NESCARGOT_ASSERT(false);

  return 0;
}

v8::Task* DefaultPlatform::PopTaskInMainThreadQueue(v8::Isolate* isolate) {
  auto it = main_thread_queue_.find(isolate);
  if (it == main_thread_queue_.end() || it->second.empty()) {
    return nullptr;
  }
  v8::Task* task = it->second.front();
  it->second.pop();
  return task;
}

int DefaultPlatform::NumberOfWorkerThreads() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

std::shared_ptr<v8::TaskRunner> DefaultPlatform::GetForegroundTaskRunner(
    v8::Isolate* isolate) {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

void DefaultPlatform::CallOnWorkerThread(std::unique_ptr<v8::Task> task) {
  NESCARGOT_UNIMPLEMENTED("");
}

void DefaultPlatform::CallDelayedOnWorkerThread(std::unique_ptr<v8::Task> task,
                                                double delay_in_seconds) {
  NESCARGOT_UNIMPLEMENTED("");
}

void DefaultPlatform::CallDelayedOnForegroundThread(v8::Isolate* isolate,
                                                    v8::Task* task,
                                                    double delay_in_seconds) {
  NESCARGOT_UNIMPLEMENTED("");
}

double DefaultPlatform::CurrentClockTimeMillis() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

v8::TracingController* DefaultPlatform::GetTracingController() {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}
}  // namespace EscargotShim
