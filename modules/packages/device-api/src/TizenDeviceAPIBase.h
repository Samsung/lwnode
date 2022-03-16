/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

#include <dlog.h>

#undef LOGGER_TAG
#define LOGGER_TAG "EscargotDeviceAPI"

#ifndef __MODULE__
#define __MODULE__                                                             \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define _LOGGER_LOG(prio, fmt, args...)                                        \
  dlog_print(prio,                                                             \
             LOGGER_TAG,                                                       \
             "%s: %s(%d) > " fmt,                                              \
             __MODULE__,                                                       \
             __func__,                                                         \
             __LINE__,                                                         \
             ##args);

#define _LOGGER_SLOG(prio, fmt, args...)                                       \
  SECURE_LOG_(LOG_ID_MAIN,                                                     \
              prio,                                                            \
              LOGGER_TAG,                                                      \
              "%s: %s(%d) > " fmt,                                             \
              __MODULE__,                                                      \
              __func__,                                                        \
              __LINE__,                                                        \
              ##args);

#define DEVICEAPI_LOG_INFO(fmt, args...) _LOGGER_LOG(DLOG_INFO, fmt, ##args)
#define DEVICEAPI_LOG_ERROR(fmt, args...)                                      \
  _LOGGER_LOG(DLOG_ERROR, "Error: " fmt, ##args)
#define DEVICEAPI_LOG_WARN(fmt, args...) _LOGGER_LOG(DLOG_WARN, fmt, ##args)

#define DEVICEAPI_SLOG_INFO(fmt, args...) _LOGGER_LOG(DLOG_INFO, fmt, ##args)
#define DEVICEAPI_SLOG_ERROR(fmt, args...) _LOGGER_LOG(DLOG_ERROR, fmt, ##args)
#define DEVICEAPI_SLOG_WARN(fmt, args...) _LOGGER_LOG(DLOG_WARN, fmt, ##args)
#define VALUE_NAME_STRCAT(name) name##Value

#define DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE()                                  \
  do {                                                                         \
    DEVICEAPI_LOG_ERROR(                                                       \
        "MUST NOT REACH HERE at %s (%d)\n", __FILE__, __LINE__)                \
    std::abort();                                                              \
  } while (0)

#define DEVICEAPI_ASSERT(condition)                                            \
  do {                                                                         \
    if (!(condition)) {                                                        \
      DEVICEAPI_LOG_ERROR("assert at %s (%d)\n", __FILE__, __LINE__);          \
      std::abort();                                                            \
    }                                                                          \
  } while (0)
