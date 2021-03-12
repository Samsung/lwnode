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

#if !defined(NDEBUG)

#include "cctest.h"

#include <EscargotPublic.h>
#include "api/handlescope.h"
#include "internal-api.h"

#include "api/utils/gc-container.h"

using namespace Escargot;

class Count {
 public:
  ~Count() {
    if (count_ != 0) {
      LWNODE_DLOG_ERROR("Count is: %d", count_);
    }
  }
  void add() { ++count_; }
  void sub() { --count_; }
  void reset() { count_ = 0; }
  int get() { return count_; }

 private:
  int count_ = 0;
};

struct ObjectBasic : public gc {};

struct HandleScope : public gc {
 public:
  void add(ObjectBasic* value) {
    LWNODE_CALL_TRACE("%p", value);
    handles_.push_back(value);
  }

  bool remove(ObjectBasic* value) {
    auto it = std::find(handles_.begin(), handles_.end(), value);
    if (it != handles_.end()) {
      handles_.erase(it);
      return true;
    }
    return false;
  }

  void clear() {
    LWNODE_CALL_TRACE();

    for (auto it = handles_.begin(); it != handles_.end(); it++) {
      LWNODE_DLOG_INFO("%p", *it);
    }

    handles_.clear();
  }

  ObjectBasic* get(size_t idx) {
    LWNODE_CHECK(idx < handles_.size());
    return handles_[idx];
  }

 private:
  GCVector<ObjectBasic*, true> handles_;
};

struct Object : public gc {
  GCVector<HandleScope*, true> handleScopes_;
};

// --- Test Start ---

static GCTracer g_tracer;

TEST(internal_GCObject1) {
  g_tracer.reset();

  []() {
    Object* object = new Object();
    g_tracer.add(object);
  }();

  CcTest::CollectGarbage();

  CHECK_EQ(g_tracer.getAllocatedCount(), 0);
}

TEST(internal_GCObject2) {
  g_tracer.reset();

  []() {
    auto scope = new HandleScope();

    scope->add(new ObjectBasic());
    scope->add(new ObjectBasic());

    g_tracer.add(scope->get(0));
    g_tracer.add(scope->get(1));

    CHECK_EQ(g_tracer.getAllocatedCount(), 2);
  }();

  CcTest::CollectGarbage();
  CHECK_EQ(g_tracer.getAllocatedCount(), 0);
}

TEST(internal_GCObject3) {
  g_tracer.reset();

  []() {
    auto object = new Object();

    object->handleScopes_.push_back(new HandleScope());
    object->handleScopes_.push_back(new HandleScope());

    object->handleScopes_.back()->add(new ObjectBasic());

    g_tracer.add(object->handleScopes_.back());
    g_tracer.add(object->handleScopes_.back()->get(0));

    CHECK_EQ(g_tracer.getAllocatedCount(), 2);

    if (EscargotShim::Flags::isTraceGCEnabled()) {
      g_tracer.printState();
    }
  }();

  CcTest::CollectGarbage();

  if (EscargotShim::Flags::isTraceGCEnabled()) {
    g_tracer.printState();
  }

  CHECK_LE(g_tracer.getAllocatedCount(), 1);
}

TEST(internal_GCObject4) {
  g_tracer.reset();

  Escargot::PersistentRefHolder<Object> holder;

  [&holder]() {
    auto object = new Object();
    holder.reset(object);

    object->handleScopes_.push_back(new HandleScope());
    object->handleScopes_.push_back(new HandleScope());

    object->handleScopes_.back()->add(new ObjectBasic());

    g_tracer.add(object, "object");
    g_tracer.add(object->handleScopes_.back(), "scope");
    g_tracer.add(object->handleScopes_.back()->get(0), "baseobject");

    CHECK_EQ(g_tracer.getAllocatedCount(), 3);

    object->handleScopes_.pop_back();
  }();

  CcTest::CollectGarbage();

  if (EscargotShim::Flags::isTraceGCEnabled()) {
    g_tracer.printState();
  }

  CHECK_EQ(g_tracer.getAllocatedCount(), 1);

  holder.release();

  CcTest::CollectGarbage();

  CHECK_LE(g_tracer.getAllocatedCount(), 1);
}

TEST(internal_GCContainer) {
  g_tracer.reset();

  Escargot::PersistentRefHolder<GCContainer<ObjectBasic*>> holder;

  [&holder]() {
    auto obj = new ObjectBasic();
    GCContainer<ObjectBasic*>* vector = new GCContainer<ObjectBasic*>(1, obj);

    holder.reset(vector);

    g_tracer.add(vector, "vector");
    g_tracer.add(obj, "obj");

    CcTest::CollectGarbage();

    CHECK_EQ(g_tracer.getAllocatedCount(), 2);
  }();

  holder.release();

  CcTest::CollectGarbage();

  CHECK_LE(g_tracer.getAllocatedCount(), 1);
}

#endif
