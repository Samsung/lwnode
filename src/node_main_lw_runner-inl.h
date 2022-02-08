#pragma once
#include "node_bindings.h"
#include "node_internals.h"
#include "node_main_instance.h"
#include "node_options-inl.h"
#include "node_v8_platform-inl.h"
#include "util-inl.h"

#if defined(LEAK_SANITIZER)
#include <sanitizer/lsan_interface.h>
#endif

using v8::Context;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Locker;
using v8::SealHandleScope;
using namespace node;

namespace LWNode {

class MainLoopStrategy {
 public:
  virtual void RunLoop(node::Environment* env) = 0;
};

class GmainLoopStrategy : public MainLoopStrategy, public GmainLoopWork {
 public:
  void RunLoop(node::Environment* env) override {
    CHECK_NOT_NULL(env);
    env_ = env;

    LWNode::GmainLoopNodeBindings node_bindings(this);
    node_bindings.StartEventLoop();
  }

  bool RunOnce() override {
    CHECK_NOT_NULL(env_);

    auto event_loop = env_->event_loop();

    uv_run(event_loop, UV_RUN_NOWAIT);

    per_process::v8_platform.DrainVMTasks(env_->isolate());

    bool more = uv_loop_alive(event_loop);
    if (more && !env_->is_stopping()) {
      return true;
    }

    if (!uv_loop_alive(event_loop)) {
      EmitBeforeExit(env_);
    }

    more = uv_loop_alive(event_loop);

    return (more == true && !env_->is_stopping());
  }

 private:
  node::Environment* env_ = nullptr;
};

class LoopStrategy : public MainLoopStrategy {
  void RunLoop(node::Environment* env) override {
    bool more;
    do {
      uv_run(env->event_loop(), UV_RUN_DEFAULT);

      per_process::v8_platform.DrainVMTasks(env->isolate());

      more = uv_loop_alive(env->event_loop());
      if (more && !env->is_stopping()) continue;

      if (!uv_loop_alive(env->event_loop())) {
        EmitBeforeExit(env);
      }

      // Emit `beforeExit` if the loop became alive either after emitting
      // event, or after running some callbacks.
      more = uv_loop_alive(env->event_loop());
    } while (more == true && !env->is_stopping());
  }
};

class LWNodeMainRunner {
 public:
  ~LWNodeMainRunner() { v8::V8::ShutdownPlatform(); }

  int Run(node::NodeMainInstance& nodeMainInstance) {
    // To release array buffer allocator after node is finished,
    // this runner should has it.
    array_buffer_allocator_ =
        std::move(nodeMainInstance.array_buffer_allocator_);

    v8::Isolate* isolate_ = nodeMainInstance.isolate_;

    Locker locker(isolate_);
    Isolate::Scope isolate_scope(isolate_);
    HandleScope handle_scope(isolate_);

    int exit_code = 0;
    DeleteFnPtr<Environment, FreeEnvironment> env_ =
        nodeMainInstance.CreateMainEnvironment(&exit_code);

    CHECK_NOT_NULL(env_);
    Context::Scope context_scope(env_->context());

    if (exit_code == 0) {
      LoadEnvironment(env_.get());

      env_->set_trace_sync_io(env_->options()->trace_sync_io);

      {
        SealHandleScope seal(isolate_);
        env_->performance_state()->Mark(
            node::performance::NODE_PERFORMANCE_MILESTONE_LOOP_START);

        // Run main loop
        std::unique_ptr<MainLoopStrategy> mainLoop =
            std::make_unique<LoopStrategy>();
        mainLoop->RunLoop(env_.get());

        env_->performance_state()->Mark(
            node::performance::NODE_PERFORMANCE_MILESTONE_LOOP_EXIT);
      }

      env_->set_trace_sync_io(false);
      exit_code = EmitExit(env_.get());
    }

    ResetStdio();

    // TODO(addaleax): Neither NODE_SHARED_MODE nor HAVE_INSPECTOR really
    // make sense here.
#if HAVE_INSPECTOR && defined(__POSIX__) && !defined(NODE_SHARED_MODE)
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    for (unsigned nr = 1; nr < kMaxSignal; nr += 1) {
      if (nr == SIGKILL || nr == SIGSTOP || nr == SIGPROF) continue;
      act.sa_handler = (nr == SIGPIPE) ? SIG_IGN : SIG_DFL;
      CHECK_EQ(0, sigaction(nr, &act, nullptr));
    }
#endif

    // @lwnode
    // We prevent premature termination when detecting leak,
    // as our GC runs after shutting down node platform.
    LWNode::IdleGC();
#if defined(LEAK_SANITIZER)
    __lsan_do_leak_check();
#endif
    return exit_code;
  }

 private:
  std::unique_ptr<node::ArrayBufferAllocator> array_buffer_allocator_;
};

}  // namespace LWNode
