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

#pragma once

#include "api/utils/logger.h"

#define LWNODE_RETURN_VOID                                                     \
  LWNODE_UNIMPLEMENT;                                                          \
  return;

#define LWNODE_RETURN_0                                                        \
  LWNODE_UNIMPLEMENT;                                                          \
  return 0;

#define LWNODE_RETURN_FALSE                                                    \
  LWNODE_UNIMPLEMENT;                                                          \
  return false;

#define LWNODE_RETURN_NULLPTR                                                  \
  LWNODE_UNIMPLEMENT;                                                          \
  return nullptr;

#define LWNODE_RETURN_LOCAL(type)                                              \
  LWNODE_UNIMPLEMENT;                                                          \
  return Local<type>();

#define LWNODE_RETURN_MAYBE(type)                                              \
  LWNODE_UNIMPLEMENT;                                                          \
  return Nothing<type>();
