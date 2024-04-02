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

#ifndef __TizenRuntimeInfo__
#define __TizenRuntimeInfo__

#include <string>
#include <map>

namespace DeviceAPI {

class TizenRuntimeInfo {
 public:
  static TizenRuntimeInfo* getInstance();

  const std::string getRuntimeVariable(const std::string& key);

 private:
  TizenRuntimeInfo();

  void initializeDefaultVariable();

  static const int kMaxPackageNameSize{512};

  std::string appid_;
  std::string pkgid_;
  std::string rootpath_;
  std::string privileges_;

  const std::string appid();
  const std::string pkgid();
  const std::string rootpath();
  const std::string privileges();
};

}  // namespace DeviceAPI

#endif
