/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

#include <unistd.h>
#if defined(LWNODE_PLATFORM_LINUX)
#include <execinfo.h>
#endif

#include "api/context.h"
#include "api/es-helper.h"
#include "api/isolate.h"
#include "debug.h"
#include "misc.h"

using namespace Escargot;
using namespace v8;

namespace EscargotShim {

#if defined(LWNODE_PLATFORM_LINUX)
static const int kStackTraceBufferSize = 512;
static const int kStackTraceFrameSize = 128;

static const char* getCurrentProcessName() {
  static char processName[kStackTraceBufferSize];
  char pathname[kStackTraceBufferSize];
  snprintf(pathname, kStackTraceBufferSize, "/proc/%d/exe", getpid());
  auto lengthProcessName =
      readlink(pathname, processName, kStackTraceBufferSize - 1);
  if (lengthProcessName == -1) {
    return nullptr;
  } else {
    processName[lengthProcessName] = 0;
    return processName;
  }
}
#endif

void DebugUtils::printStackTrace() {
#if defined(NDEBUG)
  if (EscargotShim::Flags::isInternalLogEnabled() == false) {
    return;
  }
#endif
#if defined(LWNODE_PLATFORM_LINUX)
  void* frames[kStackTraceFrameSize];
  static const char* processName = getCurrentProcessName();
  if (!processName) {
    LWNODE_DLOG_WARN("Cannot print stack trace!");
    return;
  }

  auto frameSize = backtrace(frames, kStackTraceFrameSize);
  if (frameSize == 0) {
    LWNODE_DLOG_WARN("Cannot get stack trace!");
    return;
  }
  auto symbols = backtrace_symbols(frames, frameSize);
  if (!symbols) {
    LWNODE_DLOG_WARN("Cannot get stack symbols!");
    return;
  }

  LWNODE_LOGR("\n[Backtrace]");
  for (int i = 1; i < frameSize - 2; ++i) {
    char cmd[kStackTraceBufferSize];
    snprintf(cmd,
             kStackTraceBufferSize,
             "addr2line %p -e %s -f -C 2>&1",
             frames[i],
             processName);
    auto stream = popen(cmd, "r");
    if (stream) {
      char line[kStackTraceBufferSize];
      bool isFirstLine = true;
      while (fgets(line, sizeof(line), stream)) {
        line[strnlen(line, kStackTraceBufferSize) - 1] = 0;
        if (isFirstLine) {
          LWNODE_LOGR("#%-2d: %s", i - 1, line);
          isFirstLine = false;
        } else {
          LWNODE_LOGR("%s", line);
        }
      }
      LWNODE_LOGR("");
      pclose(stream);
    } else if (symbols[i]) {
      LWNODE_LOGR("#%-2d: %s", i - 1, symbols[i]);
    }
  }
#endif
}

static void printObjectProperties(ExecutionStateRef* state,
                                  ObjectRef* object,
                                  int depth = 0) {
  auto propertyNames = object->ownPropertyKeys(state);
  LWNODE_LOGR("%*s[prop(%zu)]\n", depth, "", propertyNames->size());
  for (size_t i = 0; i < propertyNames->size(); i++) {
    auto propertyValue = object->getOwnProperty(state, propertyNames->at(i));
    auto propertyNameString = propertyNames->at(i)->toString(state);
    auto propertyValueString = propertyValue->toString(state);
    LWNODE_LOGR("  %*s%s: %s\n",
                depth,
                "",
                propertyNameString->toStdUTF8String().data(),
                propertyValueString->toStdUTF8String().data());
  }
}

void DebugUtils::printObject(ObjectRef* value, int depth) {
  auto lwIsolate = IsolateWrap::GetCurrent();
  auto r = Evaluator::execute(
      lwIsolate->GetCurrentContext()->get(),
      [](ExecutionStateRef* state, ObjectRef* object, int depth) -> ValueRef* {
        printObjectProperties(state, object, depth);

        OptionalRef<ObjectRef> maybePrototype =
            object->getPrototypeObject(state);
        if (maybePrototype.hasValue()) {
          auto prototype = maybePrototype.value();
          printObjectProperties(state, prototype, depth);

          OptionalRef<ObjectRef> maybePrototype =
              prototype->getPrototypeObject(state);
          if (maybePrototype.hasValue()) {
            printObject(maybePrototype.value(), depth + 2);
          }
        }
        return ValueRef::createNull();
      },
      value,
      depth);
  LWNODE_CHECK(r.isSuccessful());
}

void DebugUtils::printToString(ValueRef* value) {
  auto context = IsolateWrap::GetCurrent()->GetCurrentContext()->get();
  EvalResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ValueRef* value) -> ValueRef* {
        auto valueString = value->toString(state);
        LWNODE_LOGR("toString: %s\n", valueString->toStdUTF8String().c_str());
        return value;
      },
      value);
}

std::string DebugUtils::v8StringToStd(Isolate* isolate, Local<Value> value) {
  Local<String> s;
  value->ToString(isolate->GetCurrentContext()).ToLocal(&s);
  v8::String::Utf8Value utf8String(isolate, s);
  return std::string(*utf8String);
}

}  // namespace EscargotShim
