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

#include "node_escargot_info.h"

bool is_aul_launched = false;

namespace nescargot {

void getInfoString(std::vector<std::string>& array) {
#ifdef NODE_ENGINE_ESCARGOT
  array.push_back("escargot");
#endif

#ifdef HOST_TIZEN
  array.push_back("tizen");
  array.push_back(DEVICE_PROFILE);
#else
  array.push_back("linux");
#endif

#ifdef NESCARGOT_TIZEN_TV
  array.push_back("signed");
#endif

  if (is_aul_launched) {
    array.push_back("aul");
  }
}

}  // namespace nescargot
