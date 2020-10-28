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

namespace v8 {

using namespace EscargotShim;

Handle<Primitive> Undefined(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);
  JsValueRef valueRef =
      IsolateShim::ToIsolateShim(isolate)->Cached()->Undefined();
  return Handle<Primitive>::New(isolate, valueRef);
}

Handle<Primitive> Null(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);
  return Handle<Primitive>::New(
      isolate, IsolateShim::ToIsolateShim(isolate)->Cached()->Null());
}

Handle<Boolean> True(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);
  return Handle<Boolean>::New(
      isolate, IsolateShim::ToIsolateShim(isolate)->Cached()->True());
}

Handle<Boolean> False(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);
  return Handle<Boolean>::New(
      isolate, IsolateShim::ToIsolateShim(isolate)->Cached()->False());
}

bool SetResourceConstraints(ResourceConstraints* constraints) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(
      "Ignore for now, we don't support setting the stack limit.");
  return true;
}

}  // namespace v8
