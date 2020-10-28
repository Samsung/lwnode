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

Local<Value> Private::Name() const {
  return reinterpret_cast<const Symbol*>(this)->Name();
}

Local<Private> Private::New(Isolate* isolate, Local<String> name) {
  NESCARGOT_ASSERT(isolate);

  Local<Symbol> symbol = Symbol::New(isolate, name);
  return Local<Private>::New(isolate, *symbol);
}

Local<Private> Private::ForApi(Isolate* isolate, Local<String> name) {
  NESCARGOT_ASSERT(isolate);

  auto symbolForKey = CreateJsValueSymbolFor(
      IsolateShim::ToIsolateShim(isolate)->vmInstanceRef(),
      name->asJsValueRef()->asString());
  return Local<Private>::New(isolate, symbolForKey);
}

}  // namespace v8
