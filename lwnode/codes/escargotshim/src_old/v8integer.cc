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

using namespace EscargotShim;

namespace v8 {

Local<Integer> Integer::New(Isolate* isolate, int32_t value) {
  JsValueRef val = CreateJsValue(value);
  return Local<Integer>::New(isolate, val);
}

Local<Integer> Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
  JsValueRef val = CreateJsValue(value);
  return Local<Integer>::New(isolate, val);
}

Integer* Integer::Cast(v8::Value* obj) {
  return static_cast<Integer*>(obj);
}

int64_t Integer::Value() const {
  return IntegerValue();
}

}  // namespace v8
