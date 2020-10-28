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

#include "v8.h"

namespace v8 {
using Resolver = Promise::Resolver;
// CHAKRA-TODO: Unimplemented completely
MaybeLocal<Resolver> Resolver::New(Local<Context> context) {
  NESCARGOT_UNIMPLEMENTED("Resolver::New");
  return Local<Resolver>();
}

Resolver* Resolver::Cast(Value* obj) {
  return static_cast<Resolver*>(obj);
}

Maybe<bool> Resolver::Resolve(Local<Context> context, Local<Value> value) {
  NESCARGOT_UNIMPLEMENTED("Resolve");
  return Just(false);
}

Maybe<bool> Resolver::Reject(Local<Context> context, Local<Value> value) {
  NESCARGOT_UNIMPLEMENTED("Reject");
  return Just(false);
}
}  // namespace v8
