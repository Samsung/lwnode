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

// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "string-util.h"

// Magic values subtracted from a buffer value during UTF8 conversion.
// This table contains as many values as there might be trailing bytes
// in a UTF-8 sequence.
const uint32_t UTF8Sequence::s_offsetsFromUTF8[6] = {
    0x00000000UL,
    0x00003080UL,
    0x000E2080UL,
    0x03C82080UL,
    static_cast<uint32_t>(0xFA082080UL),
    static_cast<uint32_t>(0x82082080UL)};

bool UTF8Sequence::convertUTF8ToLatin1(
    std::basic_string<unsigned char, std::char_traits<unsigned char>>&
        oneByteString,
    const unsigned char* sequence,
    const unsigned char* endSequence) {
  while (sequence < endSequence) {
    char32_t character =
        UTF8Sequence::read(sequence, UTF8Sequence::getLength(*sequence));

    // if character is out of acsii and latin1
    if (character > 255) {
      return false;
    } else {
      oneByteString += (unsigned char)character;
    }
  }

  return true;
}
