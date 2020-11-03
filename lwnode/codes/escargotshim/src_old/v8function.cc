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

#include <memory>
#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8.h"

using namespace EscargotShim;

namespace v8 {

MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback,
                                   Local<Value> data,
                                   int length,
                                   ConstructorBehavior behavior) {
  // Local<FunctionTemplate> funcTemplate =
  //     FunctionTemplate::New(IsolateShim::GetCurrentAsIsolate(), callback,
  //     data,
  //                           Local<Signature>(), length);

  // if (behavior == ConstructorBehavior::kThrow) {
  //   funcTemplate->RemovePrototype();
  // }

  // return Local<Function>(funcTemplate->GetFunction());
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Function>();
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context,
                                         int argc,
                                         Handle<Value> argv[]) const {
  NESCARGOT_ASSERT(*context);
  ContextShim* contextShim = context->asContextShim();
  JsContextRef contextRef = contextShim->contextRef();
  JsObjectRef self = asJsValueRef()->asObject();
  JsValueRef* consArgv = CastTo<JsValueRef*>(argv);

  auto result = EvalScript(contextRef,
                           [](JsExecutionStateRef state,
                              JsObjectRef self,
                              int argc,
                              JsValueRef* argv) -> JsValueRef {
                             return self->construct(state, argc, argv);
                           },
                           self,
                           argc,
                           consArgv);
  VERIFY_EVAL_RESULT(result, Local<Object>());
  return Local<Object>::New(contextShim->isolateShim()->asIsolate(),
                            result.result);
}

Local<Object> Function::NewInstance(int argc, Handle<Value> argv[]) const {
  return FromMaybe(NewInstance(
      IsolateShim::GetCurrent()->currentContext()->asContext(), argc, argv));
}

Local<Object> Function::NewInstance() const {
  return NewInstance(0, nullptr);
}

MaybeLocal<Value> Function::Call(Local<Context> context,
                                 Handle<Value> recv,
                                 int argc,
                                 Handle<Value> argv[]) {
  NESCARGOT_ASSERT(!context.IsEmpty());
  ContextShim* contextShim = context->asContextShim();
  JsContextRef contextRef = contextShim->contextRef();
  JsObjectRef self = asJsValueRef()->asObject();
  if (!self->isCallable()) {
    return Local<Value>();
  }

  IsolateShim::GetCurrent()->SetScriptExecuted();
  auto sbresult = EvalScript(contextRef,
                             [](JsExecutionStateRef state,
                                JsObjectRef self,
                                JsValueRef recv,
                                int argc,
                                JsValueRef* argv) -> JsValueRef {
                               return self->call(state, recv, argc, argv);
                             },
                             self,
                             recv->asJsValueRef(),
                             argc,
                             CastTo<JsValueRef*>(argv));

  if (sbresult.error.hasValue()) {
    LoggingJSErrorInfo(sbresult);
    auto scriptException = IsolateShim::GetCurrent()->GetScriptException();
    if (!scriptException->IsThrownException()) {
      scriptException->SetException(sbresult);
      TryCatch tryCatch;
      tryCatch.CheckReportExternalException();
    }
    return Local<Value>();
  }
  return Local<Value>::New(sbresult.result);
}

Local<Value> Function::Call(Handle<Value> recv,
                            int argc,
                            Handle<Value> argv[]) {
  return FromMaybe(
      Call(IsolateShim::GetCurrent()->currentContext()->asContext(),
           recv,
           argc,
           argv));
}

void Function::SetName(Handle<String> name) {
  JsObjectRef self = asJsValueRef()->asObject();
  auto value = name->asJsValueRef();

  bool result = false;
  JsErrorCode error = DefineDataProperty(GetCurrentJsContextRef(),
                                         self,
                                         GetCachedJsValue(CachedStringId::name),
                                         false,
                                         false,
                                         false,
                                         value,
                                         JS_INVALID_REFERENCE,
                                         JS_INVALID_REFERENCE,
                                         JS_INVALID_REFERENCE,
                                         result);

  NESCARGOT_ASSERT(error == JsNoError);
  NESCARGOT_ASSERT(result);
  UNUSED(error);
}

Local<Value> Function::GetName() const {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Value>();
}

// Local<Value> Function::GetInferredName() const {
//   // CHAKRA-TODO: Figure out what to do here
//   CHAKRA_ASSERT(false);
//   return Local<Value>();
// }

// const int Function::kLineOffsetNotFound = -1;

// int Function::GetScriptLineNumber() const {
//   // CHAKRA-TODO: Figure out what to do here
//   CHAKRA_ASSERT(false);
//   return 0;
// }

// int Function::GetScriptColumnNumber() const {
//   // CHAKRA-TODO: Figure out what to do here
//   CHAKRA_ASSERT(false);
//   return 0;
// }

// int Function::ScriptId() const {
//   // CHAKRA-TODO: Figure out what to do here
//   CHAKRA_ASSERT(false);
//   return 0;
// }

Local<Value> Function::GetDebugName() const {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Value>();
}

Local<Value> Function::GetBoundFunction() const {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Value>();
}

Function* Function::Cast(Value* obj) {
  NESCARGOT_ASSERT(obj->IsFunction());
  return static_cast<Function*>(obj);
}

}  // namespace v8
