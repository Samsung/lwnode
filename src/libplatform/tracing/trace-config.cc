/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "escargotshim-base.h"
#include "include/libplatform/v8-tracing.h"

namespace v8 {

class Isolate;

namespace platform {
namespace tracing {

TraceConfig* TraceConfig::CreateDefaultTraceConfig() {
  LWNODE_RETURN_NULLPTR;
}

bool TraceConfig::IsCategoryGroupEnabled(const char* category_group) const {
  LWNODE_RETURN_FALSE;
}

void TraceConfig::AddIncludedCategory(const char* included_category) {
  LWNODE_RETURN_VOID;
}

}  // namespace tracing
}  // namespace platform
}  // namespace v8
