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
  LWNODE_RETURN_NULLPTR;
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
  LWNODE_UNIMPLEMENT;
}

// struct ValueSerializer::PrivateData {
//   LWNODE_UNIMPLEMENT;
// };

ValueSerializer::ValueSerializer(Isolate* isolate)
    : ValueSerializer(isolate, nullptr) {
  LWNODE_UNIMPLEMENT;
}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate) {
  LWNODE_UNIMPLEMENT;
}
// : private_(
//       new PrivateData(reinterpret_cast<i::Isolate*>(isolate),
//       delegate)) {}

ValueSerializer::~ValueSerializer() {
  LWNODE_UNIMPLEMENT;
}

void ValueSerializer::WriteHeader() {
  LWNODE_UNIMPLEMENT;
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value){
    LWNODE_RETURN_MAYBE(bool)}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  return std::make_pair(nullptr, 0);
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {}

void ValueSerializer::WriteUint32(uint32_t value) {}

void ValueSerializer::WriteUint64(uint64_t value) {}

void ValueSerializer::WriteDouble(double value) {}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {}

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(
    Isolate* v8_isolate){LWNODE_RETURN_LOCAL(Object)}

MaybeLocal<WasmModuleObject> ValueDeserializer::Delegate::GetWasmModuleFromId(
    Isolate* v8_isolate, uint32_t id){LWNODE_RETURN_LOCAL(WasmModuleObject)}

MaybeLocal<SharedArrayBuffer> ValueDeserializer::Delegate::
    GetSharedArrayBufferFromId(Isolate* v8_isolate, uint32_t id) {
  LWNODE_RETURN_LOCAL(SharedArrayBuffer)
}

struct ValueDeserializer::PrivateData {};

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size)
    : ValueDeserializer(isolate, data, size, nullptr) {}

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size,
                                     Delegate* delegate) {}

ValueDeserializer::~ValueDeserializer() {}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  LWNODE_RETURN_MAYBE(bool)
}

void ValueDeserializer::SetSupportsLegacyWireFormat(
    bool supports_legacy_wire_format) {}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  LWNODE_RETURN_0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  LWNODE_RETURN_LOCAL(Value)
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
                                            Local<ArrayBuffer> array_buffer) {}

void ValueDeserializer::TransferSharedArrayBuffer(
    uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {}

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
}
