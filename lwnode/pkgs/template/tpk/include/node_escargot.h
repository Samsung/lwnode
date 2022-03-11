/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
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

#ifndef NODE_ESCARGOT_H
#define NODE_ESCARGOT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_PACKAGE_STR_SIZE 512

namespace node {

int nescargot_set_printf_tagname(const char* name);
int nescargot_get_printf_tagname(char* buffer, unsigned num);

int nescargot_printf(const char* format, ...);
int nescargot_printf_warn(const char* format, ...);
int nescargot_printf_err(const char* format, ...);
}  // namespace node

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
