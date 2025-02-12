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

#include <string>

#ifndef LWNODE_EXPORT
#define LWNODE_EXPORT __attribute__((visibility("default")))
#endif

namespace lwnode {

LWNODE_EXPORT bool ParseAULEvent(int argc, char** argv);

// Sets the path of the root directory of the JavaScript. If you do
// not put the path argument, the root path is the app's resource path by
// default on Tizen AUL mode. Be sure to call this function before lwnode::Start
// function.
LWNODE_EXPORT bool InitScriptRootPath(const std::string path = "");

// Sets the dlog tag id for debugging. This is only used on Tizen when not in AUL mode.
LWNODE_EXPORT void SetDlogID(const std::string& appId);

LWNODE_EXPORT int Start(int argc, char** argv);

}  // namespace lwnode
