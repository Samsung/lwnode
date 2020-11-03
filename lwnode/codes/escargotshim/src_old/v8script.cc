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
// #include <memory>

using namespace EscargotShim;

namespace v8 {

// using CachedPropertyIdRef = jsrt::CachedPropertyIdRef;

// THREAD_LOCAL JsSourceContext currentContext;
// extern bool g_useStrict;

// Local<Script> Script::Compile(Handle<String> source, ScriptOrigin* origin) {
//   return FromMaybe(Compile(Local<Context>(), source, origin));
// }

// CHAKRA-TODO: Convert this function to javascript
// Create a object to hold the script infomration
// static JsErrorCode CreateScriptObject(JsValueRef sourceRef,
//                                       JsValueRef filenameRef,
//                                       JsValueRef scriptFunction,
//                                       JsValueRef * scriptObject) {
//   JsErrorCode error = JsCreateObject(scriptObject);
//   if (error != JsNoError) {
//     return error;
//   }

//   error = jsrt::SetProperty(*scriptObject, CachedPropertyIdRef::source,
//                             sourceRef);
//   if (error != JsNoError) {
//     return error;
//   }

//   error = jsrt::SetProperty(*scriptObject, CachedPropertyIdRef::filename,
//                             filenameRef);
//   if (error != JsNoError) {
//     return error;
//   }

//   return jsrt::SetProperty(*scriptObject, CachedPropertyIdRef::function,
//                            scriptFunction);
// }

// Compiled script object, bound to the context that was active when this
// function was called. When run it will always use this context.
MaybeLocal<Script> Script::Compile(Local<Context> context,
                                   Handle<String> source,
                                   ScriptOrigin* origin) {
  NESCARGOT_ASSERT(!context.IsEmpty());
  ContextShim* contextShim = EscargotShim::ContextShim::ToContextShim(*context);
  JsContextRef contextRef = contextShim->contextRef();

  JsStringRef filenameRef = JS_INVALID_REFERENCE;

  if (origin != nullptr) {
    Local<Value> value = *origin->ResourceName();
    filenameRef = value->asJsValueRef()->asString();
  }
  if (filenameRef == nullptr) {
    filenameRef = CreateJsStringFromASCII("unknown");
  }

  JsStringRef sourceRef = source->asJsValueRef()->asString();

  if (!sourceRef->isString()) {
    NESCARGOT_LOG_ERROR("Script Parser Error\n");
    return Local<Script>();
  }

  auto s = sourceRef->asString()->toStdUTF8String();
  auto part = s.substr(0, s.length() > 500 ? 500 : s.length());
  NESCARGOT_LOG(4, INFO, "$ source:\n%s .....\n\n", part.c_str());

  JsScriptRef parsedScript = JS_INVALID_REFERENCE;

  NESCARGOT_LOG(2,
                INFO,
                "$ Compile: %s\n",
                filenameRef->asString()->toStdUTF8String().c_str());

  if (ParseScript(contextRef, sourceRef, filenameRef, parsedScript) !=
      JsNoError) {
    return Local<Script>();
  }

  JsObjectRef scriptObject = JS_INVALID_REFERENCE;
  if (CreateJsScriptObject(
          contextRef, sourceRef, filenameRef, parsedScript, scriptObject) !=
      JsNoError) {
    return Local<Script>();
  }

  return Local<Script>::New(contextShim->isolateShim()->asIsolate(),
                            scriptObject);
}

// Local<Script> Script::Compile(Handle<String> source,
//                               Handle<String> file_name) {
//   ScriptOrigin origin(file_name);
//   return FromMaybe(Compile(Local<Context>(), source, &origin));
// }

MaybeLocal<Value> Script::Run(Local<Context> context) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);

  EscargotShim::IsolateShim::GetCurrent()->SetScriptExecuted();

  JsObjectRef objectRef = CastTo<JsObjectRef>(this)->asObject();
  ScriptData* scriptData = static_cast<ScriptData*>(GetExtraData(objectRef));
  JsScriptRef scriptRef = scriptData->script;
  NESCARGOT_ASSERT(scriptRef != nullptr);

#if defined(DEBUG)
  JsValueRef filename = JS_INVALID_REFERENCE;
  if (GetProperty(contextRef, objectRef, CachedStringId::filename, filename) !=
      JsNoError) {
    NESCARGOT_ASSERT(false);
  }
  NESCARGOT_LOG(
      2, INFO, "$ Run: %s\n", filename->asString()->toStdUTF8String().c_str());
#endif

  auto sbresult = EvalScript(
      contextRef,
      [](JsExecutionStateRef state, JsScriptRef scriptRef) -> JsValueRef {
        return scriptRef->execute(state);
      },
      scriptRef);

  // VERIFY_EVAL_RESULT(sbresult, Local<Value>());
  if (!sbresult.isSuccessful()) {
    NESCARGOT_LOG_ERROR(
        "Script Run Error: %s\n",
        sbresult.resultOrErrorToString(contextRef)->toStdUTF8String().c_str());
    EscargotShim::IsolateShim::GetCurrent()->GetScriptException()->SetException(
        sbresult);
    return Local<Value>();
  }

  return Local<Value>::New(sbresult.result);
}

Local<Value> Script::Run() {
  return FromMaybe(Run(Local<Context>()));
}

Local<UnboundScript> Script::GetUnboundScript() {
  // FIXME: Do we support unbounded script?
  auto contextRef = GetCurrentJsContextRef();
  JsObjectRef unboundScriptObject = JS_INVALID_REFERENCE;
  if (CreateJsObject(contextRef, unboundScriptObject) != JsNoError) {
    return Local<UnboundScript>();
  }

  bool result = false;
  if (SetProperty(contextRef,
                  unboundScriptObject,
                  CachedStringId::script,
                  CastTo<JsValueRef>(this),
                  result) != JsNoError ||
      result == false) {
    return Local<UnboundScript>();
  }

  SetExtraData(unboundScriptObject, new ScriptData(contextRef));
  return Local<UnboundScript>::New(unboundScriptObject);
}

Local<Script> UnboundScript::BindToCurrentContext() {
  auto object = CastTo<JsValueRef>(this)->asObject();
  auto scriptData = static_cast<ScriptData*>(GetExtraData(object));
  auto contextRef = GetCurrentJsContextRef();

  if (scriptData->context == contextRef) {
    JsValueRef scriptRef = JS_INVALID_REFERENCE;
    if (GetProperty(contextRef, object, CachedStringId::script, scriptRef) !=
        JsNoError) {
      return Local<Script>();
    }
    // Same context, we can reuse the same script object
    return Local<Script>::New(scriptRef);
  }

  // TODO: Create a script object in another context
  JsValueRef scriptRef = JS_INVALID_REFERENCE;
  if (GetProperty(
          scriptData->context, object, CachedStringId::script, scriptRef) !=
      JsNoError) {
    return Local<Script>();
  }

  JsValueRef originalSourceRef = JS_INVALID_REFERENCE;
  if (GetProperty(scriptData->context,
                  scriptRef->asObject(),
                  CachedStringId::source,
                  originalSourceRef) != JsNoError) {
    return Local<Script>();
  }
  JsValueRef originalFilenameRef = JS_INVALID_REFERENCE;
  if (GetProperty(scriptData->context,
                  scriptRef->asObject(),
                  CachedStringId::filename,
                  originalFilenameRef) != JsNoError) {
    return Local<Script>();
  }

  JsScriptRef scriptFunction = JS_INVALID_REFERENCE;
  if (ParseScript(contextRef,
                  originalSourceRef->asString(),
                  originalFilenameRef->asString(),
                  scriptFunction) != JsNoError) {
    return Local<Script>();
  }

  auto originalSourceStdString =
      originalSourceRef->asString()->toStdUTF8String();
  JsStringRef source = CreateJsStringFromUTF8(originalSourceStdString.data(),
                                              originalSourceStdString.length());

  auto originalFilenameStdString =
      originalFilenameRef->asString()->toStdUTF8String();
  JsStringRef filename = CreateJsStringFromUTF8(
      originalFilenameStdString.data(), originalFilenameStdString.length());

  JsObjectRef scriptObject = JS_INVALID_REFERENCE;

  if (CreateJsScriptObject(scriptData->context,
                           source,
                           filename,
                           scriptFunction,
                           scriptObject) != JsNoError) {
    return Local<Script>();
  }

  return Local<Script>(scriptObject);
}

int UnboundScript::GetId() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* isolate, Source* source, CompileOptions options) {
  Local<Context> context = isolate->GetCurrentContext();
  MaybeLocal<Script> maybe = Compile(context, source, options);
  if (maybe.IsEmpty()) {
    return Local<UnboundScript>();
  }
  return FromMaybe(maybe)->GetUnboundScript();
}

Local<UnboundScript> ScriptCompiler::CompileUnbound(Isolate* isolate,
                                                    Source* source,
                                                    CompileOptions options) {
  return FromMaybe(CompileUnboundScript(isolate, source, options));
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
                                           Source* source,
                                           CompileOptions options) {
  ScriptOrigin origin(source->resource_name);
  return Script::Compile(context, source->source_string, &origin);
}

Local<Script> ScriptCompiler::Compile(Isolate* isolate,
                                      Source* source,
                                      CompileOptions options) {
  return FromMaybe(Compile(Local<Context>(), source, options));
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

}  // namespace v8
