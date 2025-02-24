/*
 * Copyright (c) 2025-present Samsung Electronics Co., Ltd
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

#include "es.h"
#include <iostream>

static AbortHandler s_abortHandler = nullptr;

void Abort(const char* message) {
  if (s_abortHandler) {
    return s_abortHandler(message);
  }
  std::cout << "abort: " << message << std::endl;
  std::abort();
}

void SetAbortHandler(AbortHandler handler) {
  s_abortHandler = handler;
}
