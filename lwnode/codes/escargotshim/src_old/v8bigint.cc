/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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
#include "v8utils.h"

namespace v8 {

using namespace EscargotShim;

// TODO:
Local<BigInt64Array> BigInt64Array::New(Local<ArrayBuffer> array_buffer,
                                        size_t byte_offset,
                                        size_t length) {
  NESCARGOT_UNIMPLEMENTED("");
  Local<ArrayBuffer> array =
      ArrayBuffer::New(IsolateShim::GetCurrent()->asIsolate(), length);
  return Utils::ToLocal<BigInt64Array>((BigInt64Array*)(*array));
}

// TODO:
Local<BigUint64Array> BigUint64Array::New(Handle<ArrayBuffer> arrayBuffer,
                                          size_t byteOffset,
                                          size_t length) {
  NESCARGOT_UNIMPLEMENTED("");
  Local<ArrayBuffer> array =
      ArrayBuffer::New(IsolateShim::GetCurrent()->asIsolate(), length);
  return Utils::ToLocal<BigUint64Array>((BigUint64Array*)(*array));
}

BigUint64Array* BigUint64Array::Cast(Value* value) {
  NESCARGOT_UNIMPLEMENTED("");
  NESCARGOT_ASSERT(value->IsArrayBuffer());
  return static_cast<BigUint64Array*>(value);
}

uint64_t BigInt::Uint64Value(bool* lossless) const {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}
int64_t BigInt::Int64Value(bool* lossless) const {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

Local<BigInt> v8::BigInt::New(Isolate* isolate, int64_t value) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<BigInt>();
}

Local<BigInt> v8::BigInt::NewFromUnsigned(Isolate* isolate, uint64_t value) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<BigInt>();
}

MaybeLocal<BigInt> v8::BigInt::NewFromWords(Local<Context> context,
                                            int sign_bit,
                                            int word_count,
                                            const uint64_t* words) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<BigInt>();
}

int BigInt::WordCount() const {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

void BigInt::ToWordsArray(int* sign_bit,
                          int* word_count,
                          uint64_t* words) const {
  NESCARGOT_UNIMPLEMENTED("");
}

// BigInt* BigInt::Cast(v8::Value* value) {
//   NESCARGOT_UNIMPLEMENTED("");
//   return nullptr;
// }

}  // namespace v8
