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

using namespace EscargotShim;

namespace v8 {

struct PropertyDescriptor::PrivateData {
  PrivateData()
      : enumerable(false),
        has_enumerable(false),
        configurable(false),
        has_configurable(false),
        writable(false),
        has_writable(false),
        value(JS_INVALID_REFERENCE),
        get(JS_INVALID_REFERENCE),
        set(JS_INVALID_REFERENCE) {}

  bool has_value() { return value != JS_INVALID_REFERENCE; }
  bool has_get() { return get != JS_INVALID_REFERENCE; }
  bool has_set() { return set != JS_INVALID_REFERENCE; }

  bool enumerable : 1;
  bool has_enumerable : 1;
  bool configurable : 1;
  bool has_configurable : 1;
  bool writable : 1;
  bool has_writable : 1;
  JsValueRef value;
  JsValueRef get;
  JsValueRef set;
};

PropertyDescriptor::PropertyDescriptor() : private_(new PrivateData()) {}

PropertyDescriptor::PropertyDescriptor(Local<Value> value)
    : private_(new PrivateData()) {
  private_->value = reinterpret_cast<JsValueRef>(*value);
  if (private_->value != JS_INVALID_REFERENCE) {
    addToPersistentStorage(IsolateShim::GetCurrent()->asIsolate(),
                           private_->value);  // FIXME: check
  }
}

PropertyDescriptor::PropertyDescriptor(Local<Value> value, bool writable)
    : private_(new PrivateData()) {
  private_->value = reinterpret_cast<JsValueRef>(*value);
  if (private_->value != JS_INVALID_REFERENCE) {
    addToPersistentStorage(IsolateShim::GetCurrent()->asIsolate(),
                           private_->value);  // FIXME: check
  }
  private_->has_writable = true;
  private_->writable = writable;
}

PropertyDescriptor::PropertyDescriptor(Local<Value> get, Local<Value> set)
    : private_(new PrivateData()) {
  NESCARGOT_ASSERT(get.IsEmpty() || get->IsUndefined() || get->IsFunction());
  NESCARGOT_ASSERT(set.IsEmpty() || set->IsUndefined() || set->IsFunction());
  private_->get = reinterpret_cast<JsValueRef>(*get);
  private_->set = reinterpret_cast<JsValueRef>(*set);
  if (private_->get != JS_INVALID_REFERENCE) {
    addToPersistentStorage(IsolateShim::GetCurrent()->asIsolate(),
                           private_->get);  // FIXME: check
  }
  if (private_->set != JS_INVALID_REFERENCE) {
    addToPersistentStorage(IsolateShim::GetCurrent()->asIsolate(),
                           private_->set);  // FIXME: check
  }
}

PropertyDescriptor::~PropertyDescriptor() {
  if (private_->value != JS_INVALID_REFERENCE) {
    removeFromPersistentStorage(private_->value);  // FIXME: check
  }
  if (private_->get != JS_INVALID_REFERENCE) {
    removeFromPersistentStorage(private_->get);  // FIXME: check
  }
  if (private_->set != JS_INVALID_REFERENCE) {
    removeFromPersistentStorage(private_->set);  // FIXME: check
  }
  delete private_;
}

Local<Value> PropertyDescriptor::value() const {
  NESCARGOT_ASSERT(private_->has_value());
  return Local<Value>::New(IsolateShim::GetCurrent()->asIsolate(),
                           private_->value);
}

Local<Value> PropertyDescriptor::get() const {
  return Local<Value>::New(IsolateShim::GetCurrent()->asIsolate(),
                           private_->get);
}

Local<Value> PropertyDescriptor::set() const {
  return Local<Value>::New(IsolateShim::GetCurrent()->asIsolate(),
                           private_->set);
}

bool PropertyDescriptor::has_value() const {
  return private_->has_value();
}
bool PropertyDescriptor::has_get() const {
  return private_->has_get();
}
bool PropertyDescriptor::has_set() const {
  return private_->has_set();
}

bool PropertyDescriptor::writable() const {
  NESCARGOT_ASSERT(private_->has_writable);
  return private_->writable;
}

bool PropertyDescriptor::has_writable() const {
  return private_->has_writable;
}

void PropertyDescriptor::set_enumerable(bool enumerable) {
  private_->has_enumerable = true;
  private_->enumerable = enumerable;
}

bool PropertyDescriptor::enumerable() const {
  NESCARGOT_ASSERT(private_->has_enumerable);
  return private_->enumerable;
}

bool PropertyDescriptor::has_enumerable() const {
  return private_->has_enumerable;
}

void PropertyDescriptor::set_configurable(bool configurable) {
  private_->has_configurable = true;
  private_->configurable = configurable;
}

bool PropertyDescriptor::configurable() const {
  NESCARGOT_ASSERT(private_->has_configurable);
  return private_->configurable;
}

bool PropertyDescriptor::has_configurable() const {
  return private_->has_configurable;
}

}  // namespace v8
