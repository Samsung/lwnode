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

#pragma once

#include <GCUtil.h>
#include <functional>
#include <string>
#include <vector>

#include "utils/sf-vector.h"

namespace Escargot {
class ValueRef;
class ObjectRef;
}  // namespace Escargot

// typedef of GC-aware vector
template <typename T,
          bool isEraseStrategyStrict = false,
          typename Allocator = GCUtil::gc_malloc_allocator<T>>
using GCVectorT = Starfish::Vector<T, Allocator, isEraseStrategyStrict>;

#if defined(GC_DEBUG)
template <typename T,
          bool isEraseStrategyStrict = true,
          typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCVector : public GCVectorT<T, isEraseStrategyStrict, Allocator>,
                 public gc {};
#else
template <typename T,
          bool isEraseStrategyStrict = false,
          typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCVector : public GCVectorT<T, isEraseStrategyStrict, Allocator>,
                 public gc {};
#endif

class GCTracer {
 public:
  ~GCTracer();
  void add(void* gcPtr, std::string description = "");
  void add(Escargot::ObjectRef* gcPtr, std::string description = "");
  void printState();
  size_t getAllocatedCount();
  void reset();

 private:
  void setAddressDeallocatd(void* gcPtr);

  struct Address {
    void* ptr = nullptr;
    std::string description;
    bool deallocated = false;
  };

  std::vector<Address> registeredAddress_;
};

enum WarnEventType {
  POOR_PERFORMANCE,
  FAILED_TO_EXPAND_HEAP,
  OUT_OF_MEMORY,
};

enum class GarbageCollectionReason {
  // NOTE: From the following enum values from v8,
  // we only use a few.
  // kUnknown = 0,
  // kAllocationFailure = 1,
  // kAllocationLimit = 2,
  // kContextDisposal = 3,
  // kCountersExtension = 4,
  // kDebugger = 5,
  // kDeserializer = 6,
  // kExternalMemoryPressure = 7,
  // kFinalizeMarkingViaStackGuard = 8,
  // kFinalizeMarkingViaTask = 9,
  // kFullHashtable = 10,
  // kHeapProfiler = 11,
  // kTask = 12,
  // kLastResort = 13,
  // kLowMemoryNotification = 14,
  // kMakeHeapIterable = 15,
  // kMemoryPressure = 16,
  // kMemoryReducer = 17,
  kRuntime = 18,
  // kSamplingProfiler = 19,
  // kSnapshotCreator = 20,
  kTesting = 21,
  // kExternalFinalize = 22,
  // kGlobalAllocationLimit = 23,
  // kMeasureMemory = 24
};

class MemoryUtil {
 public:
  typedef void (*OnGCWarnEventListener)(WarnEventType type);

  static void Initialize();

  static void GcSetWarningListener(OnGCWarnEventListener callback);
  static void GcPrintGCMemoryUsage(void* data);
  static void GcFull();
  static void GcInvokeFinalizers();
  static void Gc();

  typedef void (*GCAllocatedMemoryFinalizer)(void* self);
  typedef void (*GCAllocatedMemoryFinalizerWithData)(void* self, void* data);
  // @note this should not use on escargot values since they may be already
  // bound with another finalizer with its internal data.
  static void GcRegisterFinalizer(void* gcPtr,
                                  GCAllocatedMemoryFinalizerWithData callback,
                                  void* data);

  static void GcRegisterFinalizer(Escargot::ValueRef* gcPtr,
                                  GCAllocatedMemoryFinalizer callback);

  static void GcUnregisterFinalizer(Escargot::ValueRef* gcPtr,
                                    GCAllocatedMemoryFinalizer callback);
  // print
  static void PrintRegisteredGCObjects();
  static void PrintEveryReachableGCObjects();
  static void PrintGCStats();
  static void PrintBacktrace(void* gcPtr);
  static void PrettyBytes(char* buf,
                          size_t nBuf,
                          size_t bytes,
                          std::function<bool(uint, double)> filter = nullptr);
  static GCTracer tracer;
};
