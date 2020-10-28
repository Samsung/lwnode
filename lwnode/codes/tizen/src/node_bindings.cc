/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
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

#include <glib.h>
#include <cassert>
#include "uv.h"
#include "node_bindings.h"
#include "node_escargot_logger.h"
#ifdef HOST_TIZEN
#include "Extension.h"
#endif

namespace glib {

using namespace nescargot;

struct SourceData {
  GSource source;
  gpointer tag;
  NodeBindings* node_bindings;
};

// TODO: classify EventLoop if needed

static GMainContext* gcontext;
static GMainLoop* gmainLoop;
static GSource* uvsource;
static GSourceFuncs source_funcs;
static bool gmainLoopDone = false;

static gboolean GmainLoopPrepareCallback(GSource* source, gint* timeout) {
  uv_update_time(uv_default_loop());
  *timeout = uv_backend_timeout(uv_default_loop());

  if (!uv_watcher_queue_empty(uv_default_loop())) {
    return TRUE;
  }

  return 0 == *timeout;
}

static gboolean GmainLoopCheckCallback(GSource* source) {
  if (!uv_backend_timeout(uv_default_loop())) {
    return TRUE;
  }

  return (G_IO_IN ==
          g_source_query_unix_fd(source, ((SourceData*)source)->tag));
}

static gboolean GmainLoopDispatchCallback(GSource* source, GSourceFunc callback,
                                          gpointer user_data) {
  assert(gcontext);
  g_main_context_iteration(gcontext, FALSE);

  if (gmainLoopDone) {
    return G_SOURCE_REMOVE;
  }

  NodeBindings* node_bindings = ((SourceData*)source)->node_bindings;

  node_bindings->RunOnce();

  if (!node_bindings->HasMoreTasks()) {
    g_main_loop_quit(gmainLoop);
    return G_SOURCE_REMOVE;
  }
  return G_SOURCE_CONTINUE;
}

void GmainLoopInit(NodeBindings* self) {
  gcontext = g_main_context_default();
  gmainLoop = g_main_loop_new(gcontext, FALSE);
  source_funcs = {
      .prepare = GmainLoopPrepareCallback,
      .check = GmainLoopCheckCallback,
      .dispatch = GmainLoopDispatchCallback,
  };

  uvsource = g_source_new(&source_funcs, sizeof(SourceData));
  ((SourceData*)uvsource)->tag = g_source_add_unix_fd(
      uvsource, uv_backend_fd(uv_default_loop()),
      (GIOCondition)(G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_PRI));
  ((SourceData*)uvsource)->node_bindings = self;

  g_source_attach(uvsource, gcontext);
  g_source_unref(uvsource);

#ifdef HOST_TIZEN
  DeviceAPI::ESPostMessageListener::SetMainThreadIdlerRegister(
      [](DeviceAPI::ESPostMessageListener::Idler_t idler, void* data) {
        g_idle_add(idler, data);
      });
#endif
}

void GmainLoopStart() {
  assert(gmainLoop);
  assert(gcontext);

  g_main_loop_run(gmainLoop);
  gmainLoopDone = true;
  g_main_context_iteration(gcontext, TRUE);
}

void GmainLoopExit() {
  if (uvsource) {
    g_source_destroy(uvsource);
  }
  if (gmainLoop) {
    g_main_loop_unref(gmainLoop);
  }
  if (gcontext) {
    g_main_context_unref(gcontext);
  }
}

}  // namespace glib

#ifdef HOST_TIZEN
#include "Queue.hpp"
#include <mutex>
#include <thread>

#define NESCARGOT_AUL_TERMINATION_MESSAGE "AUL_TERMINATION"

namespace nescargot {

struct Task {
  std::string data;
};

Queue<Task> g_queue;

static void UvNoOp(uv_async_t* handle) {
  uv_close((uv_handle_t*)handle,
           [](uv_handle_t* handle) { delete (uv_async_t*)handle; });
}

void push_aul_message(const char* message) {
  g_queue.push({.data = message});
  // wake up the uv queue
  uv_async_t* async = new uv_async_t;
  uv_async_init(uv_default_loop(), async, UvNoOp);
  uv_async_send(async);
}

void push_aul_termination_message() {
  push_aul_message(NESCARGOT_AUL_TERMINATION_MESSAGE);
}
}  // namespace nescargot
#endif

namespace nescargot {

static void pump_aul_message(v8::Isolate* isolate, NodeBindings* bindings) {
#ifdef HOST_TIZEN
  // TODO: move the following to m_platform.PumpMessageLoop(isolate)
  if (!g_queue.empty()) {
    auto task = g_queue.pop();
    node::EmitMessage(isolate, task.data.c_str());
    if (task.data == std::string(NESCARGOT_AUL_TERMINATION_MESSAGE)) {
      bindings->TerminateGMainLoop();
    }
  }
#endif
}

NodeBindings::NodeBindings() {}

void NodeBindings::Initialize(Environment&& env, Platform&& platform,
                              Node&& node) {
  assert(platform.PumpMessageLoop);
  assert(platform.EnterIdleMode);

  m_env = std::move(env);
  m_platform = std::move(platform);
  m_node = std::move(node);
  m_isInitialize = true;
}

void NodeBindings::StartEventLoop() {
  assert(m_isInitialize);

  glib::GmainLoopInit(this);

  RunOnce();

  // NOTE: We try an intense memory saving mode called Idle Mode.
  // It will be exited once any javascript operation runs.
  m_platform.EnterIdleMode(m_env.isolate());

  if (HasMoreTasks()) {
    glib::GmainLoopStart();
  }

  glib::GmainLoopExit();
}

bool NodeBindings::HasMoreTasks() {
  return (m_hasMoreNodeTasks && !m_isTerminated);
}

void NodeBindings::RunOnce() {
  auto isolate = m_env.isolate();
  auto event_loop = m_env.event_loop();

  m_platform.PumpMessageLoop(isolate);
  pump_aul_message(isolate, this);

  bool more = uv_run(event_loop, UV_RUN_NOWAIT);

  if (more == false) {
    m_platform.PumpMessageLoop(isolate);
    pump_aul_message(isolate, this);

    m_node.EmitBeforeExit();

    // Emit `beforeExit` if the loop became alive either after emitting
    // event, or after running some callbacks.
    more = uv_loop_alive(event_loop);
    if (uv_run(event_loop, UV_RUN_NOWAIT) != 0) {
      more = true;
    }
  }
  m_hasMoreNodeTasks = more;

  if (m_idleCheckTimeoutID) {
    g_source_remove(m_idleCheckTimeoutID);
  }

#ifndef IDLE_CHECK_TIMEOUT
#define IDLE_CHECK_TIMEOUT 500
#endif
  m_idleCheckTimeoutID = g_timeout_add(
      IDLE_CHECK_TIMEOUT,
      [](gpointer data) -> gboolean {
        NodeBindings* self = (NodeBindings*)data;
        self->m_idleCheckTimeoutID = 0;
        self->m_platform.EnterIdleMode(self->m_env.isolate());
        return G_SOURCE_REMOVE;
      },
      this);
}

}  // namespace nescargot
