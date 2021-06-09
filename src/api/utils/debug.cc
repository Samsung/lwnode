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
#include "debug.h"
#include "logger.h"

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

  LWNODE_LOG_RAW("\n[Backtrace]");
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
          LWNODE_LOG_RAW("#%-2d: %s", i - 1, line);
          isFirstLine = false;
        } else {
          LWNODE_LOG_RAW("%s", line);
        }
      }
      LWNODE_LOG_RAW("");
      pclose(stream);
    } else if (symbols[i]) {
      LWNODE_LOG_RAW("#%-2d: %s", i - 1, symbols[i]);
    }
  }
#endif
}

}  // namespace EscargotShim
