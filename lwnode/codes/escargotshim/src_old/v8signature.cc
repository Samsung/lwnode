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

using namespace EscargotShim;

namespace v8 {

Local<Signature> Signature::New(Isolate* isolate,
                                Handle<FunctionTemplate> receiver,
                                int argc,
                                Handle<FunctionTemplate> argv[]) {
  if (argc != 0) {
    NESCARGOT_UNIMPLEMENTED("v8::Signature::New with args list");
  }

  return reinterpret_cast<Signature*>(*receiver);
}

Local<AccessorSignature> AccessorSignature::New(
    Isolate* isolate, Handle<FunctionTemplate> receiver) {
  return reinterpret_cast<AccessorSignature*>(*receiver);
}

bool Utils::CheckSignature(Local<FunctionTemplate> receiver,
                           Local<Object> thisPointer,
                           Local<Object>* holder) {
  // TODO
  *holder = thisPointer;
  return true;
}

}  // namespace v8
