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

Promise::Promise() {}

Local<Value> Promise::Result() {
  return Local<Value>();
}

Promise::PromiseState Promise::State() {
  return PromiseState::kFulfilled;
}

MaybeLocal<Promise> Promise::Then(Local<Context> context,
                                  Local<Function> handler) {
  NESCARGOT_ASSERT(false);
  return Local<Promise>();
}

MaybeLocal<Promise> Promise::Catch(Local<Context> context,
                                   Local<Function> handler) {
  NESCARGOT_ASSERT(false);
  return Local<Promise>();
}

Promise* Promise::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsPromise());
  return static_cast<Promise*>(obj);
}

Local<Promise> Promise::Resolver::GetPromise() {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Promise>();
}

}  // namespace v8
