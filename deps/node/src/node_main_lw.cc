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

#include <cstdio>
#include "lwnode/lwnode-public.h"
#include "node.h"

// UNIX
#ifdef __linux__
#include <elf.h>
#ifdef __LP64__
#define Elf_auxv_t Elf64_auxv_t
#else
#define Elf_auxv_t Elf32_auxv_t
#endif  // __LP64__
extern char** environ;
#endif  // __linux__
#if defined(__POSIX__) && defined(NODE_SHARED_MODE)
#include <signal.h>
#include <string.h>
#endif

namespace node {
namespace per_process {
extern bool linux_at_secure;
}  // namespace per_process
}  // namespace node

int main(int argc, char* argv[]) {
#if defined(__POSIX__) && defined(NODE_SHARED_MODE)
  // In node::PlatformInit(), we squash all signal handlers for non-shared lib
  // build. In order to run test cases against shared lib build, we also need
  // to do the same thing for shared lib build here, but only for SIGPIPE for
  // now. If node::PlatformInit() is moved to here, then this section could be
  // removed.
  {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, nullptr);
  }
#endif

#if defined(__linux__)
  char** envp = environ;
  while (*envp++ != nullptr) {
  }
  Elf_auxv_t* auxv = reinterpret_cast<Elf_auxv_t*>(envp);
  for (; auxv->a_type != AT_NULL; auxv++) {
    if (auxv->a_type == AT_SECURE) {
      node::per_process::linux_at_secure = auxv->a_un.a_val;
      break;
    }
  }
#endif
  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);

  lwnode::Runtime runtime;
  std::pair<bool, int> init_result; // early_return, exit_code
  // FIXME: Fix Runtime::Init() call to ensure environment initialization
  // before running the loop, Runtime::Run(). This workaround passes a
  // promise directly to know when that is.
  std::promise<void> promise;

  if (lwnode::ParseAULEvent(argc, argv)) {
    if (!lwnode::InitScriptRootPath()) {
      exit(-errno);
    }
    char* args[] = {"", "index.js", nullptr};
    init_result = runtime.Init(2, args, std::move(promise));
  } else {
    // started by command line
    init_result = runtime.Init(argc, argv, std::move(promise));
  }

  if (init_result.first) {
    return init_result.second;
  }

  return runtime.Run();
}
