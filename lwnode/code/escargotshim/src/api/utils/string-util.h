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

// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "misc.h"

namespace EscargotShim {
class Constants {
 public:
  static constexpr int kMaxStringLength =
      sizeof(void*) == 4 ? (1 << 28) - 16 : (1 << 29) - 24;
};
}  // namespace EscargotShim

inline int strLength(const char* string) {
  size_t len = strlen(string);
  LWNODE_CHECK(EscargotShim::Constants::kMaxStringLength >= len);
  return static_cast<int>(len);
}

inline int strLength(const uint8_t* string) {
  return strLength(reinterpret_cast<const char*>(string));
}

inline int strLength(const uint16_t* string) {
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

class UTF8Sequence {
 public:
  static inline bool isASCII(uint16_t character) {
    return !(character & ~0x7F);
  }

  static inline int getLengthNonASCII(uint8_t byte) {
    if ((byte & 0xC0) != 0xC0) return 0;
    if ((byte & 0xE0) == 0xC0) return 2;
    if ((byte & 0xF0) == 0xE0) return 3;
    if ((byte & 0xF8) == 0xF0) return 4;
    return 0;
  }

  static inline int getLength(uint8_t byte) {
    return isASCII(byte) ? 1 : getLengthNonASCII(byte);
  }

  static inline uint32_t read(const uint8_t*& sequence, size_t length) {
    uint32_t character = 0;

    // The cases all fall through.
    switch (length) {
      case 6:
        character += static_cast<uint8_t>(*sequence++);
        character <<= 6;  // Fall through.
      case 5:
        character += static_cast<uint8_t>(*sequence++);
        character <<= 6;  // Fall through.
      case 4:
        character += static_cast<uint8_t>(*sequence++);
        character <<= 6;  // Fall through.
      case 3:
        character += static_cast<uint8_t>(*sequence++);
        character <<= 6;  // Fall through.
      case 2:
        character += static_cast<uint8_t>(*sequence++);
        character <<= 6;  // Fall through.
      case 1:
        character += static_cast<uint8_t>(*sequence++);
    }

    return character - s_offsetsFromUTF8[length - 1];
  }

  using u8string = std::basic_string<uint8_t, std::char_traits<uint8_t>>;
  static bool convertUTF8ToLatin1(u8string& latin1String,
                                  const uint8_t* sequence,
                                  const uint8_t* endSequence);

 private:
  static const uint32_t s_offsetsFromUTF8[6];
};
