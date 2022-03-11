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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <node_api.h>
#include <app_common.h>

#include "common.h"

napi_value StringValue(napi_env env, char* str) {
  napi_value val = NULL;
  size_t len = strlen(str);
  if (len == 0) {
    char* unknown = "unknown";
    NAPI_CALL(env,
              napi_create_string_utf8(env, unknown, strlen(unknown), &val));
  } else {
    NAPI_CALL(env, napi_create_string_utf8(env, str, len, &val));
    free(str);
  }
  return val;
}

#define COMMON_PATH_PROPERTY(api, Api) \
  {("get" #Api "Path"), 0, (Get##Api##Path), 0, 0, 0, napi_default, 0},

#define COMMON_APPINFO_PROPERTY(api, Api) \
  {("get" #Api), 0, (Get##Api), 0, 0, 0, napi_default, 0},

#define GET_PATH_FUNCTION(api, Api)                                  \
  napi_value Get##Api##Path(napi_env env, napi_callback_info info) { \
    return StringValue(env, app_get_##api##_path());                 \
  }

#define GET_APPINFO_FUNCTION(api, Api)                         \
  napi_value Get##Api(napi_env env, napi_callback_info info) { \
    char* str = NULL;                                          \
    int ret = app_get_##api(&str);                             \
    if (ret != APP_ERROR_NONE) {                               \
      return StringValue(env, "");                             \
    }                                                          \
    return StringValue(env, str);                              \
  }

#define COMMON_GET_PATH_API(F)       \
  F(data, Data)                      \
  F(cache, Cache)                    \
  F(resource, Resource)              \
  F(shared_data, SharedData)         \
  F(shared_resource, SharedResource) \
  F(shared_trusted, SharedTrusted)   \
  F(external_data, ExternalData)     \
  F(external_cache, ExternalCache)   \
  F(external_shared_data, ExternalSharedData)

#define COMMON_GET_APPINFO_API(F) \
  F(name, Name)                   \
  F(id, ID)                       \
  F(version, Version)

COMMON_GET_PATH_API(GET_PATH_FUNCTION)
COMMON_GET_APPINFO_API(GET_APPINFO_FUNCTION)

napi_value Common(napi_env env) {
  napi_value common = NULL;
  NAPI_CALL(env, napi_create_object(env, &common));

  napi_property_descriptor properties[] = {COMMON_GET_PATH_API(
      COMMON_PATH_PROPERTY) COMMON_GET_APPINFO_API(COMMON_APPINFO_PROPERTY)};

  NAPI_CALL(env, napi_define_properties(
                     env, common, sizeof(properties) / sizeof(*properties),
                     properties));

  return common;
}
