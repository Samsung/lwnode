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

#pragma once

#include <EscargotPublic.h>
#include "utils/gc.h"

using namespace Escargot;

namespace EscargotShim {

class IsolateWrap;
class ContextWrap;

typedef Evaluator::EvaluatorResult EvalResult;

class ObjectRefHelper : public gc {
 public:
  ObjectRefHelper(ContextRef* context);
  ObjectRefHelper(ContextRef* context, ObjectRef* object);
  ObjectRefHelper(ContextWrap* lwContext, ValueRef* value);

  static ObjectRef* create(ContextRef* context);
  EvalResult setPrototype(ValueRef* proto);
  ObjectRef* getPrototype();

  void* operator new(size_t size) = delete;
  void* operator new[](size_t size) = delete;

 private:
  ObjectRef* object_ = nullptr;
  ContextRef* context_ = nullptr;
};

}  // namespace EscargotShim
