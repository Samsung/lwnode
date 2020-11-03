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
Local<Object> Proxy::GetTarget() {
  // CHAKRA-TODO: Need to add JSRT API to get target of proxy
  //   JsValueRef undefinedValue = jsrt::GetUndefined();
  NESCARGOT_UNIMPLEMENTED("GetTarget");
  return Local<Object>::New(JsUndefined());
}

Local<Value> Proxy::GetHandler() {
  // CHAKRA-TODO: Need to add JSRT API to get handler of proxy
  //   JsValueRef undefinedValue = jsrt::GetUndefined();
  NESCARGOT_UNIMPLEMENTED("GetHandler");
  return Local<Value>::New(JsUndefined());
}

Proxy* Proxy::Cast(v8::Value* obj) {
  NESCARGOT_ASSERT(obj->IsProxy());
  return static_cast<Proxy*>(obj);
}
}  // namespace v8
