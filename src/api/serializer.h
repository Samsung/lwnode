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

struct SerializerBuffer {
  SerializerBuffer() = default;
  SerializerBuffer(const uint8_t* _data, size_t _size)
      : data(const_cast<uint8_t*>(_data)), size(_size) {}
  uint8_t* data = nullptr;
  size_t size = 0;
  size_t capacity = 0;
  size_t position = 0;

  uint8_t currentPositionData() {
    LWNODE_CHECK(!isOverflow());
    return data[position];
  }

  uint8_t* currentPositionAddress() {
    LWNODE_CHECK(!isOverflow());
    return &data[position];
  }

  bool isOverflow() { return position >= size; }
};

class ValueSerializer {
 public:
  ValueSerializer(IsolateWrap* lwIsolate,
                  v8::ValueSerializer::Delegate* delegate);
  void WriteHeader();
  bool WriteValue(ValueRef* value);
  bool WriteUint32(uint32_t value);
  bool WriteInt32(int32_t value);
  bool WriteNumber(double value);

  std::pair<uint8_t*, size_t> Release();

 private:
  void WriteTag(SerializationTag tag);
  void WriteRawBytes(const void* source, size_t length);
  uint8_t* ReserveRawBytes(size_t bytes);

  template <typename T>
  void WriteVarint(T value);
  template <typename T>
  void WriteZigZag(T value);
  bool WriteBoolean(bool value);
  void WriteString(StringRef* string);
  bool WriteObject(ObjectRef* object);
  bool WriteHostObject(ObjectRef* object);
  bool WriteArrayBuffer(size_t length, uint8_t* bytes);
  bool WriteArrayBufferView(ArrayBufferViewRef* arrayBufferView);
  bool WriteTypedArrayObject(Escargot::ArrayBufferViewRef* bufferView);
  bool ExpandBuffer(size_t required_capacity);
  bool ThrowIfOutOfMemory();
  void ThrowDataCloneError();

  IsolateWrap* lwIsolate_ = nullptr;
  v8::ValueSerializer::Delegate* delegate_ = nullptr;
  SerializerBuffer buffer_;
  bool out_of_memory_ = false;
};

class ValueDeserializer {
 public:
  ValueDeserializer(IsolateWrap* isolate,
                    v8::ValueDeserializer::Delegate* delegate,
                    const uint8_t* data,
                    const size_t size);
  ~ValueDeserializer(){};

  OptionalRef<ValueRef> ReadValue();
  bool ReadUint32(uint32_t*& value);

 private:
  bool ReadTag(SerializationTag& tag);
  bool CheckTag(SerializationTag tag);
  template <typename T>
  bool ReadVarint(T& value);
  template <typename T>
  bool ReadZigZag(T& value);
  bool ReadDouble(double& value);
  bool ReadOneByteString(StringRef*& string);
  bool ReadTwoByteString(StringRef*& string);
  bool ReadObject(ObjectRef* object);
  bool ReadHostObject(ObjectRef*& object);
  bool ReadJsArrayBuffer(ValueRef*& value);
  bool ReadArrayBuffer(ArrayBufferObjectRef*& arayBufferObject);
  bool ReadArrayBufferView(ArrayBufferViewRef*& arrayBufferView,
                           ArrayBufferObjectRef* arrayBufferObject);

  bool ReadRawBytes(size_t size, const uint8_t*& data);

  IsolateWrap* lwIsolate_ = nullptr;
  v8::ValueDeserializer::Delegate* delegate_ = nullptr;
  SerializerBuffer buffer_;
};

}  // namespace EscargotShim

#endif
