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

#include "serializer.h"
#include "context.h"
#include "es-helper.h"
#include "isolate.h"

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

// base on v8/src/objects/value-serializer.cc

static const uint32_t kLatestVersion = 13;

enum class SerializationTag : uint8_t {
  // version:uint32_t (if at beginning of data, sets version > 0)
  kVersion = 0xFF,
  // ignore
  kPadding = '\0',
  // refTableSize:uint32_t (previously used for sanity checks; safe to ignore)
  kVerifyObjectCount = '?',
  // Oddballs (no data).
  kTheHole = '-',
  kUndefined = '_',
  kNull = '0',
  kTrue = 'T',
  kFalse = 'F',
  // Number represented as 32-bit integer, ZigZag-encoded
  // (like sint32 in protobuf)
  kInt32 = 'I',
  // Number represented as 32-bit unsigned integer, varint-encoded
  // (like uint32 in protobuf)
  kUint32 = 'U',
  // Number represented as a 64-bit double.
  // Host byte order is used (N.B. this makes the format non-portable).
  kDouble = 'N',
  // BigInt. Bitfield:uint32_t, then raw digits storage.
  kBigInt = 'Z',
  // byteLength:uint32_t, then raw data
  kUtf8String = 'S',
  kOneByteString = '"',
  kTwoByteString = 'c',
  // Reference to a serialized object. objectID:uint32_t
  kObjectReference = '^',
  // Beginning of a JS object.
  kBeginJSObject = 'o',
  // End of a JS object. numProperties:uint32_t
  kEndJSObject = '{',
  // Beginning of a sparse JS array. length:uint32_t
  // Elements and properties are written as key/value pairs, like objects.
  kBeginSparseJSArray = 'a',
  // End of a sparse JS array. numProperties:uint32_t length:uint32_t
  kEndSparseJSArray = '@',
  // Beginning of a dense JS array. length:uint32_t
  // |length| elements, followed by properties as key/value pairs
  kBeginDenseJSArray = 'A',
  // End of a dense JS array. numProperties:uint32_t length:uint32_t
  kEndDenseJSArray = '$',
  // Date. millisSinceEpoch:double
  kDate = 'D',
  // Boolean object. No data.
  kTrueObject = 'y',
  kFalseObject = 'x',
  // Number object. value:double
  kNumberObject = 'n',
  // BigInt object. Bitfield:uint32_t, then raw digits storage.
  kBigIntObject = 'z',
  // String object, UTF-8 encoding. byteLength:uint32_t, then raw data.
  kStringObject = 's',
  // Regular expression, UTF-8 encoding. byteLength:uint32_t, raw data,
  // flags:uint32_t.
  kRegExp = 'R',
  // Beginning of a JS map.
  kBeginJSMap = ';',
  // End of a JS map. length:uint32_t.
  kEndJSMap = ':',
  // Beginning of a JS set.
  kBeginJSSet = '\'',
  // End of a JS set. length:uint32_t.
  kEndJSSet = ',',
  // Array buffer. byteLength:uint32_t, then raw data.
  kArrayBuffer = 'B',
  // Array buffer (transferred). transferID:uint32_t
  kArrayBufferTransfer = 't',
  // View into an array buffer.
  // subtag:ArrayBufferViewTag, byteOffset:uint32_t, byteLength:uint32_t
  // For typed arrays, byteOffset and byteLength must be divisible by the size
  // of the element.
  // Note: kArrayBufferView is special, and should have an ArrayBuffer (or an
  // ObjectReference to one) serialized just before it. This is a quirk arising
  // from the previous stack-based implementation.
  kArrayBufferView = 'V',
  // Shared array buffer. transferID:uint32_t
  kSharedArrayBuffer = 'u',
  // A wasm module object transfer. next value is its index.
  kWasmModuleTransfer = 'w',
  // The delegate is responsible for processing all following data.
  // This "escapes" to whatever wire format the delegate chooses.
  kHostObject = '\\',
  // A transferred WebAssembly.Memory object. maximumPages:int32_t, then by
  // SharedArrayBuffer tag and its data.
  kWasmMemoryTransfer = 'm',
  // A list of (subtag: ErrorTag, [subtag dependent data]). See ErrorTag for
  // details.
  kError = 'r',

  // The following tags are reserved because they were in use in Chromium before
  // the kHostObject tag was introduced in format version 13, at
  //   v8           refs/heads/master@{#43466}
  //   chromium/src refs/heads/master@{#453568}
  //
  // They must not be reused without a version check to prevent old values from
  // starting to deserialize incorrectly. For simplicity, it's recommended to
  // avoid them altogether.
  //
  // This is the set of tags that existed in SerializationTag.h at that time and
  // still exist at the time of this writing (i.e., excluding those that were
  // removed on the Chromium side because there should be no real user data
  // containing them).
  //
  // It might be possible to also free up other tags which were never persisted
  // (e.g. because they were used only for transfer) in the future.
  kLegacyReservedMessagePort = 'M',
  kLegacyReservedBlob = 'b',
  kLegacyReservedBlobIndex = 'i',
  kLegacyReservedFile = 'f',
  kLegacyReservedFileIndex = 'e',
  kLegacyReservedDOMFileSystem = 'd',
  kLegacyReservedFileList = 'l',
  kLegacyReservedFileListIndex = 'L',
  kLegacyReservedImageData = '#',
  kLegacyReservedImageBitmap = 'g',
  kLegacyReservedImageBitmapTransfer = 'G',
  kLegacyReservedOffscreenCanvas = 'H',
  kLegacyReservedCryptoKey = 'K',
  kLegacyReservedRTCCertificate = 'k',
};

ValueSerializer::ValueSerializer(v8::Isolate* isolate,
                                 v8::ValueSerializer::Delegate* delegate)
    : lwIsolate_(IsolateWrap::fromV8(isolate)), delegate_(delegate) {}

void ValueSerializer::WriteHeader() {
  WriteTag(SerializationTag::kVersion);
  WriteVarint(kLatestVersion);
}

bool ValueSerializer::WriteJSObject(ObjectRef* object) {
  if (LWNODE_UNLIKELY(out_of_memory_)) {
    return ThrowIfOutOfMemory();
  }
  if (ObjectRefHelper::getInternalFieldCount(object) > 0) {
    // TODO: WriteHostObject
    LWNODE_UNIMPLEMENT;
    return false;
  }

  auto esContext = lwIsolate_->GetCurrentContext()->get();
  uint32_t propertiesWritten = 0;
  WriteTag(SerializationTag::kBeginJSObject);
  Escargot::ValueVectorRef* keys = nullptr;
  EvalResult r = Evaluator::execute(
      lwIsolate_->GetCurrentContext()->get(),
      [](ExecutionStateRef* state,
         ObjectRef* object,
         Escargot::ValueVectorRef** keys) -> ValueRef* {
        *keys = object->ownPropertyKeys(state);
        return ValueRef::createUndefined();
      },
      object,
      &keys);
  LWNODE_CHECK(r.isSuccessful());

  for (size_t i = 0; i < keys->size(); i++) {
    auto propValueResult =
        ObjectRefHelper::getProperty(esContext, object, keys->at(i));
    LWNODE_CHECK(propValueResult.isSuccessful());
    bool success =
        WriteValue(keys->at(i)) && WriteValue(propValueResult.result);
    if (!success) {
      return false;
    }
    propertiesWritten++;
    WriteTag(SerializationTag::kEndJSObject);
    WriteVarint<uint32_t>(propertiesWritten);
  }
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteValue(ValueRef* value) {
  if (value->isUndefined()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isNull()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isBoolean()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isNumber()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isBigInt()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isArrayBufferObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isDataViewObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isString()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isTypedArrayObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isArrayObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isSymbol()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isFunctionObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isObject()) {
    return WriteJSObject(value->asObject());
  } else {
    LWNODE_UNIMPLEMENT;
  }
  return false;
}

// base on v8
void ValueSerializer::WriteTag(SerializationTag tag) {
  uint8_t raw_tag = static_cast<uint8_t>(tag);
  WriteRawBytes(&raw_tag, sizeof(raw_tag));
}

// base on v8
template <typename T>
void ValueSerializer::WriteVarint(T value) {
  // Writes an unsigned integer as a base-128 varint.
  // The number is written, 7 bits at a time, from the least significant to the
  // most significant 7 bits. Each byte, except the last, has the MSB set.
  // See also https://developers.google.com/protocol-buffers/docs/encoding
  static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                "Only unsigned integer types can be written as varints.");
  uint8_t stack_buffer[sizeof(T) * 8 / 7 + 1];
  uint8_t* next_byte = &stack_buffer[0];
  do {
    *next_byte = (value & 0x7F) | 0x80;
    next_byte++;
    value >>= 7;
  } while (value);
  *(next_byte - 1) &= 0x7F;
  WriteRawBytes(stack_buffer, next_byte - stack_buffer);
}

// base on v8
void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  uint8_t* dest;
  if (ReserveRawBytes(length).To(&dest) && length > 0) {
    memcpy(dest, source, length);
  }
}

// base on v8
Maybe<uint8_t*> ValueSerializer::ReserveRawBytes(size_t bytes) {
  size_t old_size = buffer_size_;
  size_t new_size = old_size + bytes;
  if (LWNODE_UNLIKELY(new_size > buffer_capacity_)) {
    bool ok;
    if (!ExpandBuffer(new_size).To(&ok)) {
      return Nothing<uint8_t*>();
    }
  }
  buffer_size_ = new_size;
  return Just(&buffer_[old_size]);
}

// base on v8
Maybe<bool> ValueSerializer::ExpandBuffer(size_t required_capacity) {
  LWNODE_CHECK(required_capacity > buffer_capacity_);
  size_t requested_capacity =
      std::max(required_capacity, buffer_capacity_ * 2) + 64;
  size_t provided_capacity = 0;
  void* new_buffer = nullptr;
  if (delegate_) {
    new_buffer = delegate_->ReallocateBufferMemory(
        buffer_, requested_capacity, &provided_capacity);
  } else {
    new_buffer = realloc(buffer_, requested_capacity);
    provided_capacity = requested_capacity;
  }
  if (new_buffer) {
    LWNODE_CHECK(provided_capacity >= requested_capacity);
    buffer_ = reinterpret_cast<uint8_t*>(new_buffer);
    buffer_capacity_ = provided_capacity;
    return Just(true);
  } else {
    out_of_memory_ = true;
    return Nothing<bool>();
  }
}

bool ValueSerializer::ThrowIfOutOfMemory() {
  if (out_of_memory_) {
    // TODO: thrwo internal error
    return false;
  }
  return true;
}

}  // namespace EscargotShim
