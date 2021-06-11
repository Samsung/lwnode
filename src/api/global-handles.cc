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

#include "global-handles.h"
#include <EscargotPublic.h>

namespace EscargotShim {

GlobalHandles::~GlobalHandles() {
  persistentMap_.clear();
}

size_t GlobalHandles::add(ValueWrap* ptr) {
  auto iter = persistentMap_.find(ptr);
  if (iter == persistentMap_.end()) {
    persistentMap_.insert(std::make_pair(ptr, 1));
    return 1;
  } else {
    return ++iter->second;
  }
}

size_t GlobalHandles::remove(ValueWrap* ptr) {
  auto iter = persistentMap_.find(ptr);
  if (iter != persistentMap_.end()) {
    if (iter->second == 1) {
      persistentMap_.erase(iter);
      return 0;
    } else {
      return --iter->second;
    }
  }
  return 0;
}

}  // namespace EscargotShim
