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

#include <memory>

#include "EscargotPublic.h"

#include "utils/logger/flags.h"

namespace EscargotShim {
class Flags;
class ContextWrap;

class Global {
 public:
  Global() {}

  static Flags* flags();

  static void initGlobalObject(ContextWrap* context);

 private:
  static std::unique_ptr<Flags> s_flags;

  static void initErrorObject(Escargot::ContextRef* context);
  static void initEvalObject(Escargot::ContextRef* context);
};

}  // namespace EscargotShim
