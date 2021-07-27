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
#include <v8.h>

using namespace Escargot;

namespace EscargotShim {

enum class SerializationTag : uint8_t;

class IsolateWrap;

class ValueSerializer {
 public:
  ValueSerializer(v8::Isolate* isolate,
                  v8::ValueSerializer::Delegate* delegate);
  void WriteHeader();
  bool WriteValue(ValueRef* value);

 private:
  void WriteTag(SerializationTag tag);
  template <typename T>
  void WriteVarint(T value);
  void WriteUint32(uint32_t value);
  void WriteUint64(uint64_t value);
  void WriteRawBytes(const void* source, size_t length);
  void WriteDouble(double value);
  bool WriteJSObject(ObjectRef* object);

  v8::Maybe<uint8_t*> ReserveRawBytes(size_t bytes);
  v8::Maybe<bool> ExpandBuffer(size_t required_capacity);

  bool ThrowIfOutOfMemory();

  IsolateWrap* lwIsolate_ = nullptr;
  v8::ValueSerializer::Delegate* delegate_ = nullptr;
  uint8_t* buffer_ = nullptr;
  size_t buffer_size_ = 0;
  size_t buffer_capacity_ = 0;
  bool out_of_memory_ = false;
};

}  // namespace EscargotShim
