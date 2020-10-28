/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
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

#include "programoptions.h"

namespace Starfish {

bool ProgramOptions::has(const char* key) {
  assert(key != nullptr);
  auto it = m_map.find(key);
  if (it != m_map.end()) {
    return true;
  }
  return false;
}

bool ProgramOptions::is(const char* key) {
  assert(key != nullptr);
  return get<bool>(key);
}

#define MAX_STR_LEN 100

GlobalOptions& GlobalOptions::instance() {
  static GlobalOptions instance;
  return instance;
}

GlobalOptions::GlobalOptions() {
#if !defined(NDEBUG)
  const char* verbose;

  // usage: set environments e.g) `export DEBUG_LEVEL=0`

  verbose = getenv("DEBUG_LEVEL");
  if ((verbose != nullptr) && (strnlen(verbose, MAX_STR_LEN) > 0)) {
    set("DEBUG_LEVEL", std::atoi(verbose));
  } else {
    set("DEBUG_LEVEL", 0);
  }
#endif
}

}  // namespace Starfish
