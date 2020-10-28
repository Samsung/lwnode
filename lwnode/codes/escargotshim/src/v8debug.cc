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

#include "v8-debug.h"

namespace v8 {

THREAD_LOCAL bool g_EnableInspector = false;
THREAD_LOCAL bool g_EnableReplayDebug = false;

void Debug::EnableInspector(bool enableReplayDebug) {
  g_EnableInspector = true;
  g_EnableReplayDebug = enableReplayDebug;
}

Local<Context> Debug::GetDebugContext(Isolate* isolate) {
  // jsrt::IsolateShim* isoShim = jsrt::IsolateShim::FromIsolate(isolate);
  // JsContextRef debugContextRef = JS_INVALID_REFERENCE;
  // if (isoShim->debugContext == JS_INVALID_REFERENCE) {
  //   HandleScope scope(isolate);
  //   debugContextRef = *Context::New(isolate);
  //   isoShim->debugContext = isoShim->GetContextShim(
  //     debugContextRef);
  //   JsAddRef(debugContextRef, nullptr);

  //   Local<Object> global = Context::GetCurrent()->Global();

  //   // CHAKRA-TODO: Chakra doesn't fully implement the debugger without
  //   // --debug flag. Add a dummy 'Debug' on global object if it doesn't
  //   // already exist.
  //   {
  //     Context::Scope context_scope(debugContextRef);
  //     jsrt::ContextShim* contextShim = jsrt::ContextShim::GetCurrent();
  //     JsValueRef ensureDebug = contextShim->GetensureDebugFunction();
  //     JsValueRef unused;
  //     if (jsrt::CallFunction(ensureDebug, *global, &unused) != JsNoError) {
  //       return Local<Context>();
  //     }
  //   }
  // }

  // return static_cast<Context*>(isoShim->debugContext->GetContextRef());
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Context>();
}

bool Debug::SetDebugEventListener(Isolate* isolate,
                                  EventCallback that,
                                  Local<Value> data) {
  return false;
}

void Debug::SetLiveEditEnabled(Isolate* isolate, bool enable) {
  // CHAKRA-TODO: Figure out what to do here
  //
  // @digitalinfinity: This is edit and continue, right? I don't recall if
  // there are JSRT APIs enabled for this today but if there aren't, it would
  // be an interesting exercise to see what would be needed here (and how
  // Chakra's Edit and Continue differs from v8's). @jianxu would be the
  // expert here in Sandeep's absence
  NESCARGOT_ASSERT(false);
}

}  // namespace v8
