/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <cassert>
#include <functional>
#include "nd-logger.h"

class DebugUtil {
 public:
  ~DebugUtil();

  using CustomHandler = std::function<void(int signal)>;
  static void SetupSignalHandler(CustomHandler custom_handler = nullptr);
  static CustomHandler custom_handler() { return custom_handler_; }

 private:
  static CustomHandler custom_handler_;
};

#ifndef UNLIKELY
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define UNLIKELY(x) __builtin_expect((x), 0)
#else
#define UNLIKELY(x) (x)
#endif
#endif  // UNLIKELY

#ifndef LIKELY
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define LIKELY(x) __builtin_expect((x), 1)
#else
#define LIKELY(x) (x)
#endif
#endif  // LIKELY

#define CHECK_FAILED_HANDLER(msg, ...)                                         \
  LOG_ERROR("CHECK FAILED: (" msg ") at %s (%s:%d)",                           \
            ##__VA_ARGS__,                                                     \
            __PRETTY_FUNCTION__,                                               \
            __FILE_NAME__,                                                     \
            __LINE__);                                                         \
  std::abort();

#define CHECK_MSG(condition, msg, ...)                                         \
  do {                                                                         \
    if (UNLIKELY(!(condition))) {                                              \
      CHECK_FAILED_HANDLER(msg, ##__VA_ARGS__);                                \
    }                                                                          \
  } while (0)

#define CHECK(condition) CHECK_MSG(condition, #condition)

#define CHECK_NOT_NULL(condition) CHECK((condition) != nullptr)
