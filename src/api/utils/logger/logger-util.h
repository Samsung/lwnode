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

#include <string>

std::string getPrettyFunctionName(const std::string fullname);
std::string createCodeLocation(const char* functionName,
                               const char* filename,
                               const int line);

inline const char* strBool(bool value) {
  return value ? "True" : "False";
}

class IndentCounter {
 public:
  IndentCounter(std::string id);
  ~IndentCounter();
  static std::string getString(std::string id = "");
  static void indent(std::string id);
  static void unIndent(std::string id);

 private:
  std::string id_;
};
