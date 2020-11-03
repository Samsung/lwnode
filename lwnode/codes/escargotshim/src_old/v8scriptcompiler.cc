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

#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8.h"

namespace v8 {

using namespace EscargotShim;

MaybeLocal<Function> ScriptCompiler::CompileFunctionInContext(
    Local<Context> context,
    Source* source,
    size_t arguments_count,
    Local<String> arguments[],
    size_t context_extension_count,
    Local<Object> context_extensions[],
    CompileOptions options,
    NoCacheReason no_cache_reason,
    Local<ScriptOrModule>* script_or_module_out) {
  GENERATE_CONTEXT_AND_CHECK_EXIST(context);
  if (script_or_module_out && !script_or_module_out->IsEmpty()) {
    NESCARGOT_UNIMPLEMENTED("");
  }

  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  std::string str_ = "(function() { "
                     "return function() {";

  if (context_extension_count != 0) {
    str_ += "with (this) {";
    for (size_t i = 1; i < context_extension_count; i++) {
      str_ += "with (arguments[" + std::to_string(i - 1);
      str_ += "]) {";
    }
  }

  // body start
  str_ += " return function(";
  for (size_t i = 0; i < arguments_count; i++) {
    if (i != 0) {
      str_ += ",";
    }
    auto s = v8::String::Cast(*arguments[i]);
    int length = s->Utf8Length();
    std::vector<char> buffer;
    buffer.reserve(length + 1);
    s->WriteUtf8(buffer.data());
    str_.append(buffer.data(), length);
  }

  str_ += ") {";

  uint32_t utf8_length = source->source_string->Utf8Length();
  std::vector<char> buffer;
  buffer.reserve(utf8_length + 1);
  source->source_string->WriteUtf8(buffer.data());
  str_.append(buffer.data(), utf8_length);
  str_ += "};";

  // body end
  for (size_t i = 0; i < context_extension_count; i++) {
    str_ += "}";
  }

  str_ += "};})()";

  Local<String> str =
      v8::String::NewFromUtf8(
          v8::Isolate::GetCurrent(), str_.c_str(), v8::NewStringType::kNormal)
          .ToLocalChecked();

  v8::Local<v8::Script> script;
  v8::Script::Compile(context, str).ToLocal(&script);

  // Check whether the JS source has been read successfully
  NESCARGOT_ASSERT(!script.IsEmpty());

  v8::Local<v8::Value> fun;
  script->Run(context).ToLocal(&fun);

  Local<Function> fn = Local<Function>::Cast(fun);
  v8::Local<Value>* args = NULL;
  Local<Value> result;
  fn->Call(context, Undefined(context->GetIsolate()), 0, args).ToLocal(&result);

  return MaybeLocal<Function>(Local<Function>::Cast(result));
}

MaybeLocal<Module> ScriptCompiler::CompileModule(Isolate* isolate,
                                                 Source* source) {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<Module>();
}

Local<PrimitiveArray> ScriptOrModule::GetHostDefinedOptions() {
  NESCARGOT_UNIMPLEMENTED("");
  return Local<PrimitiveArray>();
}

}  // namespace v8
