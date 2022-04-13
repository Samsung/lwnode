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

#include <atomic>
#include <mutex>
#include <thread>

#include "api/utils/gc-util.h"

namespace EscargotShim {

class ThreadManager : public gc {
 public:
  void Lock();
  void Unlock();

  void ArchiveThread();
  bool RestoreThread();
  void FreeThreadResources();
  bool IsArchived();

  bool IsLockedByCurrentThread() const;

  std::mutex mutex_;
  std::atomic<std::thread::id> mutex_owner_;
};
}  // namespace EscargotShim
