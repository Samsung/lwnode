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

#include <cstdint>

namespace EscargotShim {

typedef uint8_t flags_t;

enum Flags : flags_t {
  Empty = 0,
  ExposeGC = 1 << 1,
  UseStrict = 1 << 2,
  DisableIdleGC = 1 << 3,
  TopLevelWait = 1 << 4,
};

void setFlags(flags_t flags);
flags_t getFlags();

}  // namespace EscargotShim
