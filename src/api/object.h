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

#include <EscargotPublic.h>
#include <GCUtil.h>
#include <v8.h>

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

class IsolateWrap;

template <typename Getter, typename Setter>
class NativeDataAccessorPropertyDataWrap
    : public ObjectRef::NativeDataAccessorPropertyData {
 public:
  NativeDataAccessorPropertyDataWrap(v8::Isolate* isolate,
                                     Local<Name> name,
                                     Getter getter,
                                     Setter setter,
                                     Local<Value> data,
                                     bool isWritable,
                                     bool isEnumerable,
                                     bool isConfigurable);

  static NativeDataAccessorPropertyDataWrap* toWrap(
      ObjectRef::NativeDataAccessorPropertyData* ptr) {
    return reinterpret_cast<
        NativeDataAccessorPropertyDataWrap<Getter, Setter>*>(ptr);
  }

  void* operator new(size_t size) { return GC_MALLOC(size); }

  v8::Isolate* m_isolate{nullptr};
  v8::Name* m_name;
  Getter m_getter{nullptr};
  Setter m_setter{nullptr};
  v8::Value* m_data;
};

class ObjectUtils {
 public:
  static Maybe<bool> SetAccessor(ObjectRef* esObject,
                                 IsolateWrap* lwIsolate,
                                 Local<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 Local<Value> data,
                                 PropertyAttribute attribute);
};

class ObjectTemplateUtils {
 public:
  template <typename T, typename F>
  static void SetAccessor(ObjectTemplateRef* esObjectTemplate,
                          IsolateWrap* lwIsolate,
                          Local<Name> name,
                          T getter,
                          F setter,
                          Local<Value> data,
                          PropertyAttribute attribute);
};

}  // namespace EscargotShim
