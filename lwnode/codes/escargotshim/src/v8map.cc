/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

size_t Map::Size() const {
  auto isolateShim = EscargotShim::IsolateShim::GetCurrent();
  JsContextRef contextRef = isolateShim->currentContext()->contextRef();

  JsObjectRef object = asJsValueRef()->asObject();

  auto result = EvalScript(
      contextRef,
      [](JsExecutionStateRef state, JsObjectRef object) -> JsValueRef {
        JsValueRef size = CreateJsValue(object->asMapObject()->size(state));
        return size;
      },
      object);
  size_t value = (size_t)result.result->asNumber();
  return value;
}

void Map::Clear() {
  auto isolateShim = EscargotShim::IsolateShim::GetCurrent();
  JsContextRef contextRef = isolateShim->currentContext()->contextRef();

  JsObjectRef object = asJsValueRef()->asObject();

  auto result = EvalScript(
      contextRef,
      [](JsExecutionStateRef state, JsObjectRef object) -> JsValueRef {
        object->asMapObject()->clear(state);
        return CreateJsValue(true);
      },
      object);
}

MaybeLocal<Value> Map::Get(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsObjectRef object = asJsValueRef()->asObject();
  JsValueRef keyRef = key->asJsValueRef();

  auto result = EvalScript(
      contextRef,
      [](JsExecutionStateRef state, JsObjectRef object, JsValueRef keyRef)
          -> JsValueRef { return object->asMapObject()->get(state, keyRef); },
      object,
      keyRef);
  return Local<Value>::New(result.result);
}

MaybeLocal<Map> Map::Set(Local<Context> context,
                         Local<Value> key,
                         Local<Value> value) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsObjectRef object = asJsValueRef()->asObject();
  JsValueRef keyRef = key->asJsValueRef();
  JsValueRef valueRef = value->asJsValueRef();

  auto result =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef keyRef,
                    JsValueRef valueRef) -> JsValueRef {
                   object->asMapObject()->set(state, keyRef, valueRef);
                   return object;
                 },
                 object,
                 keyRef,
                 valueRef);

  return Local<Map>::New(result.result);
}

Maybe<bool> Map::Has(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsObjectRef object = asJsValueRef()->asObject();
  JsValueRef keyRef = key->asJsValueRef();

  auto result = EvalScript(
      contextRef,
      [](JsExecutionStateRef state,
         JsObjectRef object,
         JsValueRef keyRef) -> JsValueRef {
        return CreateJsValue(object->asMapObject()->has(state, keyRef));
      },
      object,
      keyRef);
  bool value = result.result->asBoolean();
  return Just(value);
}

Maybe<bool> Map::Delete(Local<Context> context, Local<Value> key) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  JsObjectRef object = asJsValueRef()->asObject();
  JsValueRef keyRef = key->asJsValueRef();

  auto result =
      EvalScript(contextRef,
                 [](JsExecutionStateRef state,
                    JsObjectRef object,
                    JsValueRef keyRef) -> JsValueRef {
                   return CreateJsValue(
                       object->asMapObject()->deleteOperation(state, keyRef));
                 },
                 object,
                 keyRef);
  bool value = result.result->asBoolean();
  return Just(value);
}

Local<Array> Map::AsArray() const {
  // TODO in EscargotPublic in class Map new method should be added values()
  // from MapObject.h Then new array can be created.
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Array>();
}

Local<Map> Map::New(Isolate* isolate) {
  NESCARGOT_ASSERT(isolate);

  JsContextRef context =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();

  JsMapObjectRef newMapObjectRef;

  auto result =
      EvalScript(context, [](JsExecutionStateRef state) -> JsValueRef {
        Escargot::MapObjectRef* map = Escargot::MapObjectRef::create(state);
        return map;
      });

  newMapObjectRef = result.result->asObject()->asMapObject();

  return Local<Map>::New(newMapObjectRef);
}

Map* Map::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsMap());
  return static_cast<Map*>(obj);
}

}  // namespace v8
