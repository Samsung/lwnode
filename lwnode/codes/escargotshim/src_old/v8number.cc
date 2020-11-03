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
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

double Number::Value() const {
  return NumberValue();
}

Local<Number> Number::New(Isolate* isolate, double value) {
  JsNumberRef number = CreateJsValue(value);
  return Local<Number>::New(isolate, number);
}

Number* Number::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsNumber());
  return static_cast<Number*>(obj);
}

}  // namespace v8
