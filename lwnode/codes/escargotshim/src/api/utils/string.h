/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

#include "misc.h"

namespace EscargotShim {
class Constants {
 public:
  static constexpr int kMaxStringLength =
      sizeof(void*) == 4 ? (1 << 28) - 16 : (1 << 29) - 24;
};
}  // namespace EscargotShim

inline int stringLength(const char* string) {
  size_t len = strlen(string);
  LWNODE_CHECK(EscargotShim::Constants::kMaxStringLength >= len);
  return static_cast<int>(len);
}

inline int stringLength(const uint8_t* string) {
  return stringLength(reinterpret_cast<const char*>(string));
}

inline int stringLength(const uint16_t* string) {
  size_t length = 0;
  while (string[length] != '\0') length++;
  LWNODE_CHECK(EscargotShim::Constants::kMaxStringLength >= length);
  return static_cast<int>(length);
}

inline bool strEquals(const char* str, const char* pat) {
  return strncmp(str, pat, EscargotShim::Constants::kMaxStringLength) == 0;
}

template <size_t N>
bool strStartsWith(const char* str, const char (&prefix)[N]) {
  return strncmp(str, prefix, N - 1) == 0;
}
