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

#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8.h"
#include "v8utils.h"

namespace v8 {

using namespace EscargotShim;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  void* Allocate(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_calloc(length, 1);
#else
    void* data = calloc(length, 1);
#endif
    return data;
  }

  void* AllocateUninitialized(size_t length) override {
#if V8_OS_AIX && _LINUX_SOURCE_COMPAT
    // Work around for GCC bug on AIX
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79839
    void* data = __linux_malloc(length);
#else
    void* data = malloc(length);
#endif
    return data;
  }

  void Free(void* data, size_t) override { free(data); }
};

ArrayBuffer::Allocator* ArrayBuffer::Allocator::NewDefaultAllocator() {
  NESCARGOT_UNIMPLEMENTED("");
  return nullptr;
}

Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate, size_t byte_length) {
  NESCARGOT_ASSERT(isolate);

  IsolateShim* curIsolate = IsolateShim::ToIsolateShim(isolate);
  JsContextRef context = curIsolate->currentContext()->contextRef();

  JsArrayBufferRef arrayBuffer = JS_INVALID_REFERENCE;
  if (CreateJsArrayBufferObject(context, byte_length, arrayBuffer) !=
      JsNoError) {
    return Local<ArrayBuffer>();
  }
  return Local<ArrayBuffer>::New(isolate, arrayBuffer);
}

Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate,
                                    void* data,
                                    size_t byte_length,
                                    ArrayBufferCreationMode mode) {
  NESCARGOT_ASSERT(isolate);
  IsolateShim* curIsolate = IsolateShim::ToIsolateShim(isolate);
  JsContextRef context = curIsolate->currentContext()->contextRef();

  JsArrayBufferRef arrayBuffer = JS_INVALID_REFERENCE;
  if (CreateJsArrayBufferObject(
          context, data, byte_length, mode, arrayBuffer) != JsNoError) {
    return Local<ArrayBuffer>();
  }

  return Local<ArrayBuffer>::New(isolate, arrayBuffer);
}

size_t ArrayBuffer::ByteLength() const {
  JsArrayBufferRef self = asJsValueRef()->asObject()->asArrayBufferObject();
  return self->byteLength();
}

ArrayBuffer::Contents ArrayBuffer::GetContents() {
  JsArrayBufferRef self = asJsValueRef()->asObject()->asArrayBufferObject();

  Contents contents;
  contents.data_ = self->rawBuffer();
  contents.byte_length_ = self->byteLength();
  return contents;
}

void ArrayBuffer::Detach() {
  NESCARGOT_UNIMPLEMENTED("");
}

ArrayBuffer* ArrayBuffer::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsArrayBuffer());
  return static_cast<ArrayBuffer*>(obj);
}

bool ArrayBuffer::IsExternal() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

bool ArrayBuffer::IsDetachable() const {
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

ArrayBuffer::Contents ArrayBuffer::Externalize() {
  NESCARGOT_UNIMPLEMENTED("");
  return ArrayBuffer::Contents();
}

}  // namespace v8
