#ifndef SRC_NODE_PERF_COMMON_H_
#define SRC_NODE_PERF_COMMON_H_

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "aliased_buffer.h"
#include "node.h"
#include "uv.h"
#include "v8.h"

#include <algorithm>
#include <map>
#include <string>

namespace node {
namespace performance {

#define PERFORMANCE_NOW() uv_hrtime()

// These occur before the environment is created. Cache them
// here and add them to the milestones when the env is init'd.
extern uint64_t performance_v8_start;

#define NODE_PERFORMANCE_MILESTONES(V)                                        \
  V(ENVIRONMENT, "environment")                                               \
  V(NODE_START, "nodeStart")                                                  \
  V(V8_START, "v8Start")                                                      \
  V(LOOP_START, "loopStart")                                                  \
  V(LOOP_EXIT, "loopExit")                                                    \
  V(BOOTSTRAP_COMPLETE, "bootstrapComplete")


#define NODE_PERFORMANCE_ENTRY_TYPES(V)                                       \
  V(NODE, "node")                                                             \
  V(MARK, "mark")                                                             \
  V(MEASURE, "measure")                                                       \
  V(GC, "gc")                                                                 \
  V(FUNCTION, "function")                                                     \
  V(HTTP2, "http2")                                                           \
  V(HTTP, "http")

enum PerformanceMilestone {
#define V(name, _) NODE_PERFORMANCE_MILESTONE_##name,
  NODE_PERFORMANCE_MILESTONES(V)
#undef V
  NODE_PERFORMANCE_MILESTONE_INVALID
};

enum PerformanceEntryType {
#define V(name, _) NODE_PERFORMANCE_ENTRY_TYPE_##name,
  NODE_PERFORMANCE_ENTRY_TYPES(V)
#undef V
  NODE_PERFORMANCE_ENTRY_TYPE_INVALID
};

class PerformanceState {
 public:
  explicit PerformanceState(v8::Isolate* isolate) :
    root(
      isolate,
      sizeof(performance_state_internal)),
    milestones(
      isolate,
      offsetof(performance_state_internal, milestones),
      NODE_PERFORMANCE_MILESTONE_INVALID,
      root),
    observers(
      isolate,
      offsetof(performance_state_internal, observers),
      NODE_PERFORMANCE_ENTRY_TYPE_INVALID,
      root) {
    for (size_t i = 0; i < milestones.Length(); i++)
      milestones[i] = -1.;
  }

  AliasedUint8Array root;
  AliasedFloat64Array milestones;
  AliasedUint32Array observers;

  uint64_t performance_last_gc_start_mark = 0;

  void Mark(enum PerformanceMilestone milestone,
            uint64_t ts = PERFORMANCE_NOW());

 private:
  struct performance_state_internal {
    // doubles first so that they are always sizeof(double)-aligned
    double milestones[NODE_PERFORMANCE_MILESTONE_INVALID];
    uint32_t observers[NODE_PERFORMANCE_ENTRY_TYPE_INVALID];
  };
};

}  // namespace performance
}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_NODE_PERF_COMMON_H_
