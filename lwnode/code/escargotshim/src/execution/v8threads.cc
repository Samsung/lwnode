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

// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v8threads.h"

#include "api/isolate.h"
#include "base.h"
#include "v8.h"

using namespace EscargotShim;
namespace v8 {

// Once the Locker is initialized, the current thread will be guaranteed to have
// the lock for a given isolate.
void Locker::Initialize(v8::Isolate* isolate) {
  LWNODE_CHECK(isolate);
  has_lock_ = false;
  top_level_ = true;
  isolate_ = reinterpret_cast<internal::Isolate*>(isolate);
  ThreadManager* manager = IsolateWrap::fromV8(isolate_)->thread_manager();

  if (!manager->IsLockedByCurrentThread()) {
    manager->Lock();
    has_lock_ = true;
  }

  LWNODE_CHECK(manager->IsLockedByCurrentThread());
}

bool Locker::IsLocked(v8::Isolate* isolate) {
  LWNODE_CHECK(isolate);
  return IsolateWrap::fromV8(isolate)
      ->thread_manager()
      ->IsLockedByCurrentThread();
}

bool Locker::IsActive() {
  LWNODE_RETURN_FALSE;
}

Locker::~Locker() {
  ThreadManager* manager = IsolateWrap::fromV8(isolate_)->thread_manager();
  LWNODE_CHECK(manager->IsLockedByCurrentThread());

  if (has_lock_) {
    if (top_level_) {
      manager->FreeThreadResources();
    } else {
      manager->ArchiveThread();
    }
    manager->Unlock();
  }
}

void Unlocker::Initialize(v8::Isolate* isolate) {
  LWNODE_CHECK(isolate);

  ThreadManager* manager = IsolateWrap::fromV8(isolate)->thread_manager();
  LWNODE_CHECK(manager->IsLockedByCurrentThread());
  manager->ArchiveThread();
  manager->Unlock();
}

Unlocker::~Unlocker() {
  ThreadManager* manager = IsolateWrap::fromV8(isolate_)->thread_manager();
  LWNODE_CHECK(!manager->IsLockedByCurrentThread());
  manager->Lock();
  manager->RestoreThread();
}
}  // namespace v8

namespace EscargotShim {

void ThreadManager::Lock() {
  mutex_.lock();
  mutex_owner_ = std::this_thread::get_id();
  LWNODE_CHECK(IsLockedByCurrentThread());
}

void ThreadManager::Unlock() {
  std::thread::id invalid;
  mutex_owner_ = invalid;
  mutex_.unlock();
}

bool ThreadManager::IsLockedByCurrentThread() const {
  return mutex_owner_ == std::this_thread::get_id();
}

void ThreadManager::ArchiveThread() {
  // TODO: Check if we need this logic
}

bool ThreadManager::RestoreThread() {
  // TODO: Check if we need this logic
  return false;
}

void ThreadManager::FreeThreadResources() {
  // TODO: Check if we need this logic
}

bool ThreadManager::IsArchived() {
  // TODO: Check if we need this logic
  return false;
}

}  // namespace EscargotShim
