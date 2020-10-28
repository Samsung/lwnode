/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "v8.h"

namespace v8 {

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(
    Isolate* isolate) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Object>();
}

MaybeLocal<WasmModuleObject> ValueDeserializer::Delegate::GetWasmModuleFromId(
    Isolate* isolate, uint32_t transfer_id) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<WasmModuleObject>();
}

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size,
                                     Delegate* delegate) {
  NESCARGOT_UNIMPLEMENTED("");
}

ValueDeserializer::~ValueDeserializer() {
  // Intentionally left empty to suppress warning C4722.
}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Value>();
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
                                            Local<ArrayBuffer> array_buffer) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueDeserializer::TransferSharedArrayBuffer(
    uint32_t id, Local<SharedArrayBuffer> shared_array_buffer) {
  NESCARGOT_UNIMPLEMENTED("");
}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool ValueDeserializer::ReadDouble(double* value) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

MaybeLocal<SharedArrayBuffer>
ValueDeserializer::Delegate::GetSharedArrayBufferFromId(Isolate* v8_isolate,
                                                        uint32_t id) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<SharedArrayBuffer>();
}

void ValueDeserializer::SetExpectInlineWasm(bool expect_inline_wasm) {
  NESCARGOT_UNIMPLEMENTED("");
}

}  // namespace v8
