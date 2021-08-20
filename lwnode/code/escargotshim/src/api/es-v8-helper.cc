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

#include "es-v8-helper.h"

namespace EscargotShim {

ObjectRef::PresentAttribute V8Helper::toPresentAttribute(
    v8::PropertyAttribute attributes) {
  ObjectRef::PresentAttribute presentAttributes =
      ObjectRef::PresentAttribute::NotPresent;

  if (attributes & v8::ReadOnly) {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes | ObjectRef::PresentAttribute::NonWritablePresent);
  } else {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes | ObjectRef::PresentAttribute::WritablePresent);
  }

  if (attributes & v8::DontEnum) {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes | ObjectRef::PresentAttribute::NonEnumerablePresent);
  } else {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes | ObjectRef::PresentAttribute::EnumerablePresent);
  }

  if (attributes & v8::DontDelete) {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes |
        ObjectRef::PresentAttribute::NonConfigurablePresent);
  } else {
    presentAttributes = static_cast<ObjectRef::PresentAttribute>(
        presentAttributes | ObjectRef::PresentAttribute::ConfigurablePresent);
  }

  return presentAttributes;
}

v8::PropertyAttribute V8Helper::toPropertyAttribute(
    ObjectRef::PresentAttribute attributes) {
  int propertyAttributes = v8::PropertyAttribute::None;

  if (attributes & ObjectRef::PresentAttribute::NonWritablePresent) {
    propertyAttributes =
        propertyAttributes | ObjectRef::PresentAttribute::NonWritablePresent;
  }

  if (attributes & ObjectRef::PresentAttribute::NonEnumerablePresent) {
    propertyAttributes =
        propertyAttributes | ObjectRef::PresentAttribute::NonEnumerablePresent;
  }

  if (attributes & ObjectRef::PresentAttribute::NonConfigurablePresent) {
    propertyAttributes = propertyAttributes |
                         ObjectRef::PresentAttribute::NonConfigurablePresent;
  }

  return static_cast<v8::PropertyAttribute>(propertyAttributes);
}
}  // namespace EscargotShim
