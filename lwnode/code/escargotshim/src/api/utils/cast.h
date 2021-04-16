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

namespace EscargotShim {

// note: std::reinterpret_pointer_cast (since C++20)
template <class T, class U>
std::shared_ptr<T> reinterpret_shared_pointer_cast(
    const std::shared_ptr<U>& ptr_to_convert) noexcept {
  return std::shared_ptr<T>(
      ptr_to_convert,
      reinterpret_cast<typename std::shared_ptr<T>::element_type*>(
          ptr_to_convert.get()));
}

}  // namespace EscargotShim
