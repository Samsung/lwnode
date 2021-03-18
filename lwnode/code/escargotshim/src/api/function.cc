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

#include "function.h"
#include "isolate.h"

namespace EscargotShim {

static v8::internal::Address* ToAddress(HandleWrap** ref) {
  return reinterpret_cast<v8::internal::Address*>(ref);
}

// FunctionCallbackInfoWrap
FunctionCallbackInfoWrap::FunctionCallbackInfoWrap(
    v8::Isolate* isolate,
    ValueRef* holder,
    ValueRef* thisValue,
    OptionalRef<ObjectRef> newTarget,
    ValueWrap* data,
    int argc,
    ValueRef** argv)
    : v8::FunctionCallbackInfo<v8::Value>(
          ToAddress(m_implicitArgs),
          ToAddress(toWrapperArgs(thisValue, argc, argv)),
          argc) {
  auto lwIsolate = IsolateWrap::fromV8(isolate);

  m_implicitArgs[T::kHolderIndex] = lwIsolate->hole();
  m_implicitArgs[T::kIsolateIndex] = reinterpret_cast<HandleWrap*>(isolate);
  // m_implicitArgs[T::kReturnValueDefaultValueIndex];  // TODO
  m_implicitArgs[T::kReturnValueIndex] = lwIsolate->defaultReturnValue();
  m_implicitArgs[T::kDataIndex] = data;
  m_implicitArgs[T::kNewTargetIndex] =
      newTarget.hasValue() ? ValueWrap::createValue(newTarget.get())
                           : lwIsolate->undefined();
}

HandleWrap** FunctionCallbackInfoWrap::toWrapperArgs(ValueRef* thisValue,
                                                     int argc,
                                                     ValueRef** argv) {
  HandleWrap** values = new HandleWrap*[argc + 1];
#ifdef V8_REVERSE_JSARGS
#error "Not implement V8_REVERSE_JSARGS"
#else
  for (int i = 0; i < argc; i++) {
    values[argc - i - 1] = ValueWrap::createValue(argv[i]);
  }
  values[argc] = ValueWrap::createValue(thisValue);
  return values + argc - 1;
#endif
}

FunctionCallbackInfoWrap::~FunctionCallbackInfoWrap() {
  HandleWrap** values =
      reinterpret_cast<HandleWrap**>(this->values_) - this->Length() + 1;
  delete[] values;
}

// PropertyCallbackInfoWrap
template class PropertyCallbackInfoWrap<v8::Value>;
template class PropertyCallbackInfoWrap<void>;
template class PropertyCallbackInfoWrap<v8::Integer>;
template class PropertyCallbackInfoWrap<v8::Boolean>;
template class PropertyCallbackInfoWrap<v8::Array>;

template <typename T>
PropertyCallbackInfoWrap<T>::PropertyCallbackInfoWrap(v8::Isolate* isolate,
                                                      ValueRef* holder,
                                                      ValueRef* thisValue,
                                                      ValueWrap* data)
    : v8::PropertyCallbackInfo<T>(
          reinterpret_cast<v8::internal::Address*>(m_implicitArgs)) {
  auto lwIsolate = IsolateWrap::fromV8(isolate);
  // m_implicitArgs[F::kShouldThrowOnErrorIndex]; // TODO
  m_implicitArgs[F::kHolderIndex] = lwIsolate->hole();
  m_implicitArgs[F::kIsolateIndex] = reinterpret_cast<HandleWrap*>(isolate);
  // m_implicitArgs[F::kReturnValueDefaultValueIndex]; // TODO
  m_implicitArgs[F::kReturnValueIndex] = lwIsolate->defaultReturnValue();
  m_implicitArgs[F::kDataIndex] = data;
  m_implicitArgs[F::kThisIndex] = ValueWrap::createValue(thisValue);
}

template <typename T>
bool PropertyCallbackInfoWrap<T>::hasReturnValue() {
  auto lwIsolate =
      reinterpret_cast<IsolateWrap*>(m_implicitArgs[F::kIsolateIndex]);

  return m_implicitArgs[F::kReturnValueIndex] !=
         lwIsolate->defaultReturnValue();
}

}  // namespace EscargotShim
