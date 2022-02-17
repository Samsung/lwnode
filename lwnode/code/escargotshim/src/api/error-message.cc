/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "error-message.h"
#include "utils/misc.h"

using namespace Escargot;

namespace EscargotShim {

ErrorObjectRef::Code ErrorMessage::getErrorCode(ErrorMessageType type) {
  switch (type) {
#define CASE(NAME, CODE, STRING)                                               \
  case ErrorMessageType::k##NAME:                                              \
    return Escargot::ErrorObjectRef::Code::CODE;
    ERROR_MESSAGE_TEMPLATES(CASE)
#undef CASE
    case ErrorMessageType::kErrorMessageCount:
    default:
      return ErrorObjectRef::Code::None;
  }
}

const char* ErrorMessage::getErrorString(ErrorMessageType type) {
  switch (type) {
#define CASE(NAME, CODE, STRING)                                               \
  case ErrorMessageType::k##NAME:                                              \
    return STRING;
    ERROR_MESSAGE_TEMPLATES(CASE)
#undef CASE
    case ErrorMessageType::kErrorMessageCount:
    default:
      return nullptr;
  }
}

Escargot::StringRef* ErrorMessage::createErrorStringRef(ErrorMessageType type) {
  auto errorString = ErrorMessage::getErrorString(type);
  LWNODE_DCHECK_NOT_NULL(errorString);
  return StringRef::createFromASCII(errorString, strlen(errorString));
}

}  // namespace EscargotShim
