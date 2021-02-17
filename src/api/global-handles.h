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

#include "handle.h"
#include "utils/gc.h"

namespace EscargotShim {

class GlobalHandles final {
 public:
  ~GlobalHandles();
  size_t add(ValueWrap* ptr);
  size_t remove(ValueWrap* ptr);

 private:
  GCUnorderedMap<ValueWrap*, size_t> persistentMap_;
};
}  // namespace EscargotShim
