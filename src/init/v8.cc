// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "init/v8.h"
#include "api/utils/misc.h"
#include "base.h"
#include "v8-platform.h"

namespace v8 {
namespace internal {

v8::Platform* V8::platform_ = nullptr;

void V8::InitializePlatform(v8::Platform* platform) {
  LWNODE_CHECK(!platform_);
  // LWNODE_CHECK(platform); // TODO: default platform is null
  platform_ = platform;
}

void V8::ShutdownPlatform() {
  // LWNODE_CHECK(platform_); // TODO: default platform is null
  platform_ = nullptr;
}

v8::Platform* V8::GetCurrentPlatform() {
  LWNODE_CHECK(platform_);
  return platform_;
}

}  // namespace internal

// static
double Platform::SystemClockTimeMillis() {
  LWNODE_RETURN_0;
}
}  // namespace v8
