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

Maybe<bool> ValueSerializer::Delegate::WriteHostObject(Isolate* isolate,
                                                       Local<Object> object) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

Maybe<uint32_t> ValueSerializer::Delegate::GetSharedArrayBufferId(
    Isolate* isolate, Local<SharedArrayBuffer> shared_array_buffer) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<unsigned int>();
}

void* ValueSerializer::Delegate::ReallocateBufferMemory(void* old_buffer,
                                                        size_t size,
                                                        size_t* actual_size) {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
  NESCARGOT_UNIMPLEMENTED("");
}

ValueSerializer::ValueSerializer(Isolate* isolate) {
  NESCARGOT_UNIMPLEMENTED("");
}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate) {
  NESCARGOT_UNIMPLEMENTED("");
}

ValueSerializer::~ValueSerializer() {
  // Intentionally left empty to suppress warning C4722.
}

void ValueSerializer::WriteHeader() {
  NESCARGOT_UNIMPLEMENTED("");
}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  NESCARGOT_UNIMPLEMENTED("");
  return std::pair<uint8_t*, size_t>();
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueSerializer::WriteUint32(uint32_t value) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueSerializer::WriteUint64(uint64_t value) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueSerializer::WriteDouble(double value) {
  NESCARGOT_UNIMPLEMENTED("");
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  NESCARGOT_UNIMPLEMENTED("");
}

Maybe<uint32_t> ValueSerializer::Delegate::GetWasmModuleTransferId(
    Isolate* v8_isolate, Local<WasmModuleObject> module) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<uint32_t>();
}

}  // namespace v8
