/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include "nd-debug.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

static const int kStackTraceFrameSize = 128;

DebugUtil::CustomHandler DebugUtil::custom_handler_;

static void SignalHandler(int signal) {
  if (DebugUtil::custom_handler()) {
    DebugUtil::custom_handler()(signal);
  }

  void* callstack[kStackTraceFrameSize];
  int nr_frames = backtrace(callstack, sizeof(callstack) / sizeof(void*));
  char** strs = backtrace_symbols(callstack, nr_frames);
  int c = 0;
  for (int i = 0; i < nr_frames; i++) {
    if (strs[i] == nullptr) {
      continue;
    }

    char* mangled_name = nullptr;
    char* offset_begin = nullptr;
    char* offset_end = nullptr;

    // Find parentheses and +address offset surrounding mangled name
    for (char* p = strs[i]; *p; ++p) {
      if (*p == '(') {
        mangled_name = p;
      } else if (*p == '+') {
        offset_begin = p;
      } else if (*p == ')') {
        offset_end = p;
        break;
      }
    }

    if (mangled_name && offset_begin && offset_end &&
        mangled_name < offset_begin) {
      *mangled_name++ = '\0';
      *offset_begin++ = '\0';
      *offset_end = '\0';

      int status;
      char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

      if (status == 0) {
        printf("#%d %s : %s+%s\n", ++c, strs[i], real_name, offset_begin);
      } else {
        printf("#%d %s : %s+%s\n", ++c, strs[i], mangled_name, offset_begin);
      }
      free(real_name);
    } else {
      printf("%s\n", strs[i]);
    }
  }
  free(strs);
  exit(1);
}

#ifdef __SANITIZE_ADDRESS__
static DebugUtil g_util;

DebugUtil::~DebugUtil() {
  printf("\nAddressSanitizer (ASan) ran.\n");
}
#endif

void DebugUtil::SetupSignalHandler(CustomHandler custom_handler) {
  struct sigaction sa;
  sa.sa_handler = (void (*)(int))SignalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);

  custom_handler_ = custom_handler;
}
