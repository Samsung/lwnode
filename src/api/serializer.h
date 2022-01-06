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

#if defined(LWNODE_ENABLE_EXPERIMENTAL_SERIALIZATION)

#pragma once

#include <EscargotPublic.h>
#include <v8.h>

#include "utils/optional.h"

using namespace Escargot;

namespace EscargotShim {

enum class SerializationTag : uint8_t;

class IsolateWrap;

class ValueSerializer {
 public:
  ValueSerializer(IsolateWrap* lwIsolate,
                  v8::ValueSerializer::Delegate* delegate);
  void WriteHeader();
  bool WriteValue(ValueRef* value);

  std::pair<uint8_t*, size_t> Release();

 private:
  void WriteTag(SerializationTag tag);
  void WriteRawBytes(const void* source, size_t length);
  uint8_t* ReserveRawBytes(size_t bytes);

  template <typename T>
  void WriteVarint(T value);
  template <typename T>
  void WriteZigZag(T value);
  void WriteString(StringRef* string);
  bool WriteObject(ObjectRef* object);
  bool WriteArrayBuffer(size_t length, uint8_t* bytes);
  bool WriteArrayBufferView(ArrayBufferViewRef* arrayBufferView);
  bool ExpandBuffer(size_t required_capacity);
  bool ThrowIfOutOfMemory();

  IsolateWrap* lwIsolate_ = nullptr;
  v8::ValueSerializer::Delegate* delegate_ = nullptr;
  uint8_t* buffer_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
  bool out_of_memory_ = false;
};

class ValueDeserializer {
 public:
  ValueDeserializer(IsolateWrap* isolate,
                    v8::ValueDeserializer::Delegate* delegate,
                    const uint8_t* data,
                    const size_t size);
  ~ValueDeserializer(){};

  bool ReadTag(SerializationTag& tag);
  bool CheckTag(SerializationTag tag);

  OptionalRef<ValueRef> ReadValue();

 private:
  template <typename T>
  bool ReadVarint(T& value);
  template <typename T>
  bool ReadZigZag(T& value);
  bool ReadDouble(double& value);
  bool ReadObject(ObjectRef* object);
  bool ReadArrayBuffer(ArrayBufferObjectRef*& arayBufferObject);
  bool ReadArrayBufferView(ArrayBufferViewRef*& arrayBufferView,
                           ArrayBufferObjectRef* arrayBufferObject);

  bool ReadRawBytes(size_t size, const uint8_t*& data);

  IsolateWrap* isolate_ = nullptr;
  v8::ValueDeserializer::Delegate* delegate_ = nullptr;
  const uint8_t* buffer_ = nullptr;
  const size_t size_ = 0;
  size_t position_ = 0;
  const size_t end_ = 0;
};

}  // namespace EscargotShim

#endif
