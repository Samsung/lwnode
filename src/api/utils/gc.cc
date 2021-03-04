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

#include "gc.h"
#include <EscargotPublic.h>
#include <GCUtil.h>
#include "logger.h"

using namespace Escargot;

#define LOG_HANDLER(msg, ...)                                                  \
  do {                                                                         \
    LWNODE_LOG_RAW(COLOR_DIM "GC " msg COLOR_RESET, ##__VA_ARGS__);            \
  } while (0)

static std::function<bool(uint, double)> g_filter = [](uint idx, double value) {
  // @note bug walkaround: GC print returns an underflowed value at termination.
  if (idx >= 5 && value >= 10) {
    return true;
  }
  return false;
};

#define THRESHOLD_BYTES_BACKTRACE 2048

void MemoryUtil::startGCStatsTrace() {
  Memory::setGCEventListener([]() {
    static size_t last_use = 0, last_heap = 0;

    size_t nuse = GC_get_memory_use();
    size_t nheap = GC_get_heap_size();

    if (last_use == nuse && last_heap == nheap) {
      // skip
      return;
    } else {
      last_use = nuse;
      last_heap = nheap;
    }

    char heap[20], use[20];
    prettyBytes(use, sizeof(use), nuse);
    prettyBytes(heap, sizeof(heap), nheap, g_filter);
    LOG_HANDLER("use %s, heap %s", use, heap);
  });

#ifdef ENABLE_GC_WARN
  GC_set_warn_proc([](char* msg, GC_word arg) {
    char formatted[1024];
    snprintf(formatted, sizeof(formatted) - 1, msg, arg);
    LOG_HANDLER("[WARN] %s\n", formatted);
  });
#endif
}

void MemoryUtil::printGCStats() {
  // struct GC_prof_stats_s stats;
  // GC_get_prof_stats(&stats, sizeof(stats));

  char u[20], h[20];
  prettyBytes(u, sizeof(u), GC_get_memory_use());
  prettyBytes(h, sizeof(h), GC_get_heap_size(), g_filter);
  LOG_HANDLER("use %s, heap %s", u, h);
}

void MemoryUtil::printBacktrace(void* gcPtr) {
#if !defined(NDEBUG)
  GC_print_backtrace(gcPtr);
#endif
}

void MemoryUtil::printEveryReachableGCObjects() {
  LOG_HANDLER("print reachable pointers -->\n");
  GC_gcollect();
  GC_disable();

  struct temp_t {
    size_t totalCount = 0;
    size_t totalRemainSize = 0;
  } temp;

  size_t totalRemainSize = 0;

  GC_enumerate_reachable_objects_inner(
      [](void* obj, size_t bytes, void* cd) {
        size_t size;
        int kind = GC_get_kind_and_size(obj, &size);
        void* ptr = GC_USR_PTR_FROM_BASE(obj);

        temp_t* tmp = (temp_t*)cd;
        tmp->totalRemainSize += size;
        tmp->totalCount++;

        // @note about kinds: bdwgc/include/private/gc_priv.h:1511
        LOG_HANDLER(
            "@@@ kind %d pointer %p size %d B", (int)kind, ptr, (int)size);

#if !defined(NDEBUG)
        // details
        if (size > THRESHOLD_BYTES_BACKTRACE) {
          GC_print_backtrace(ptr);
        }
#endif
      },
      &temp);
  GC_enable();

  LOG_HANDLER("<-- end of print reachable pointers %fKB (count: %ld)\n",
              temp.totalRemainSize / 1024.f,
              temp.totalCount);
}

void MemoryUtil::collectAllGarbage() {
  GC_register_mark_stack_func([]() {
    // do nothing for skip stack
    // assume there is no gc-object on stack
  });

  GC_gcollect();
  GC_gcollect();
  GC_gcollect_and_unmap();
  GC_register_mark_stack_func(nullptr);
  GC_gcollect();
}

void MemoryUtil::endGCStatsTrace() {
  Memory::setGCEventListener(nullptr);
}

void MemoryUtil::prettyBytes(char* buf,
                             size_t nBuf,
                             size_t bytes,
                             std::function<bool(uint, double)> filter) {
  const char* suffix[7] = {
      "B",
      "KB",
      "MB",
      "GB",
      "TB",
      "PB",
      "EB",
  };
  uint s = 0;
  double c = bytes;
  while (c >= 1024 && s < 7) {
    c /= 1024;
    s++;
  }

  if (filter != nullptr && filter(s, c)) {
    snprintf(buf, nBuf, "0 B (F)");
    return;
  }

  if (c - ((int)c) == 0.f) {
    snprintf(buf, nBuf, "%d %s", (int)c, suffix[s]);
  } else {
    snprintf(buf, nBuf, "%.3f %s", c, suffix[s]);
  }
}
