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

#include "escargotutil.h"
#include "jsutils.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

// Converts an object to a UTF-8-encoded character array.  Useful if
// you want to print the object.  If conversion to a string fails
// (e.g. due to an exception in the toString() method of the object)
// then the length() method returns 0 and the * operator returns
// NULL.
String::Utf8Value::Utf8Value(Handle<v8::Value> obj)
    : Utf8Value(IsolateShim::GetCurrent()->asIsolate(), obj) {}

String::Utf8Value::Utf8Value(Isolate* isolate, Local<v8::Value> obj)
    : _str(nullptr), _length(0) {
  NESCARGOT_ASSERT(isolate);

  Local<String> strObj = FromMaybe(
      obj->ToString(IsolateShim::GetCurrent()->currentContext()->asContext()));
  if (strObj.IsEmpty()) {
    return;
  }

  std::string str =
      strObj->asJsValueRef()->asString()->toStdUTF8String(REPLACE_INVALID_UTF8);
  size_t len = str.length();

  // v8 API requires a char* for the string
  char* buffer = new char[len + 1];
  strncpy(buffer, str.data(), len);

  buffer[len] = '\0';
  _str = buffer;
  _length = static_cast<int>(len);
}

String::Utf8Value::~Utf8Value() {
  if (_str != nullptr) {
    delete[] _str;
    _str = nullptr;
    _length = 0;
  }
}

// Converts an object to a two-byte (UTF-16-encoded) string.
// If conversion to a string fails (eg. due to an exception in the toString()
// method of the object) then the length() method returns 0 and the * operator
// returns NULL.
String::Value::Value(Handle<v8::Value> obj)
    : Value(IsolateShim::GetCurrent()->asIsolate(), obj) {}

String::Value::Value(Isolate* isolate, Local<v8::Value> obj)
    : _str(nullptr), _length(0) {
  NESCARGOT_ASSERT(isolate);
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("VERIFYING IMPLEMENTAION");

  Handle<String> str = FromMaybe(
      obj->ToString(IsolateShim::GetCurrent()->currentContext()->asContext()));
  if (str.IsEmpty()) {
    return;
  }

  _length = str->Length();
  _str = new uint16_t[_length + 1];
  str->Write(_str);
}

String::Value::~Value() {
  if (_str != nullptr) {
    delete[] _str;
  }
}

// Returns the number of characters (UTF-16 code units) in this string.
int String::Length() const {
  return asJsValueRef()->asString()->length();
}

// Returns the number of bytes in the UTF-8 encoded
// representation of this string.
int String::Utf8Length() const {
  return Utf8Length(nullptr);
}

int String::Utf8Length(Isolate* isolate) const {
  std::string self =
      asJsValueRef()->asString()->toStdUTF8String(REPLACE_INVALID_UTF8);
  return self.length();
}

template <typename T>
static void CopyString(uint16_t* dest, const void* src, int start, int length) {
  size_t size = sizeof(T);
  if (size == 1) {
    for (int i = 0; i < length; i++) {
      *(dest + i) = 0;  // clean whole destination word
      memcpy(dest + i, (T*)src + start + i, size);
    }
  } else {
    for (int i = 0; i < length; i++) {
      memcpy(dest + i, (T*)src + start + i, size);
    }
  }
}

int String::Write(uint16_t* buffer, int start, int length, int options) const {
  return Write(nullptr, buffer, start, length, options);
}

int String::Write(Isolate* isolate,
                  uint16_t* buffer,
                  int start,
                  int length,
                  int options) const {
  JsStringRef str = asJsValueRef()->asString();
  auto strBuffer = str->stringBufferAccessData();

  if (buffer == nullptr || start < 0 ||
      strBuffer.length < (static_cast<size_t>(start))) {
    return 0;
  }

  size_t len = static_cast<size_t>(length);
  size_t count = MIN(len, strBuffer.length - start);

  if (strBuffer.has8BitContent) {
    CopyString<uint8_t>(buffer, strBuffer.buffer, start, count);
  } else {
    CopyString<uint16_t>(buffer, strBuffer.buffer, start, count);
  }
  if (!(options & String::NO_NULL_TERMINATION) &&
      (length == -1 || count < len)) {
    buffer[count] = '\0';
  }
  return count;
}

int String::WriteOneByte(uint8_t* buffer,
                         int start,
                         int length,
                         int options) const {
  return WriteOneByte(nullptr, buffer, start, length, options);
}

int String::WriteOneByte(Isolate* isolate,
                         uint8_t* buffer,
                         int start,
                         int length,
                         int options) const {
  JsStringRef self = asJsValueRef()->asString();
  auto accessData = self->stringBufferAccessData();
  size_t len = static_cast<size_t>(length);
  size_t count = MIN(len, accessData.length - start);

  memcpy(buffer, (uint8_t*)accessData.buffer + start, count);

  if (!(options & String::NO_NULL_TERMINATION) &&
      (length == -1 || count < len)) {
    buffer[count] = '\0';
  }
  return count;
}

static bool parseUtf8String(const char* src,
                            int srcBufferSize,
                            int destBufferSize,
                            int& nbytes,
                            int& nchars) {
  const char* cur = &src[0];
  const char* end = &src[srcBufferSize];
  int len = 0, numCharacters = 0;

  while (cur != end) {
    u_char c = *cur;
    int n;
    if ((c & 0x80) == 0) {
      n = 1;
    } else if ((c & 0xE0) == 0xC0) {
      n = 2;
    } else if ((c & 0xF0) == 0xE0) {
      n = 3;
    } else if ((c & 0xF8) == 0xF0) {
      n = 4;
    } else {
      NESCARGOT_LOG_ERROR("invalid UTF-8: ignore more bytes");
      return false;
    }

    if (end - cur < n) {
      NESCARGOT_LOG_ERROR("invalid UTF-8: string too short");
      return false;
    }

    if (0 > destBufferSize - len - n) {
      // deflation abortion since a buffer will run out.
      break;
    }

    for (int i = 1; i < n; ++i) {
      if ((cur[i] & 0xC0) != 0x80) {
        return -1;
      }
    }
    len += n;
    cur += n;
    numCharacters++;
  }

  nbytes = len;
  nchars = numCharacters;

  return true;
}

int String::WriteUtf8(Isolate* isolate,
                      char* buffer,
                      int length,
                      int* ncharsRef,
                      int options) const {
  if (ncharsRef) {
    *ncharsRef = 0;
  }

  if (buffer == nullptr || length == 0) {
    return 0;
  }

  bool writeNull = !(options & String::NO_NULL_TERMINATION);
  int writeOptions = Escargot::StringRef::WriteOptions::NoOptions;
  if (options & String::REPLACE_INVALID_UTF8) {
    writeOptions = Escargot::StringRef::WriteOptions::ReplaceInvalidUtf8;
  }

  if (options & (HINT_MANY_WRITES_EXPECTED | PRESERVE_ONE_BYTE_NULL)) {
    NESCARGOT_UNIMPLEMENTED("Warning: the given option is not supported\n");
  }

  std::string thisString =
      asJsValueRef()->asString()->toStdUTF8String(writeOptions);

  int bufferCapacity = length;
  if (bufferCapacity == -1) {
    bufferCapacity = thisString.length();
  }

  const char* strBuffer = thisString.data();
  int nbytes = 0, nchar = 0;
  if (!parseUtf8String(
          strBuffer, thisString.size(), bufferCapacity, nbytes, nchar)) {
    NESCARGOT_LOG_WARN("Warning: An invalid UTF8 string is given\n");
    return 0;
  }

  memcpy(buffer, strBuffer, nbytes);

  // Write null if the string is fully copied and there is enough space in the
  // output buffer
  if (writeNull && (length == -1 ||
                    thisString.size() < static_cast<size_t>(bufferCapacity))) {
    buffer[nbytes] = '\0';
    nbytes++;
  }

  if (ncharsRef) {
    *ncharsRef = nchar;
  }

  return nbytes;
}

int String::WriteUtf8(char* buffer,
                      int length,
                      int* ncharsRef,
                      int options) const {
  return WriteUtf8(nullptr, buffer, length, ncharsRef, options);
}

Local<String> String::Empty(Isolate* isolate) {
  return Local<String>::New(isolate, CreateJsEmptyString());
}

String* String::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsString());
  return static_cast<String*>(obj);
}

MaybeLocal<String> String::NewFromUtf8(Isolate* isolate,
                                       const char* data,
                                       v8::NewStringType type,
                                       int length) {
  if (length == 0) {
    return Local<String>(CreateJsEmptyString());
  }
  if (length > String::kMaxLength) {
    return MaybeLocal<String>();
  }

  if (length < 0) {
    length = strlen(data);
  }

  JsValueRef str = CreateJsStringFromUTF8(data, length);
  return Local<String>::New(isolate, str);
}

Local<String> String::NewFromUtf8(Isolate* isolate,
                                  const char* data,
                                  NewStringType type,
                                  int length) {
  return FromMaybe(
      NewFromUtf8(isolate, data, static_cast<v8::NewStringType>(type), length));
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate,
                                          const uint8_t* data,
                                          v8::NewStringType type,
                                          int length) {
  if (length == 0) {
    return Local<String>(CreateJsEmptyString());
  }
  if (length > String::kMaxLength) {
    return MaybeLocal<String>();
  }

  if (length < 0) {
    length = strlen((const char*)data);
  }

  JsStringRef strRef = CreateJsStringFromLatin1(data, length);

  return Local<String>::New(isolate, strRef);
}

Local<String> String::NewFromOneByte(Isolate* isolate,
                                     const uint8_t* data,
                                     NewStringType type,
                                     int length) {
  return FromMaybe(NewFromOneByte(
      isolate, data, static_cast<v8::NewStringType>(type), length));
}

static size_t WideStrLen(const uint16_t* p) {
  size_t len = 0;
  while (*p++) {
    len++;
  }
  return len;
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          v8::NewStringType type,
                                          int length) {
  if (length < 0) {
    length = WideStrLen(data);
  }

  JsStringRef strRef =
      CreateJsStringFromUTF16(reinterpret_cast<const char16_t*>(data), length);

  return Local<String>::New(isolate, strRef);
}

Local<String> String::NewFromTwoByte(Isolate* isolate,
                                     const uint16_t* data,
                                     NewStringType type,
                                     int length) {
  return FromMaybe(NewFromTwoByte(
      isolate, data, static_cast<v8::NewStringType>(type), length));
}

Local<String> String::Concat(Isolate* isolate,
                             Local<String> left,
                             Local<String> right) {
  JsContextRef context = GetCurrentJsContextRef();
  JsValueRef args[] = {right->asJsValueRef()->asString()};
  JsValueRef result;

  if (CallJsFunction(context,
                     left->asJsValueRef()->asString(),
                     js::GetStringConcatFunction(context),
                     args,
                     _countof(args),
                     &result) != JsNoError) {
    return Local<String>();
  }

  return Local<String>::New(isolate, result);
}

Local<String> String::Concat(Handle<String> left, Handle<String> right) {
  return Concat(Isolate::GetCurrent(), left, right);
}

MaybeLocal<String> String::NewExternalTwoByte(
    Isolate* isolate, ExternalStringResource* resource) {
  if (resource->data() != nullptr) {
    auto newStr = CreateJsStringExternalFromUTF16(
        (const char16_t*)resource->data(), resource->length());
    resource->Dispose();
    return Local<String>::New(isolate, newStr);
  }

  // otherwise the resource is empty just delete it and return an empty string
  resource->Dispose();
  return Empty(nullptr);
}

// Local<String> String::NewExternal(Isolate* isolate,
//                                   ExternalStringResource* resource) {
//   return FromMaybe(NewExternalTwoByte(isolate, resource));
// }

MaybeLocal<String> String::NewExternalOneByte(
    Isolate* isolate, ExternalOneByteStringResource* resource) {
  if (resource->data() != nullptr) {
    auto newStr = CreateJsStringExternalFromLatin1(
        (const unsigned char*)(resource->data()), resource->length());
    resource->Dispose();
    return Local<String>::New(isolate, newStr);
  }

  // otherwise the resource is empty just delete it and return an empty string
  resource->Dispose();
  return Empty(nullptr);
}

Local<String> String::NewExternal(Isolate* isolate,
                                  ExternalOneByteStringResource* resource) {
  return FromMaybe(NewExternalOneByte(isolate, resource));
}

}  // namespace v8
