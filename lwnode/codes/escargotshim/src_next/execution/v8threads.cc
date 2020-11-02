/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v8.h"
#include "escargotshim-base.h"

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

void Unlocker::Initialize(v8::Isolate* isolate) {
  LWNODE_RETURN_VOID
}

Unlocker::~Unlocker() {
  LWNODE_UNIMPLEMENT;
}

}  // namespace v8
