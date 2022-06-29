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
#ifdef HOST_TIZEN
#include "Extension.h"
#endif

#include "node_bindings.h"

namespace glib {

using namespace LWNode;

struct SourceData {
  GSource source;
  gpointer tag;
  GmainLoopNodeBindings* node_bindings = nullptr;
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

static gboolean GmainLoopDispatchCallback(GSource* source,
                                          GSourceFunc callback,
                                          gpointer user_data) {
  assert(gcontext);
  g_main_context_iteration(gcontext, FALSE);

  if (gmainLoopDone) {
    return G_SOURCE_REMOVE;
  }

  GmainLoopNodeBindings* node_bindings = ((SourceData*)source)->node_bindings;

  node_bindings->RunOnce();

  if (!node_bindings->HasMoreTasks()) {
    g_main_loop_quit(gmainLoop);
    return G_SOURCE_REMOVE;
  }
  return G_SOURCE_CONTINUE;
}

void GmainLoopInit(GmainLoopNodeBindings* self) {
  gcontext = g_main_context_default();
  gmainLoop = g_main_loop_new(gcontext, FALSE);
  source_funcs = {
      .prepare = GmainLoopPrepareCallback,
      .check = GmainLoopCheckCallback,
      .dispatch = GmainLoopDispatchCallback,
  };

  uvsource = g_source_new(&source_funcs, sizeof(SourceData));
  ((SourceData*)uvsource)->tag = g_source_add_unix_fd(
      uvsource,
      uv_backend_fd(uv_default_loop()),
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

// TODO: pump aul message for Tizen AUL application
#if 0
#ifdef HOST_TIZEN
#include <mutex>
#include <thread>
#include "Queue.hpp"

#define NESCARGOT_AUL_TERMINATION_MESSAGE "AUL_TERMINATION"

namespace LWNode {

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
}  // namespace LWNode
#endif

namespace LWNode {

// static void pump_aul_message(v8::Isolate* isolate, GmainLoopNodeBindings* bindings) {
// #ifdef HOST_TIZEN
//   // TODO: move the following to m_platform.PumpMessageLoop(isolate)
//   if (!g_queue.empty()) {
//     auto task = g_queue.pop();
//     node::EmitMessage(isolate, task.data.c_str());
//     if (task.data == std::string(NESCARGOT_AUL_TERMINATION_MESSAGE)) {
//       bindings->TerminateGMainLoop();
//     }
//   }
// #endif
// }
}
#endif

namespace LWNode {

static thread_local bool g_enableGmainLoop = false;

GmainLoopNodeBindings::GmainLoopNodeBindings(GmainLoopWork* gmainLoopWork)
    : gmainLoopWork_(gmainLoopWork) {
  m_isInitialize = true;
}

void GmainLoopNodeBindings::enable() {
  g_enableGmainLoop = true;
}

bool GmainLoopNodeBindings::isEnabled() {
  return g_enableGmainLoop;
}

void GmainLoopNodeBindings::StartEventLoop() {
  assert(m_isInitialize);
  assert(g_enableGmainLoop);

  glib::GmainLoopInit(this);

  RunOnce();

  if (HasMoreTasks()) {
    glib::GmainLoopStart();
  }

  glib::GmainLoopExit();
}

bool GmainLoopNodeBindings::HasMoreTasks() {
  return (m_hasMoreNodeTasks && !m_isTerminated);
}

void GmainLoopNodeBindings::RunOnce() {
  m_hasMoreNodeTasks = gmainLoopWork_->RunOnce();
}

}  // namespace LWNode
