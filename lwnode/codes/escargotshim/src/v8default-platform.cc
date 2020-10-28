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

#include "escargotutil.h"

#include "escargotisolateshim.h"
#include "escargotplatform.h"

#include "v8.h"

#include "libplatform/libplatform.h"

namespace v8 {
namespace platform {
v8::Platform* CreateDefaultPlatform(int thread_pool_size) {
  EscargotShim::DefaultPlatform* platform = new EscargotShim::DefaultPlatform();
  return platform;
}

bool PumpMessageLoop(v8::Platform* platform, v8::Isolate* isolate) {
  return static_cast<EscargotShim::DefaultPlatform*>(platform)->PumpMessageLoop(
      isolate);
}

void SetTracingController(
    v8::Platform* platform,
    v8::platform::tracing::TracingController* tracing_controller) {
  NESCARGOT_UNIMPLEMENTED("TracingController");
}

std::unique_ptr<v8::Platform> NewDefaultPlatform(
    int thread_pool_size,
    IdleTaskSupport idle_task_support,
    InProcessStackDumping in_process_stack_dumping,
    std::unique_ptr<v8::TracingController> tracing_controller) {
  NESCARGOT_UNIMPLEMENTED("");
  return std::unique_ptr<v8::Platform>(nullptr);
}

}  // namespace platform
}  // namespace v8
