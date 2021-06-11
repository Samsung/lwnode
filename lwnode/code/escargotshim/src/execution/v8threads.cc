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

#include "base.h"
#include "v8.h"

namespace v8 {

// Once the Locker is initialized, the current thread will be guaranteed to have
// the lock for a given isolate.
void Locker::Initialize(v8::Isolate* isolate) {
  LWNODE_RETURN_VOID;
}

bool Locker::IsLocked(v8::Isolate* isolate) {
  LWNODE_RETURN_FALSE;
}

bool Locker::IsActive() {
  LWNODE_RETURN_FALSE;
}

Locker::~Locker() {
  LWNODE_UNIMPLEMENT;
}

void Unlocker::Initialize(v8::Isolate* isolate){LWNODE_RETURN_VOID}

Unlocker::~Unlocker() {
  LWNODE_UNIMPLEMENT;
}

}  // namespace v8
