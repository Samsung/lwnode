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

#include "v8utils.h"
#include "escargotisolateshim.h"
#include "escargotutil.h"
#include "v8.h"

using namespace EscargotShim;

namespace EscargotShim {
JsObjectTemplateRef TemplateData::castToJsObjectTemplateRef(
    v8::ObjectTemplate* v8ObjectTemplate) {
  // NOTE: v8::ObjectTemplate does not inherit v8::Value
  JsObjectRef objectTemplateWrapper =
      reinterpret_cast<JsValueRef>(v8ObjectTemplate)->asObject();
  NESCARGOT_ASSERT(objectTemplateWrapper->isObject());

  JsObjectTemplateRef objectTemplate =
      reinterpret_cast<JsObjectTemplateRef>(objectTemplateWrapper->extraData());
  NESCARGOT_ASSERT(objectTemplate && objectTemplate->isObjectTemplate());

  return objectTemplate;
}

JsFunctionTemplateRef TemplateData::castToJsFunctionTemplateRef(
    v8::FunctionTemplate* v8FunctionTemplate) {
  // NOTE: v8::FunctionTemplate does not inherit v8::Value
  JsObjectRef functionTemplateWrapper =
      reinterpret_cast<JsValueRef>(v8FunctionTemplate)->asObject();
  NESCARGOT_ASSERT(functionTemplateWrapper->isObject());

  JsFunctionTemplateRef functionTemplate =
      reinterpret_cast<JsFunctionTemplateRef>(
          functionTemplateWrapper->extraData());
  NESCARGOT_ASSERT(functionTemplate && functionTemplate->isFunctionTemplate());

  return functionTemplate;
}
}  // namespace EscargotShim

namespace v8 {

ExternalData::ExternalData(ExternalDataTypes type) : type(type) {
  GC_REGISTER_FINALIZER_NO_ORDER(this,
                                 [](void* obj, void* cd) {
                                   NESCARGOT_ASSERT(obj != nullptr);
                                   auto self = static_cast<ExternalData*>(obj);
                                   self->~ExternalData();
                                 },
                                 NULL,
                                 NULL,
                                 NULL);
}

std::string Utils::GetStdString(Handle<String> string) {
  if (string.IsEmpty()) {
    return std::string("");
  }
  return string->asJsValueRef()->asString()->toStdUTF8String();
}

}  // namespace v8
