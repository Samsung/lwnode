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

#include "api.h"
#include "base.h"

using namespace Escargot;
using namespace EscargotShim;

namespace v8 {
// --- V a l u e   S e r i a l i z a t i o n ---

Maybe<bool> ValueSerializer::Delegate::WriteHostObject(Isolate* v8_isolate,
                                                       Local<Object> object) {
  LWNODE_RETURN_MAYBE(bool);
}

Maybe<uint32_t> ValueSerializer::Delegate::GetSharedArrayBufferId(
    Isolate* v8_isolate, Local<SharedArrayBuffer> shared_array_buffer) {
  LWNODE_RETURN_MAYBE(uint32_t);
}

Maybe<uint32_t> ValueSerializer::Delegate::GetWasmModuleTransferId(
    Isolate* v8_isolate, Local<WasmModuleObject> module) {
  LWNODE_RETURN_MAYBE(uint32_t);
}

void* ValueSerializer::Delegate::ReallocateBufferMemory(void* old_buffer,
                                                        size_t size,
                                                        size_t* actual_size) {
  *actual_size = size;
  return realloc(old_buffer, size);
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
  return free(buffer);
}

struct ValueSerializer::PrivateData {
  explicit PrivateData(ValueSerializer::Delegate* delegate)
      : delegate(delegate) {}

  ~PrivateData() {}

  ValueSerializer::Delegate* delegate = nullptr;
  std::ostringstream stream;
};

ValueSerializer::ValueSerializer(Isolate* isolate)
    : ValueSerializer(isolate, nullptr) {
  LWNODE_UNIMPLEMENT;
}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate)
    : private_(new PrivateData(delegate)) {}

ValueSerializer::~ValueSerializer() {
  delete private_;
}

void ValueSerializer::WriteHeader() {
  LWNODE_ONCE(LWNODE_UNIMPLEMENT_WORKAROUND);
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value) {
  bool result =
      SerializerRef::serializeInto(CVAL(*value)->value(), private_->stream);

  if (!result) {
    LWNODE_DLOG_WARN("ValueSerializer::WriteValue: Writing to the type of "
                     "input value is not supported.")
  }
  return Just(result);
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  auto str = private_->stream.str();
  auto size = str.size();
  LWNODE_CALL_TRACE_ID(SERIALIZER, "buffer size: %zu\n", size);

  uint8_t* buffer = nullptr;

  if (size > 0) {
    if (private_->delegate) {
      size_t allocatedSize = 0;
      buffer = static_cast<uint8_t*>(private_->delegate->ReallocateBufferMemory(
          buffer, size, &allocatedSize));
      LWNODE_CHECK(size == allocatedSize);
    } else {
      buffer = static_cast<uint8_t*>(malloc(size * sizeof(uint8_t)));
    }

    memcpy(buffer, str.data(), str.length());
  }

  private_->stream.str("");
  private_->stream.clear();

  return std::make_pair(buffer, size);
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {
  LWNODE_RETURN_VOID;
}

void ValueSerializer::WriteUint32(uint32_t value) {
  SerializerRef::serializeInto(ValueRef::create(value), private_->stream);
}

void ValueSerializer::WriteUint64(uint64_t value) {
  SerializerRef::serializeInto(ValueRef::create(value), private_->stream);
}

void ValueSerializer::WriteDouble(double value) {
  SerializerRef::serializeInto(ValueRef::create(value), private_->stream);
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  LWNODE_RETURN_VOID;
}

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(
    Isolate* v8_isolate) {
  LWNODE_RETURN_LOCAL(Object);
}

MaybeLocal<WasmModuleObject> ValueDeserializer::Delegate::GetWasmModuleFromId(
    Isolate* v8_isolate, uint32_t id) {
  LWNODE_RETURN_LOCAL(WasmModuleObject);
}

MaybeLocal<SharedArrayBuffer>
ValueDeserializer::Delegate::GetSharedArrayBufferFromId(Isolate* v8_isolate,
                                                        uint32_t id) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer);
}

struct ValueDeserializer::PrivateData {
  explicit PrivateData(ValueDeserializer::Delegate* delegate)
      : delegate(delegate) {}
  ValueDeserializer::Delegate* delegate = nullptr;
  std::istringstream stream;
};

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size)
    : ValueDeserializer(isolate, data, size, nullptr) {}

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size,
                                     Delegate* delegate)
    : private_(new PrivateData(delegate)) {
  private_->stream = std::istringstream(
      std::string(reinterpret_cast<const char*>(data), size));
}

ValueDeserializer::~ValueDeserializer() {
  delete private_;
}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  LWNODE_RETURN_MAYBE(bool)
}

void ValueDeserializer::SetSupportsLegacyWireFormat(
    bool supports_legacy_wire_format) {
  LWNODE_RETURN_VOID;
}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  LWNODE_RETURN_0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  API_ENTER_WITH_CONTEXT(context, MaybeLocal<Value>());
  auto esContext = lwIsolate->GetCurrentContext()->get();
  auto output = SerializerRef::deserializeFrom(esContext, private_->stream);

  return Utils::NewLocal<Uint32>(lwIsolate->toV8(), output);
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
                                            Local<ArrayBuffer> array_buffer) {
  LWNODE_RETURN_VOID;
}

void ValueDeserializer::TransferSharedArrayBuffer(
    uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {
  LWNODE_RETURN_VOID;
}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadDouble(double* value) {
  LWNODE_RETURN_FALSE;
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
  LWNODE_RETURN_FALSE;
}
}  // namespace v8
