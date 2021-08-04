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

#if defined(LWNODE)

// internals
#include "api/handle.h"
#include "api/utils/flags.h"
#include "api/utils/gc.h"
#include "api/utils/logger.h"
#include "api/utils/misc.h"

namespace e = EscargotShim;

#define VAL(that) reinterpret_cast<e::ValueWrap*>(that)
#define CVAL(that) reinterpret_cast<const e::ValueWrap*>(that)

#endif
