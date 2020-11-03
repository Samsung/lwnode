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

#include "v8.h"
#include <algorithm>
#include "escargotutil.h"
// #include "v8-debug.h"
#include "escargotisolateshim.h"
#include "escargotplatform.h"
#ifndef _WIN32
#include "escargotshim-version.h"
#endif

namespace v8 {

static const size_t kMaxVersionLength = 64;

bool g_alive = false;
bool g_exposeGC = false;
bool g_useStrict = false;
bool g_disableIdleGc = false;
bool g_trace_debug_json = false;

HeapStatistics::HeapStatistics()
    : total_heap_size_(0),
      total_heap_size_executable_(0),
      total_physical_size_(0),
      total_available_size_(0),
      used_heap_size_(0),
      heap_size_limit_(0),
      malloced_memory_(0),
      peak_malloced_memory_(0),
      does_zap_garbage_(0) {}

const char* V8::GetVersion() {
  static char versionStr[kMaxVersionLength] = {};

  if (versionStr[0] == '\0') {
#ifdef _WIN32
    NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
// HMODULE hModule;
// if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
//                       TEXT(NODE_ENGINE), &hModule)) {
//   WCHAR filename[_MAX_PATH];  // NOLINT(runtime/arrays)
//   DWORD len = GetModuleFileNameW(hModule, filename, _countof(filename));
//   if (len > 0) {
//     DWORD dwHandle = 0;
//     DWORD size = GetFileVersionInfoSizeExW(0, filename, &dwHandle);
//     if (size > 0) {
//       std::unique_ptr<BYTE[]> info(new BYTE[size]);
//       if (GetFileVersionInfoExW(0, filename, dwHandle, size, info.get()))
//       {
//         UINT len = 0;
//         VS_FIXEDFILEINFO* vsfi = nullptr;
//         if (VerQueryValueW(info.get(), L"\\",
//                            reinterpret_cast<LPVOID*>(&vsfi), &len)) {
//           sprintf_s(
//               versionStr, "%d.%d.%d.%d", HIWORD(vsfi->dwFileVersionMS),
//               LOWORD(vsfi->dwFileVersionMS),
//               HIWORD(vsfi->dwFileVersionLS),
//               LOWORD(vsfi->dwFileVersionLS));
//         }
//       }
//     }
//   }
// }
#else
    snprintf(versionStr, kMaxVersionLength, "%s.%s", GIT_REV, BUILD_TIME);
#endif
  }
  return versionStr;
}

void V8::SetFlagsFromString(const char* str) {
  // TODO: Considering: Parsing V8 flags from a string if needed
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(str);
}

void V8::SetFlagsFromString(const char* str, int length) {
  // TODO: Considering: Parsing V8 flags from a string if needed
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(str);
}

static bool equals(const char* str, const char* pat) {
  return strcmp(str, pat) == 0;
}

template <size_t N>
static bool startsWith(const char* str, const char (&prefix)[N]) {
  return strncmp(str, prefix, N - 1) == 0;
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  for (int i = 1; i < *argc; i++) {
    // Note: Node now exits on invalid options. We may not recognize V8 flags
    // and fail here, causing Node to exit.
    char* arg = argv[i];
    if (equals("--expose-gc", arg) || equals("--expose_gc", arg)) {
      g_exposeGC = true;
      if (remove_flags) {
        argv[i] = nullptr;
      }
    } else if (equals("--use-strict", arg) || equals("--use_strict", arg)) {
      g_useStrict = true;
      if (remove_flags) {
        argv[i] = nullptr;
      }
    } else if (equals("--off-idlegc", arg) || equals("--off_idlegc", arg)) {
      g_disableIdleGc = true;
      if (remove_flags) {
        argv[i] = nullptr;
      }
    } else if (equals("--trace-debug-json", arg) ||
               equals("--trace_debug_json", arg)) {
      g_trace_debug_json = true;
      if (remove_flags) {
        argv[i] = nullptr;
      }
    } else if (remove_flags &&
               (startsWith(
                    arg,
                    "--debug")  // Ignore some flags to reduce unit test noise
                || startsWith(arg, "--harmony") ||
                startsWith(arg, "--stack-size=") ||
                startsWith(arg, "--nolazy"))) {
      argv[i] = nullptr;
    } else if (equals("--help", arg)) {
      printf("Options:\n"
             " --use_strict (enforce strict mode)\n"
             "     type: bool  default: false\n"
             " --expose_gc (expose gc extension)\n"
             "     type: bool  default: false\n"
             " --off_idlegc (turn off idle GC)\n"
             " --harmony_simd (enable \"harmony simd\" (in progress))\n"
             " --harmony (Other flags are ignored in node running with "
             "chakracore)\n"
             " --debug (Ignored in node running with chakracore)\n"
             " --stack-size (Ignored in node running with chakracore)\n");
      exit(0);
    }
  }

  if (remove_flags) {
    char** end = std::remove(argv + 1, argv + *argc, nullptr);
    *argc = static_cast<int>(end - argv);
  }
}

bool V8::Initialize() {
  if (!g_alive) {
    EscargotShim::InitializeEscargotShim();
    g_alive = true;
  }
  return true;
}

void V8::SetEntropySource(EntropySource entropy_source) {
  NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE("");
}

bool V8::IsDead() {
  return !g_alive;
}

bool V8::Dispose() {
  if (g_alive) {
    // ----------------------------------------------------------------------
    EscargotShim::FinalizeEscargotShim();
    // ----------------------------------------------------------------------
    // (!) NOTE: V8::IsDead() is invoked during GC. So `g_alive` should be set
    // after GC is finished.
    g_alive = false;
    // ----------------------------------------------------------------------
  }
  return true;
}

void V8::TerminateExecution(Isolate* isolate) {
  isolate->TerminateExecution();
}

bool V8::IsExeuctionDisabled(Isolate* isolate) {
  // return jsrt::IsolateShim::FromIsolate(isolate)->IsExeuctionDisabled();
  NESCARGOT_UNIMPLEMENTED("");
  return false;
}

void V8::CancelTerminateExecution(Isolate* isolate) {
  isolate->CancelTerminateExecution();
}

void V8::FromJustIsNothing() {
  // "Maybe value is Nothing.
  NESCARGOT_FATAL_ERROR();
}

void V8::ToLocalEmpty() {
  // NOTE: `Empty MaybeLocal` MUST not fail in Escargot"
  NESCARGOT_FATAL_ERROR();
}

void V8::ShutdownPlatform() {
  NESCARGOT_UNIMPLEMENTED("");
}

Maybe<bool> FinalizationGroup::Cleanup(
    Local<FinalizationGroup> finalization_group) {
  NESCARGOT_UNIMPLEMENTED("");
  return Nothing<bool>();
}

double Platform::SystemClockTimeMillis() {
  NESCARGOT_UNIMPLEMENTED("");
  return 0;
}

}  // namespace v8
