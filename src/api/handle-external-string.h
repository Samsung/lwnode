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

using namespace Escargot;

namespace EscargotShim {

class ExternalStringWrap final : public ValueWrap {
 public:
  ExternalStringWrap(StringRef* ptr,
                     v8::String::ExternalStringResourceBase* resource)
      : ValueWrap(ptr,
                  HandleWrap::Type::JsValue,
                  HandleWrap::ValueType::ExternalString),
        resource_(resource) {}

  static ExternalStringWrap* create(
      StringRef* ptr, v8::String::ExternalStringResourceBase* resource) {
    return new ExternalStringWrap(ptr, resource);
  }

  v8::String::ExternalStringResourceBase* resource() const { return resource_; }

 private:
  v8::String::ExternalStringResourceBase* resource_{nullptr};
};

}  // namespace EscargotShim
