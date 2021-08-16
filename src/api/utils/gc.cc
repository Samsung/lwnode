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
#include "misc.h"

using namespace Escargot;

#define LOG_HANDLER(msg, ...)                                                  \
  do {                                                                         \
    if (EscargotShim::Flags::isTraceGCEnabled()) {                             \
      LWNODE_LOG_RAW(CLR_DIM msg CLR_RESET, ##__VA_ARGS__);                    \
    }                                                                          \
  } while (0)

#define LOG_HANDLER_RAW(msg, ...)                                              \
  do {                                                                         \
    printf(msg, ##__VA_ARGS__);                                                \
  } while (0)

// --- GCTracer ---

GCTracer MemoryUtil::tracer;

#define GC_DEREF_OFFSET 1
#define TO_FAKEPTR(gcPtr) ((void*)((size_t)gcPtr + GC_DEREF_OFFSET))
#define TO_GCPTR(ptr) ((void*)((size_t)ptr - GC_DEREF_OFFSET))

#define REGISTER_FINALIZER(obj, fn, data)                                      \
  GC_REGISTER_FINALIZER_NO_ORDER(obj, fn, data, nullptr, nullptr)

#define DEREGISTER_FINALIZER(obj)                                              \
  GC_REGISTER_FINALIZER_NO_ORDER(obj, nullptr, nullptr, nullptr, nullptr)

GCTracer::~GCTracer() {
  reset();
}

void GCTracer::reset() {
  for (const auto& it : registeredAddress_) {
    DEREGISTER_FINALIZER(TO_GCPTR(it.ptr));
  }
  registeredAddress_.clear();
}

void GCTracer::add(void* gcPtr, std::string description) {
  registeredAddress_.push_back(Address{TO_FAKEPTR(gcPtr), description, false});

  // @todo support multiple finalizer
  REGISTER_FINALIZER(
      gcPtr,
      [](void* gcPtr, void* data) {
        auto self = reinterpret_cast<GCTracer*>(data);
        self->setAddressDeallocatd(gcPtr);
      },
      this);
}

void GCTracer::add(Escargot::ObjectRef* ptr, std::string description) {
  // @todo
  LWNODE_CHECK_NOT_REACH_HERE();
}

void GCTracer::setAddressDeallocatd(void* gcPtr) {
  LWNODE_CALL_TRACE_ID(GCDEBUG, "de-allocated: %p", gcPtr);
  LWNODE_CHECK_NOT_NULL(gcPtr);

  for (auto& it : registeredAddress_) {
    if (it.ptr == TO_FAKEPTR(gcPtr)) {
      it.deallocated = true;
      return;
    }
  }

  LWNODE_CHECK_NOT_REACH_HERE();
}

void GCTracer::printState() {
  LOG_HANDLER("");
  GC_gcollect();
  GC_disable();

  for (const auto& it : registeredAddress_) {
    if (it.deallocated) {
      LOG_HANDLER(CLR_MAGENTA "deallocated: %s (%p)",
                  it.description.c_str(),
                  TO_GCPTR(it.ptr));
    } else {
      LOG_HANDLER(CLR_YELLOW "backtrace of %s (%p):\n",
                  it.description.c_str(),
                  TO_GCPTR(it.ptr));

      MemoryUtil::printBacktrace(TO_GCPTR(it.ptr));
    }
  }

  GC_enable();
  LOG_HANDLER("");
  // GC_print_heap_usage();
}

size_t GCTracer::getAllocatedCount() {
  size_t count = 0;
  for (auto& it : registeredAddress_) {
    if (it.deallocated == false) {
      count++;
    }
  }
  return count;
}

// --- MemoryUtil ---

static std::function<bool(uint, double)> g_filter = [](uint idx, double value) {
  // @note bug walkaround: GC print returns an underflowed value at termination.
  if (idx >= 5 && value >= 10) {
    return true;
  }
  return false;
};

#define THRESHOLD_BYTES_BACKTRACE 2048

void MemoryUtil::gcStartStatsTrace() {
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
}

static thread_local MemoryUtil::OnGCWarnEventListener g_gcWarnEventListener;

void MemoryUtil::gcSetWarningListener(OnGCWarnEventListener callback) {
  if (g_gcWarnEventListener == nullptr) {
    g_gcWarnEventListener = callback;

    GC_set_warn_proc([](char* format, GC_word arg) {
      /*
        GC Warning: ...May lead to memory leak and poor performance
        GC Warning: ...Failed to expand heap
        GC Warning: Out of Memory! ... Returning NULL!
      */
      std::string message = format;

#if !defined(NDEBUG)
      LOG_HANDLER_RAW(format, arg);
#endif

      if (message.find("poor performance") != std::string::npos) {
        g_gcWarnEventListener(POOR_PERFORMANCE);
      } else if (message.find("Failed to expand heap") != std::string::npos) {
        g_gcWarnEventListener(FAILED_TO_EXPAND_HEAP);
      } else if (message.find("Out of Memory") != std::string::npos) {
        g_gcWarnEventListener(OUT_OF_MEMORY);
      }
    });
  }
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
#else
  LOG_HANDLER("%s works in debug build only", __PRETTY_FUNCTION__);
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
          MemoryUtil::printBacktrace(ptr);
        }
#endif
      },
      &temp);
  GC_enable();

  LOG_HANDLER("<-- end of print reachable pointers %fKB (count: %zu)\n",
              temp.totalRemainSize / 1024.f,
              temp.totalCount);
}

void MemoryUtil::gcFull() {
  LWNODE_CALL_TRACE_GC_START();
  LOG_HANDLER("[FULL GC]");
  GC_register_mark_stack_func([]() {
    // do nothing for skip stack
    // assume there is no gc-object on stack
  });

  GC_gcollect();
  GC_gcollect();
  GC_gcollect_and_unmap();
  GC_register_mark_stack_func(nullptr);
  GC_gcollect();
  LWNODE_CALL_TRACE_GC_END();
}

void MemoryUtil::gc() {
  LWNODE_CALL_TRACE_GC_START();
  LOG_HANDLER("[GC]");

  for (int i = 0; i < 5; ++i) {
    GC_gcollect_and_unmap();
  }

  for (int i = 0; i < 5; i++) {
    GC_gcollect();
  }

  LWNODE_CALL_TRACE_GC_END();
}

void MemoryUtil::gcInvokeFinalizers() {
  GC_invoke_finalizers();
}

void MemoryUtil::gcEndStatsTrace() {
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
  while (c >= 1024 && s < 7 - 1) {
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

void MemoryUtil::gcRegisterFinalizer(Escargot::ValueRef* ptr,
                                     GCAllocatedMemoryFinalizer callback) {
  Escargot::Memory::gcRegisterFinalizer(ptr->asObject(), callback);
}

void MemoryUtil::gcRegisterFinalizer(EscargotShim::ValueWrap* ptr,
                                     GCAllocatedMemoryFinalizer callback) {
  Escargot::Memory::gcRegisterFinalizer(ptr, callback);
}

void MemoryUtil::gcUnregisterFinalizer(Escargot::ValueRef* ptr,
                                       GCAllocatedMemoryFinalizer callback) {
  Escargot::Memory::gcUnregisterFinalizer(ptr->asObject(), callback);
}

void MemoryUtil::gcRegisterFinalizer(
    void* gcPtr, GCAllocatedMemoryFinalizerWithData callback, void* data) {
  REGISTER_FINALIZER(gcPtr, callback, data);
}
