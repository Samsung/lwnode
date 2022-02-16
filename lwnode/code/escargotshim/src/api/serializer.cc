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

#include <cmath>

#include "base.h"
#include "context.h"
#include "es-helper.h"
#include "isolate.h"
#include "serializer.h"

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

// base on v8/src/objects/value-serializer.cc
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

enum class ArrayBufferViewTag : uint8_t {
  kInt8Array = 'b',
  kUint8Array = 'B',
  kUint8ClampedArray = 'C',
  kInt16Array = 'w',
  kUint16Array = 'W',
  kInt32Array = 'd',
  kUint32Array = 'D',
  kFloat32Array = 'f',
  kFloat64Array = 'F',
  kBigInt64Array = 'q',
  kBigUint64Array = 'Q',
  kDataView = '?',
};

ValueSerializer::ValueSerializer(IsolateWrap* lwIsolate,
                                 v8::ValueSerializer::Delegate* delegate)
    : lwIsolate_(lwIsolate), delegate_(delegate) {
  LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Create serializer");
}

void ValueSerializer::WriteHeader() {}

bool ValueSerializer::WriteValue(ValueRef* value) {
  if (value->isUndefined()) {
    WriteTag(SerializationTag::kUndefined);
  } else if (value->isNull()) {
    WriteTag(SerializationTag::kNull);
  } else if (value->isBoolean()) {
    return WriteBoolean(value->asBoolean());
  } else if (value->isUInt32()) {
    return WriteUint32(value->asUInt32());
  } else if (value->isInt32()) {
    return WriteInt32(value->asInt32());
  } else if (value->isNumber()) {
    return WriteNumber(value->asNumber());
  } else if (value->isBigInt()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isArrayBufferObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isDataViewObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isString()) {
    WriteString(value->asString());
  } else if (value->isTypedArrayObject()) {
    return WriteTypedArrayObject(value->asArrayBufferView());
  } else if (value->isArrayObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isSymbol()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isFunctionObject()) {
    LWNODE_UNIMPLEMENT;
  } else if (value->isObject()) {
    if (!WriteObject(value->asObject())) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot write object");
      return false;
    }
  } else {
    LWNODE_UNIMPLEMENT;
    return false;
  }
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteUint32(uint32_t value) {
  WriteTag(SerializationTag::kUint32);
  WriteVarint<uint32_t>(value);
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteInt32(int32_t value) {
  WriteTag(SerializationTag::kInt32);
  WriteZigZag<int32_t>(value);
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteNumber(double value) {
  WriteTag(SerializationTag::kDouble);
  WriteRawBytes(&value, sizeof(value));
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteBoolean(bool value) {
  if (value) {
    WriteTag(SerializationTag::kTrue);
  } else {
    WriteTag(SerializationTag::kFalse);
  }
  return ThrowIfOutOfMemory();
}

// base on v8
void ValueSerializer::WriteTag(SerializationTag tag) {
  uint8_t raw_tag = static_cast<uint8_t>(tag);
  LWNODE_CALL_TRACE_ID_LOG(SERIALIZER_TAG, "Write Tag %c", (char)tag);
  WriteRawBytes(&raw_tag, sizeof(raw_tag));
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  uint8_t* dest = ReserveRawBytes(length);
  if (dest && length > 0) {
    memcpy(dest, source, length);
  }
}

// base on v8
uint8_t* ValueSerializer::ReserveRawBytes(size_t bytes) {
  size_t old_size = buffer_.size;
  size_t new_size = old_size + bytes;
  if (LWNODE_UNLIKELY(new_size > buffer_.capacity)) {
    if (!ExpandBuffer(new_size)) {
      return nullptr;
    }
  }
  buffer_.size = new_size;
  return &buffer_.data[old_size];
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

template <typename T>
void ValueSerializer::WriteZigZag(T value) {
  // Writes a signed integer as a varint using ZigZag encoding (i.e. 0 is
  // encoded as 0, -1 as 1, 1 as 2, -2 as 3, and so on).
  // See also https://developers.google.com/protocol-buffers/docs/encoding
  // Note that this implementation relies on the right shift being arithmetic.
  static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                "Only signed integer types can be written as zigzag.");
  using UnsignedT = typename std::make_unsigned<T>::type;
  WriteVarint((static_cast<UnsignedT>(value) << 1) ^
              (value >> (8 * sizeof(T) - 1)));
}

void ValueSerializer::WriteString(StringRef* string) {
  auto bufferData = string->stringBufferAccessData();
  if (bufferData.has8BitContent) {
    WriteTag(SerializationTag::kOneByteString);
    WriteVarint<uint32_t>(bufferData.length);
    WriteRawBytes(bufferData.buffer, bufferData.length * sizeof(uint8_t));
  } else {
    WriteTag(SerializationTag::kTwoByteString);
    WriteVarint<uint32_t>(bufferData.length);
    WriteRawBytes(bufferData.buffer, bufferData.length * sizeof(uint16_t));
  }
}

bool ValueSerializer::WriteHostObject(ObjectRef* object) {
  WriteTag(SerializationTag::kHostObject);
  if (!delegate_) {
    // @note deps/v8/src/objects/value-serializer.cc(1006) throws an Error
    LWNODE_CHECK(false);
    return false;
  }

  v8::Isolate* v8_isolate = lwIsolate_->toV8();
  Maybe<bool> result =
      delegate_->WriteHostObject(v8_isolate, Utils::ToLocal<Object>(object));

  LWNODE_CHECK(!result.IsNothing());
  LWNODE_CHECK(result.ToChecked());
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteObject(ObjectRef* object) {
  if (ObjectRefHelper::getInternalFieldCount(object) > 0) {
    return WriteHostObject(object);
  }

  auto esContext = lwIsolate_->GetCurrentContext()->get();
  uint32_t propertiesWritten = 0;
  WriteTag(SerializationTag::kBeginJSObject);
  Escargot::ValueVectorRef* keys = nullptr;
  EvalResult r = Evaluator::execute(
      esContext,
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
  }

  WriteTag(SerializationTag::kEndJSObject);
  WriteVarint<uint32_t>(propertiesWritten);

  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteArrayBuffer(size_t length, uint8_t* bytes) {
  WriteTag(SerializationTag::kArrayBuffer);
  WriteVarint<uint32_t>(length);
  WriteRawBytes(bytes, length);
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteArrayBufferView(
    ArrayBufferViewRef* arrayBufferView) {
  WriteTag(SerializationTag::kArrayBufferView);

  ArrayBufferViewTag typeTag = ArrayBufferViewTag::kInt8Array;

  if (arrayBufferView->isDataViewObject()) {
    typeTag = ArrayBufferViewTag::kDataView;
  }
#define TYPED_ARRAY_CASE(Type, type, TYPE, ctype)                              \
  else if (arrayBufferView->is##Type##ArrayObject()) {                         \
    typeTag = ArrayBufferViewTag::k##Type##Array;                              \
  }
  TYPED_ARRAYS(TYPED_ARRAY_CASE)
#undef TYPED_ARRAY_CASE
  else {
    LWNODE_DLOG_ERROR("Serializer: Invalid buffer type");
    return false;
  }

  WriteVarint(static_cast<uint8_t>(typeTag));
  WriteVarint(static_cast<uint32_t>(arrayBufferView->byteOffset()));
  WriteVarint(static_cast<uint32_t>(arrayBufferView->arrayLength()));
  return ThrowIfOutOfMemory();
}

bool ValueSerializer::WriteTypedArrayObject(
    Escargot::ArrayBufferViewRef* bufferView) {
  auto result =
      WriteArrayBuffer(bufferView->byteLength(), bufferView->rawBuffer());
  return result && WriteArrayBufferView(bufferView);
}

// base on v8
bool ValueSerializer::ExpandBuffer(size_t required_capacity) {
  LWNODE_CHECK(required_capacity > buffer_.capacity);
  size_t requested_capacity =
      std::max(required_capacity, buffer_.capacity * 2) + 64;
  size_t provided_capacity = 0;
  void* new_buffer = nullptr;
  if (delegate_) {
    new_buffer = delegate_->ReallocateBufferMemory(
        buffer_.data, requested_capacity, &provided_capacity);
  } else {
    new_buffer = realloc(buffer_.data, requested_capacity);
    provided_capacity = requested_capacity;
  }
  if (new_buffer) {
    LWNODE_CHECK(provided_capacity >= requested_capacity);
    buffer_.data = reinterpret_cast<uint8_t*>(new_buffer);
    buffer_.capacity = provided_capacity;
    return true;
  } else {
    out_of_memory_ = true;
    return false;
  }
}

bool ValueSerializer::ThrowIfOutOfMemory() {
  if (out_of_memory_) {
    LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "out of memory");
    ThrowDataCloneError();
    return false;
  }
  return true;
}

void ValueSerializer::ThrowDataCloneError() {
  auto errorMessage =
      StringRef::createFromASCII("Data cannot be cloned, out of memory.");
  if (delegate_) {
    delegate_->ThrowDataCloneError(
        Utils::NewLocal<String>(lwIsolate_->toV8(), errorMessage));
  } else {
    auto lwContext = lwIsolate_->GetCurrentContext()->get();
    lwIsolate_->ScheduleThrow(ExceptionHelper::createErrorObject(
        lwContext, ErrorObjectRef::Code::RangeError, errorMessage));
  }

  if (lwIsolate_->sholdReportPendingMessage(false)) {
    lwIsolate_->ReportPendingMessages();
  }
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "buffer size: %zu", buffer_.size);

  uint8_t* buffer = nullptr;
  auto size = buffer_.size;
  if (size > 0) {
    if (delegate_) {
      size_t allocatedSize = 0;
      buffer = static_cast<uint8_t*>(
          delegate_->ReallocateBufferMemory(buffer, size, &allocatedSize));
      LWNODE_CHECK(size == allocatedSize);
    } else {
      buffer = static_cast<uint8_t*>(malloc(size * sizeof(uint8_t)));
    }

    memcpy(buffer, buffer_.data, size);
  }

  free(buffer_.data);
  buffer_.size = 0;

  return std::make_pair(buffer, size);
}

ValueDeserializer::ValueDeserializer(IsolateWrap* lwIsolate,
                                     v8::ValueDeserializer::Delegate* delegate,
                                     const uint8_t* data,
                                     const size_t size)
    : lwIsolate_(lwIsolate), delegate_(delegate), buffer_(data, size) {
  LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Create deserializer (%zu)", size);
}

OptionalRef<ValueRef> ValueDeserializer::ReadValue() {
  SerializationTag tag;
  if (!ReadTag(tag)) {
    LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read tag");
    return OptionalRef<ValueRef>();
  }

  LWNODE_CALL_TRACE_ID_LOG(SERIALIZER_TAG, "Read Tag %c", (char)tag);

  if (tag == SerializationTag::kNull) {
    return OptionalRef<ValueRef>(ValueRef::createNull());
  } else if (tag == SerializationTag::kUndefined) {
    return OptionalRef<ValueRef>(ValueRef::createUndefined());
  } else if (tag == SerializationTag::kTrue) {
    return OptionalRef<ValueRef>(ValueRef::create(true));
  } else if (tag == SerializationTag::kFalse) {
    return OptionalRef<ValueRef>(ValueRef::create(false));
  } else if (tag == SerializationTag::kDouble) {
    double number = .0;
    if (!ReadDouble(number)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read double value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(ValueRef::create(number));
  } else if (tag == SerializationTag::kOneByteString) {
    StringRef* string;
    if (!ReadOneByteString(string)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read one byte string value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(string);
  } else if (tag == SerializationTag::kTwoByteString) {
    StringRef* string;
    if (!ReadTwoByteString(string)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read two byte string value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(string);
  } else if (tag == SerializationTag::kUint32) {
    uint32_t value = 0;
    if (!ReadVarint<uint32_t>(value)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read uint32 value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(ValueRef::create(value));
  } else if (tag == SerializationTag::kInt32) {
    int32_t value = 0;
    if (!ReadZigZag<int32_t>(value)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read int32 value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(ValueRef::create(value));
  } else if (tag == SerializationTag::kBeginJSObject) {
    auto esContext = lwIsolate_->GetCurrentContext()->get();
    auto object = ObjectRefHelper::create(esContext);
    if (!ReadObject(object)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read object value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(object);
  } else if (tag == SerializationTag::kHostObject) {
    ObjectRef* esObject = nullptr;
    if (!ReadHostObject(esObject)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read host object value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(esObject);
  } else if (tag == SerializationTag::kArrayBuffer) {
    ValueRef* arrayBuffer = nullptr;
    if (!ReadJsArrayBuffer(arrayBuffer)) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read array buffer value");
      return OptionalRef<ValueRef>();
    }
    return OptionalRef<ValueRef>(arrayBuffer);
  } else {
    LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Fail: %c", (char)tag);
    LWNODE_UNIMPLEMENT;
  }

  return OptionalRef<ValueRef>();
}

// base on v8
bool ValueDeserializer::ReadTag(SerializationTag& tag) {
  do {
    if (buffer_.isOverflow()) {
      return false;
    }
    tag = static_cast<SerializationTag>(buffer_.currentPositionData());
    buffer_.position++;
  } while (tag == SerializationTag::kPadding);
  return true;
}

bool ValueDeserializer::CheckTag(SerializationTag check) {
  SerializationTag tag;
  size_t curPosition = buffer_.position;
  do {
    if (buffer_.isOverflow()) {
      return false;
    }
    tag = static_cast<SerializationTag>(buffer_.data[curPosition]);
    curPosition++;
  } while (tag == SerializationTag::kPadding);

  return tag == check;
}

bool ValueDeserializer::ReadUint32(uint32_t*& value) {
  if (!CheckTag(SerializationTag::kUint32)) {
    return false;
  }

  OptionalRef<ValueRef> valueRef = ReadValue();
  if (valueRef) {
    *value = valueRef->asUInt32();
  }
  return valueRef;
}

template <typename T>
bool ValueDeserializer::ReadVarint(T& value) {
  // Reads an unsigned integer as a base-128 varint.
  // The number is written, 7 bits at a time, from the least significant to the
  // most significant 7 bits. Each byte, except the last, has the MSB set.
  // If the varint is larger than T, any more significant bits are discarded.
  // See also https://developers.google.com/protocol-buffers/docs/encoding
  static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                "Only unsigned integer types can be read as varints.");
  value = 0;
  unsigned shift = 0;
  bool has_another_byte;
  do {
    if (buffer_.isOverflow()) {
      return false;
    }
    uint8_t byte = buffer_.currentPositionData();
    if (V8_LIKELY(shift < sizeof(T) * 8)) {
      value |= static_cast<T>(byte & 0x7F) << shift;
      shift += 7;
    }
    has_another_byte = byte & 0x80;
    buffer_.position++;
  } while (has_another_byte);
  return true;
}

template <typename T>
bool ValueDeserializer::ReadZigZag(T& value) {
  // Writes a signed integer as a varint using ZigZag encoding (i.e. 0 is
  // encoded as 0, -1 as 1, 1 as 2, -2 as 3, and so on).
  // See also https://developers.google.com/protocol-buffers/docs/encoding
  static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                "Only signed integer types can be read as zigzag.");
  using UnsignedT = typename std::make_unsigned<T>::type;
  UnsignedT unsigned_value;
  if (!ReadVarint<UnsignedT>(unsigned_value)) {
    return false;
  }
  value = static_cast<T>((unsigned_value >> 1) ^
                         -static_cast<T>(unsigned_value & 1));
  return true;
}

bool ValueDeserializer::ReadDouble(double& value) {
  if (buffer_.position > buffer_.size - sizeof(double)) {
    return false;
  }
  value = 0;
  memcpy(&value, buffer_.currentPositionAddress(), sizeof(double));
  buffer_.position += sizeof(double);
  if (std::isnan(value)) {
    return false;
  }
  return true;
}

bool ValueDeserializer::ReadOneByteString(StringRef*& string) {
  uint32_t length = 0;
  const uint8_t* data;
  if (!ReadVarint<uint32_t>(length) || !ReadRawBytes(length, data)) {
    return false;
  }
  string = StringRef::createFromUTF8(reinterpret_cast<const char*>(data),
                                     static_cast<size_t>(length));
  return true;
}

bool ValueDeserializer::ReadTwoByteString(StringRef*& string) {
  uint32_t length = 0;
  const uint8_t* data;
  if (!ReadVarint<uint32_t>(length) ||
      !ReadRawBytes(length * sizeof(uint16_t), data)) {
    return false;
  }
  string = StringRef::createFromUTF16(reinterpret_cast<const char16_t*>(data),
                                      static_cast<size_t>(length));
  return true;
}

bool ValueDeserializer::ReadObject(ObjectRef* object) {
  size_t propertiesRead = 0;

  while (!CheckTag(SerializationTag::kEndJSObject)) {
    auto key = ReadValue();
    if (!key.hasValue() || key.get()->isUndefinedOrNull()) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read key of object");
      return false;
    }
    auto value = ReadValue();
    if (!value.hasValue()) {
      LWNODE_CALL_TRACE_ID_LOG(SERIALIZER, "Cannot read value of object");
      return false;
    }

    ObjectRefHelper::setProperty(
        lwIsolate_->GetCurrentContext()->context()->get(),
        object,
        key.get(),
        value.get());

    propertiesRead++;
  }

  SerializationTag tag;
  if (!ReadTag(tag)) {
    return false;
  }
  if (tag == SerializationTag::kEndJSObject) {
    uint32_t propertiesWritten;
    if (ReadVarint<uint32_t>(propertiesWritten)) {
      return propertiesRead == propertiesWritten;
    }
  }
  return false;
}

bool ValueDeserializer::ReadHostObject(ObjectRef*& esObject) {
  esObject = nullptr;
  if (!delegate_) {
    return false;
  }
  v8::Isolate* v8_isolate = lwIsolate_->toV8();
  v8::Local<v8::Object> object;
  if (!delegate_->ReadHostObject(v8_isolate).ToLocal(&object)) {
    return false;
  }

  esObject = VAL(*object)->value()->asObject();
  return true;
}

bool ValueDeserializer::ReadJsArrayBuffer(ValueRef*& value) {
  ArrayBufferObjectRef* arrayBuffer = nullptr;
  if (!ReadArrayBuffer(arrayBuffer)) {
    return false;
  }

  if (CheckTag(SerializationTag::kArrayBufferView)) {
    SerializationTag tag;
    ReadTag(tag);
    ArrayBufferViewRef* arrayBufferView = nullptr;
    if (!ReadArrayBufferView(arrayBufferView, arrayBuffer)) {
      return false;
    }
    value = arrayBufferView;
    return true;
  }
  value = arrayBuffer;
  return true;
}

bool ValueDeserializer::ReadArrayBuffer(
    ArrayBufferObjectRef*& arrayBufferObject) {
  uint32_t length = 0;
  const uint8_t* bytes = nullptr;
  if (!ReadVarint<uint32_t>(length) || !ReadRawBytes(length, bytes)) {
    return false;
  }

  auto esContext = lwIsolate_->GetCurrentContext()->get();
  EvalResult r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, size_t byteLength) -> ValueRef* {
        auto arrayBuffer = ArrayBufferObjectRef::create(esState);
        arrayBuffer->allocateBuffer(esState, byteLength);
        return arrayBuffer;
      },
      (size_t)length);
  LWNODE_CHECK(r.isSuccessful());

  auto backingStore =
      BackingStoreRef::createDefaultNonSharedBackingStore(length);
  arrayBufferObject = r.result->asArrayBufferObject();
  arrayBufferObject->attachBuffer(backingStore);

  memcpy(arrayBufferObject->rawBuffer(), bytes, length);

  return true;
}

bool ValueDeserializer::ReadArrayBufferView(
    ArrayBufferViewRef*& arrayBufferView, ArrayBufferObjectRef* abo) {
  uint8_t tag = 0;
  uint32_t byteOffset = 0;
  uint32_t byteLength = 0;
  uint32_t arrayLength = 0;

  if (!ReadVarint<uint8_t>(tag) || !ReadVarint<uint32_t>(byteLength) ||
      !ReadVarint<uint32_t>(arrayLength)) {
    return false;
  }

  auto esContext = lwIsolate_->GetCurrentContext()->get();

  switch (static_cast<ArrayBufferViewTag>(tag)) {
    case ArrayBufferViewTag::kDataView: {
      return false;
    }
#define TYPED_ARRAY_CASE(Type, type, TYPE, ctype)                              \
  case ArrayBufferViewTag::k##Type##Array:                                     \
    arrayBufferView = ArrayBufferHelper::createView<Type##ArrayObjectRef>(     \
        esContext,                                                             \
        abo,                                                                   \
        byteOffset,                                                            \
        arrayLength,                                                           \
        ArrayBufferHelper::ArrayType::kExternal##Type##Array);                 \
    return true;
      TYPED_ARRAYS(TYPED_ARRAY_CASE)
#undef TYPED_ARRAY_CASE
    default:
      return false;
  }

  return false;
}

bool ValueDeserializer::ReadRawBytes(size_t size, const uint8_t*& data) {
  if (size > buffer_.size - buffer_.position) {
    return false;
  }

  data = buffer_.currentPositionAddress();
  buffer_.position += size;
  return true;
}

}  // namespace EscargotShim

#endif
