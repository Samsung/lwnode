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

Local<ArrayBuffer> ArrayBufferView::Buffer() {
  auto self = asJsValueRef()->asObject()->asArrayBufferView();
  return Local<ArrayBuffer>(CreateJsValue(self->buffer()));
}

size_t ArrayBufferView::ByteOffset() {
  auto self = asJsValueRef()->asObject()->asArrayBufferView();
  return self->byteOffset();
}

size_t ArrayBufferView::ByteLength() {
  auto self = asJsValueRef()->asObject()->asArrayBufferView();
  return self->byteLength();
}

bool ArrayBufferView::HasBuffer() const {
  auto self = asJsValueRef()->asObject()->asArrayBufferView();
  return self->buffer() != nullptr;
}

size_t ArrayBufferView::CopyContents(void* dest, size_t byte_length) {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

ArrayBufferView* ArrayBufferView::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsArrayBufferView());
  return static_cast<ArrayBufferView*>(obj);
}

size_t TypedArray::Length() {
  auto self = asJsValueRef()->asObject()->asArrayBufferView();
  return self->arrayLength();
}

TypedArray* TypedArray::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsTypedArray());
  return static_cast<TypedArray*>(obj);
}

// JsErrorCode Utils::NewTypedArray(ContextShim::GlobalType constructorIndex,
//                                  Handle<ArrayBuffer> array_buffer,
//                                  size_t byte_offset, size_t length,
//                                  JsValueRef* result) {
//   Isolate* iso = Isolate::GetCurrent();
//   HandleScope handleScope(iso);

//   ContextShim* contextShim = ContextShim::GetCurrent();
//   JsValueRef constructor = contextShim->GetGlobalType(constructorIndex);
//   JsValueRef args[4] = {
//     contextShim->GetUndefined(),
//     *array_buffer,
//     *Integer::From(static_cast<uint32_t>(byte_offset)),
//     *Integer::From(static_cast<uint32_t>(length))
//   };
//   return JsConstructObject(constructor, args, _countof(args), result);
// }

static unsigned GetByteSize(ExternalArrayType type) {
  switch (type) {
    case kExternalInt8Array:
    case kExternalUint8Array:
    case kExternalUint8ClampedArray:
      return 1;
    case kExternalInt16Array:
    case kExternalUint16Array:
      return 2;
    case kExternalInt32Array:
    case kExternalUint32Array:
    case kExternalFloat32Array:
      return 4;
    case kExternalFloat64Array:
      return 8;
    default:
      NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
      break;
  }
}

template <class T1, class T2>
Local<T2> Utils::NewTypedArray(Handle<ArrayBuffer> arrayBuffer,
                               size_t byteOffset,
                               size_t arrayLength,
                               ExternalArrayType type) {
  T1* arrayObject = JS_INVALID_REFERENCE;
  auto result =
      EvalScript(GetCurrentJsContextRef(),
                 [](JsExecutionStateRef state, T1** arrayObject) -> JsValueRef {
                   *arrayObject = T1::create(state);
                   return JsUndefined();
                 },
                 &arrayObject);
  VERIFY_EVAL_RESULT(result, Local<T2>());
  auto arrayBufferRef =
      arrayBuffer->asJsValueRef()->asObject()->asArrayBufferObject();
  arrayObject->setBuffer(
      arrayBufferRef, byteOffset, arrayLength * GetByteSize(type), arrayLength);
  return Local<T2>::New(CreateJsValue(arrayObject));
}

#define DEFINE_TYPEDARRAY_NEW(ArrayType, type)                                 \
  Local<ArrayType##Array> v8::ArrayType##Array::New(                           \
      Handle<ArrayBuffer> arrayBuffer, size_t byteOffset, size_t length) {     \
    return Utils::NewTypedArray<Escargot::ArrayType##ArrayObjectRef,           \
                                ArrayType##Array>(                             \
        arrayBuffer, byteOffset, length, type);                                \
  }

#define DEFINE_TYPEDARRAY_CAST(ArrayType)                                      \
  ArrayType##Array* ArrayType##Array::Cast(Value* obj) {                       \
    NESCARGOT_ASSERT(obj->Is##ArrayType##Array());                             \
    return static_cast<ArrayType##Array*>(obj);                                \
  }

DEFINE_TYPEDARRAY_NEW(Uint8, kExternalUint8Array)
DEFINE_TYPEDARRAY_CAST(Uint8)

DEFINE_TYPEDARRAY_NEW(Uint8Clamped, kExternalUint8ClampedArray)
DEFINE_TYPEDARRAY_CAST(Uint8Clamped)

DEFINE_TYPEDARRAY_NEW(Int8, kExternalInt8Array)
DEFINE_TYPEDARRAY_CAST(Int8)

DEFINE_TYPEDARRAY_NEW(Uint16, kExternalUint16Array)
DEFINE_TYPEDARRAY_CAST(Uint16)

DEFINE_TYPEDARRAY_NEW(Int16, kExternalInt16Array)
DEFINE_TYPEDARRAY_CAST(Int16)

DEFINE_TYPEDARRAY_NEW(Uint32, kExternalUint32Array)
DEFINE_TYPEDARRAY_CAST(Uint32)

DEFINE_TYPEDARRAY_NEW(Int32, kExternalInt32Array)
DEFINE_TYPEDARRAY_CAST(Int32)

DEFINE_TYPEDARRAY_NEW(Float32, kExternalFloat32Array)
DEFINE_TYPEDARRAY_CAST(Float32)

DEFINE_TYPEDARRAY_NEW(Float64, kExternalFloat64Array)
DEFINE_TYPEDARRAY_CAST(Float64)
}  // namespace v8
