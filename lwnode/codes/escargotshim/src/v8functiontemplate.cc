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
#include "v8utils.h"

using namespace EscargotShim;

namespace v8 {

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
                                              FunctionCallback callback,
                                              Local<Value> data,
                                              Local<Signature> signature,
                                              int length,
                                              ConstructorBehavior behavior,
                                              SideEffectType side_effect_type) {
  NESCARGOT_ASSERT(isolate);

  JsAtomicStringRef name = CreateJsEmptyAtomicString();
  bool isStrict = false;
  bool isConstructor = false;
  if (behavior == ConstructorBehavior::kAllow) {
    isConstructor = true;
  }

  JsContextRef contextRef =
      IsolateShim::ToIsolateShim(isolate)->currentContext()->contextRef();
  JsObjectRef functionTemplateWrapper = nullptr;
  if (CreateJsObject(contextRef, functionTemplateWrapper) != JsNoError) {
    return Local<FunctionTemplate>();
  }
  auto fn =
      [](JsExecutionStateRef state,
         JsValueRef thisValue,
         size_t argc,
         JsValueRef* argv,
         Escargot::OptionalRef<Escargot::ObjectRef> newTarget) -> JsValueRef {
    Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
        state->resolveCallee();
    NESCARGOT_ASSERT(callee);

    TemplateData* tpData = reinterpret_cast<TemplateData*>(callee->extraData());
    if (thisValue->asObject()->extraData()) {
      TemplateData* thisValueTpData =
          reinterpret_cast<TemplateData*>(thisValue->asObject()->extraData());

      if (thisValueTpData) {
        TemplateData* newThisValueTpData = new TemplateData(*thisValueTpData);
        thisValue->asObject()->setExtraData(newThisValueTpData);
      }
    }

    NESCARGOT_ASSERT(tpData);

    Handle<Value> result;
    if (tpData->m_callback) {
      // FIXME: signature is not supported yet
      Local<Object> holder = thisValue;
      FunctionCallbackInfo<Value> info(
          reinterpret_cast<Value**>(argv),
          argc,
          Local<Object>::New(tpData->m_isolate, thisValue),
          Local<Object>::New(tpData->m_isolate, newTarget.value()),
          holder,
          newTarget.hasValue(),
          tpData->m_callbackData,
          Local<Function>::New(tpData->m_isolate, callee.get()));

      tpData->m_callback(info);
      result = info.GetReturnValue().Get();
      IsolateShim::GetCurrent()
          ->GetScriptException()
          ->ThrowExceptionToStateIfHasError(state);
    }

    if (newTarget.hasValue()) {
      return thisValue;
    }

    if (!result.IsEmpty()) {
      return result->asJsValueRef();
    }

    return JsUndefined();
  };

  JsFunctionTemplateRef self =
      CreateJsFunctionTemplate(name, length, isStrict, isConstructor, fn);

  TemplateData* tpData =
      new TemplateData(isolate, callback, data, signature, length);
  self->setInstanceExtraData(tpData);
  functionTemplateWrapper->setExtraData(self);

  return Local<FunctionTemplate>::New(isolate, functionTemplateWrapper);
}

MaybeLocal<Function> FunctionTemplate::GetFunction(Local<Context> context) {
  JsContextRef contextRef = context->asContextShim()->contextRef();
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  JsObjectRef functionObjectRef = self->instantiate(contextRef);
  NESCARGOT_ASSERT(functionObjectRef->isFunctionObject());

  return Local<Function>::New(IsolateShim::GetCurrent()->asIsolate(),
                              functionObjectRef);
}

Local<Function> FunctionTemplate::GetFunction() {
  ContextShim* contextShim = IsolateShim::GetCurrent()->currentContext();
  return FromMaybe(GetFunction(Local<Context>::New(
      IsolateShim::GetCurrent()->asIsolate(), contextShim)));
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  JsObjectTemplateRef instanceTemplateRef = self->instanceTemplate();
  if (!instanceTemplateRef) {
    return Local<ObjectTemplate>();
  }

  JsContextRef contextRef =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  JsObjectRef objectTemplateWrapper = nullptr;
  if (CreateJsObject(contextRef, objectTemplateWrapper) != JsNoError) {
    return Local<ObjectTemplate>();
  }

  NESCARGOT_ASSERT(self->instanceExtraData());
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  instanceTemplateRef->setInstanceExtraData(tpData);
  objectTemplateWrapper->setExtraData(instanceTemplateRef);

  return Local<ObjectTemplate>::New(tpData->m_isolate, objectTemplateWrapper);
}

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  JsObjectTemplateRef prototypeTemplateRef = self->prototypeTemplate();
  if (!prototypeTemplateRef) {
    return Local<ObjectTemplate>();
  }

  JsContextRef contextRef =
      IsolateShim::GetCurrent()->currentContext()->contextRef();
  JsObjectRef objectTemplateWrapper = nullptr;
  if (CreateJsObject(contextRef, objectTemplateWrapper) != JsNoError) {
    return Local<ObjectTemplate>();
  }

  NESCARGOT_ASSERT(self->instanceExtraData());
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  prototypeTemplateRef->setInstanceExtraData(tpData);
  objectTemplateWrapper->setExtraData(prototypeTemplateRef);

  return Local<ObjectTemplate>::New(tpData->m_isolate, objectTemplateWrapper);
}

void FunctionTemplate::SetClassName(Handle<String> name) {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  tpData->m_className = name;
}

void FunctionTemplate::SetHiddenPrototype(bool value) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("SetHiddenPrototype");
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      Handle<Value> data) {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  TemplateData* tpData =
      reinterpret_cast<TemplateData*>(self->instanceExtraData());
  tpData->m_callback = callback;
  tpData->m_callbackData = data;
}

bool FunctionTemplate::HasInstance(Handle<Value> object) {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  return self->didInstantiate();
}

void FunctionTemplate::Inherit(Handle<FunctionTemplate> parent) {
  JsFunctionTemplateRef self = TemplateData::castToJsFunctionTemplateRef(this);
  JsFunctionTemplateRef parentFunctionTemplate =
      TemplateData::castToJsFunctionTemplateRef(*parent);
  self->inherit(parentFunctionTemplate);
}

void FunctionTemplate::RemovePrototype() {
  NESCARGOT_UNIMPLEMENTED("");
}

}  // namespace v8
