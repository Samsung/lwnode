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

using namespace EscargotShim;

namespace v8 {

void Utils::WeakReferenceCallbackWrapperCallback(void* self, void* data) {
  const auto callbackWrapper =
      reinterpret_cast<const v8EscargotShim::WeakReferenceCallbackWrapper*>(
          data);

  if (callbackWrapper->isWeakCallbackInfo) {
    WeakCallbackInfo<void>::Callback callback;
    void* fields[kInternalFieldsInWeakCallback] = {};
    WeakCallbackInfo<void> info(
        Isolate::GetCurrent(), callbackWrapper->parameters, fields, &callback);
    callbackWrapper->infoCallback(info);

  } else {
    WeakCallbackData<Value, void> data(Isolate::GetCurrent(),
                                       callbackWrapper->parameters,
                                       static_cast<Value*>(self));
    callbackWrapper->dataCallback(data);
  }
}

namespace v8EscargotShim {

void ClearObjectWeakReferenceCallback(JsValueRef object, bool revive) {
  if (revive) {
    GC_REGISTER_FINALIZER_NO_ORDER(object,
                                   [](void* self, void* data) {
                                     // Do nothing, only used to revive an
                                     // object temporarily
                                   },
                                   nullptr,
                                   nullptr,
                                   nullptr);
  } else {
    GC_REGISTER_FINALIZER_NO_ORDER(object, nullptr, nullptr, nullptr, nullptr);
  }
}

template <class Callback, class Func>
void SetObjectWeakReferenceCallbackCommon(
    JsValueRef object,
    Callback callback,
    WeakReferenceCallbackWrapper** weakWrapper,
    const Func& initWrapper) {
  if (callback == nullptr || object == JS_INVALID_REFERENCE) {
    return;
  }

  // This will be called once per instance
  // We do not share the _weakWrapper.
  // The memory is allocated per request and released similarly.
  // previous shared_ptr interface was actually doing the same.
  // however in case the instance was shared(which is not in motion)
  // it was keeping the callback until the shared counter reaches to unique
  // state.
  assert(!*weakWrapper && "This should be nullptr.");
  *weakWrapper = new WeakReferenceCallbackWrapper();

  WeakReferenceCallbackWrapper* callbackWrapper = (*weakWrapper);
  initWrapper(callbackWrapper);

  GC_REGISTER_FINALIZER_NO_ORDER(
      (void*)object,
      v8::Utils::WeakReferenceCallbackWrapperCallback,
      callbackWrapper,
      nullptr,
      nullptr);
}

// Set WeakCallbackInfo []
void SetObjectWeakReferenceCallback(
    JsValueRef object,
    WeakCallbackInfo<void>::Callback callback,
    void* parameters,
    WeakReferenceCallbackWrapper** weakWrapper) {
  SetObjectWeakReferenceCallbackCommon(
      object,
      callback,
      weakWrapper,
      [=](WeakReferenceCallbackWrapper* callbackWrapper) {
        callbackWrapper->parameters = parameters;
        callbackWrapper->infoCallback = callback;
        callbackWrapper->isWeakCallbackInfo = true;
      });
}

// Set WeakCallbackData
void SetObjectWeakReferenceCallback(
    JsValueRef object,
    WeakCallbackData<Value, void>::Callback callback,
    void* parameters,
    WeakReferenceCallbackWrapper** weakWrapper) {
  SetObjectWeakReferenceCallbackCommon(
      object,
      callback,
      weakWrapper,
      [=](WeakReferenceCallbackWrapper* callbackWrapper) {
        callbackWrapper->parameters = parameters;
        callbackWrapper->dataCallback = callback;
        callbackWrapper->isWeakCallbackInfo = false;
      });
}

}  // namespace v8EscargotShim

}  // namespace v8
