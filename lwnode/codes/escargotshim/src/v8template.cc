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
#include "v8utils.h"

namespace v8 {

using namespace EscargotShim;

Template::Template() {}

static JsTemplateRef castToJsTemplateRef(v8::Template* v8Template) {
  // NOTE: v8::Template is always a wrapper in JsValueRef format.
  JsObjectRef templateWrapper = CastTo<JsValueRef>(v8Template)->asObject();
  JsTemplateRef templateRef =
      reinterpret_cast<JsTemplateRef>(templateWrapper->extraData());
  NESCARGOT_ASSERT(templateRef);
  NESCARGOT_ASSERT(templateRef->isFunctionTemplate() ||
                   templateRef->isObjectTemplate());

  return templateRef;
}

void Template::Set(Local<Name> name,
                   Local<Data> value,
                   PropertyAttribute attribute) {
  bool isWritable = !(attribute & ReadOnly);
  bool isEnumerable = !(attribute & DontEnum);
  bool isConfigurable = !(attribute & DontDelete);

  JsTemplateRef self = castToJsTemplateRef(this);

  // Name can be either a string or symbol
  JsValueRef nameRef = name->asJsValueRef();
  JsTemplatePropertyNameRef propertyName;
  if (name->asJsValueRef()->isString()) {
    propertyName = JsTemplatePropertyNameRef(nameRef->asString());
  } else if (name->asJsValueRef()->isSymbol()) {
    propertyName = JsTemplatePropertyNameRef(nameRef->asSymbol());
  }

  // NOTE: value is always of type JsValueRef. When a template comes
  // it is wrapped in a JsValueRef.
  JsValueRef valueRef = CastTo<JsValueRef>(*value);
  JsTemplateRef templateValue = nullptr;
  if (valueRef->isObject() && valueRef->asObject()->extraData()) {
    templateValue = (JsTemplateRef)valueRef->asObject()->extraData();
    NESCARGOT_ASSERT(templateValue->isFunctionTemplate() ||
                     templateValue->isObjectTemplate());
  }

  if (templateValue) {
    self->set(
        propertyName, templateValue, isWritable, isEnumerable, isConfigurable);
  } else {
    self->set(propertyName, valueRef, isWritable, isEnumerable, isConfigurable);
  }
}

void Template::SetAccessorProperty(Local<Name> name,
                                   Local<FunctionTemplate> getter,
                                   Local<FunctionTemplate> setter,
                                   PropertyAttribute attribute,
                                   AccessControl settings) {
  bool isEnumerable = !(attribute & DontEnum);
  bool isConfigurable = !(attribute & DontDelete);

  JsTemplateRef self = castToJsTemplateRef(this);
  JsStringRef nameRef = name->asJsValueRef()->asString();
  JsFunctionTemplateRef getterRef = nullptr;
  if (!getter.IsEmpty()) {
    getterRef = TemplateData::castToJsFunctionTemplateRef(*getter);
  }
  JsFunctionTemplateRef setterRef = nullptr;
  if (!setter.IsEmpty()) {
    setterRef = TemplateData::castToJsFunctionTemplateRef(*setter);
  }

  self->setAccessorProperty(
      nameRef,
      Escargot::OptionalRef<Escargot::FunctionTemplateRef>(getterRef),
      Escargot::OptionalRef<Escargot::FunctionTemplateRef>(setterRef),
      isEnumerable,
      isConfigurable);
}

}  // namespace v8
