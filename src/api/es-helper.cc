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

#include "es-helper.h"
#include "context.h"
#include "isolate.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

// ObjectRefHelper

ObjectRefHelper::ObjectRefHelper(ContextRef* context) {
  context_ = context;
  object_ = ObjectRefHelper::create(context);
}

ObjectRefHelper::ObjectRefHelper(ContextRef* context, ObjectRef* object) {
  context_ = context;
  object_ = object;
}

ObjectRefHelper::ObjectRefHelper(ContextWrap* lwContext, ValueRef* value)
    : ObjectRefHelper::ObjectRefHelper(lwContext->get(), value->asObject()) {}

ObjectRef* ObjectRefHelper::create(ContextRef* context) {
  EvalResult r =
      Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
        return ObjectRef::create(state);
      });

  LWNODE_CHECK(r.isSuccessful());

  return r.result->asObject();
}

EvalResult ObjectRefHelper::setPrototype(ValueRef* proto) {
  EvalResult r = Evaluator::execute(
      context_,
      [](ExecutionStateRef* state,
         ObjectRef* object,
         ValueRef* param1) -> ValueRef* {
        object->setPrototype(state, param1);
        return object;
      },
      object_,
      proto);

  return r;
}

ObjectRef* ObjectRefHelper::getPrototype() {
  EvalResult r = Evaluator::execute(
      context_,
      [](ExecutionStateRef* state, ObjectRef* object) -> ValueRef* {
        return object->getPrototype(state);
      },
      object_);

  LWNODE_CHECK(r.isSuccessful());

  return r.result->asObject();
}

}  // namespace EscargotShim
