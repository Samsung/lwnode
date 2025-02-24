/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <EscargotPublic.h>
#include <GCUtil.h>

class BaseObject : public gc {
 public:
  BaseObject(Escargot::ObjectRef* object);
  virtual ~BaseObject();

  virtual const char* id() = 0;

  template <typename T, typename... Args>
  static Escargot::ObjectRef* CreateWrapper(Escargot::ValueRef* this_value,
                                            Args&&... args) {
    new (GC) T(this_value->asObject(), std::forward<Args>(args)...);
    Escargot::Memory::gcRegisterFinalizer(this_value, [](void* self) {
      auto object = reinterpret_cast<Escargot::ObjectRef*>(self);
      // Although T is GC-allocated, delete it to explicitly call destructors.
      delete reinterpret_cast<T*>(object->extraData());
    });
    return this_value->asObject();
  }
};

template <typename T>
T* GetExtraData(Escargot::ValueRef* value) {
  return reinterpret_cast<T*>(value->asObject()->extraData());
}
