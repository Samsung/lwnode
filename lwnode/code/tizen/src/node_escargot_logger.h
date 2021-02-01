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

#ifndef NODE_ESCARGOT_LOGGER_H
#define NODE_ESCARGOT_LOGGER_H

#include <cstring>

#ifndef __FILENAME__
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#if defined(HOST_TIZEN)
#include <dlog.h>

#define NODE_ESCARGOT_TAG "NODE_ESCARGOT"

#define _LOG(prio, fmt, args...)                                              \
  dlog_print(prio, NODE_ESCARGOT_TAG, "[%s] [%s:%d] " fmt "\n", __FILENAME__, \
             __func__, __LINE__, ##args);

#define NODE_ESCARGOT_LOG_INFO(fmt, args...) _LOG(DLOG_INFO, fmt, ##args)
#define NODE_ESCARGOT_LOG_WARN(fmt, args...) _LOG(DLOG_WARN, fmt, ##args)
#define NODE_ESCARGOT_LOG_ERROR(fmt, args...) \
  _LOG(DLOG_ERROR, "Error: " fmt, ##args)

#else

#define _LOG(fmt, args...) \
  printf("[%s] [%s:%d] " fmt "\n", __FILENAME__, __func__, __LINE__, ##args);

#define NODE_ESCARGOT_LOG_INFO(fmt, args...) _LOG(fmt, ##args)
#define NODE_ESCARGOT_LOG_WARN(fmt, args...) _LOG(fmt, ##args)
#define NODE_ESCARGOT_LOG_ERROR(fmt, args...) _LOG(fmt, ##args)

#endif  // defined(HOST_TIZEN)

#endif
