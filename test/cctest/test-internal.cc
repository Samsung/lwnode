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

#include "cctest.h"

#include <EscargotPublic.h>
#include "api/handlescope.h"
#include "internal-api.h"

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

class ObjectBasic : public gc {
 public:
  virtual ~ObjectBasic() { LWNODE_CALL_TRACE(); }
  ObjectBasic(Count* pCount) : pCount_(pCount) {
    if (pCount_) {
      pCount_->add();
    }

    Memory::gcRegisterFinalizer(this, [](void* self) {
      auto object = reinterpret_cast<ObjectBasic*>(self);
      auto pCount = object->pCount();
      if (pCount) {
        pCount->sub();
        LWNODE_CALL_TRACE("%d", pCount->get());
      }
    });
  }
  Count* pCount() const { return pCount_; }
  Count* pCount_ = nullptr;
};

class HandleScope : public gc {
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

 private:
  GCVector<ObjectBasic*> handles_;
};

struct Object : public gc {
  Object(Count* pCount = nullptr) : pCount_(pCount) {
    if (pCount_) {
      pCount_->add();
    }

    Memory::gcRegisterFinalizer(this, [](void* self) {
      auto object = reinterpret_cast<Object*>(self);
      auto pCount = object->pCount();
      if (pCount) {
        pCount->sub();
        LWNODE_CALL_TRACE("%d", pCount->get());
      }
    });
  }

  Count* pCount() const { return pCount_; }

  GCVector<Escargot::ValueRef*> eternals_;
  GCVector<HandleScope*> handleScopes_;
  Count* pCount_ = nullptr;
};

// --- Test Start ---

Count s_count;

TEST(internal_GCObject1) {
  s_count.reset();

  []() { Object* object = new Object(&s_count); }();

  CcTest::CollectGarbage();

  CHECK_EQ(s_count.get(), 0);
}

TEST(internal_GCObject2) {
  s_count.reset();

  []() {
    CHECK_EQ(s_count.get(), 0);
    auto scope = new HandleScope();
    scope->add(new ObjectBasic(&s_count));
    scope->add(new ObjectBasic(&s_count));

    CHECK_EQ(s_count.get(), 2);

    // @check remove this
    scope->clear();

    // @check
    // CcTest::CollectGarbage();
    // CHECK_EQ(s_count.get(), 0);
  }();

  CcTest::CollectGarbage();
  CHECK_EQ(s_count.get(), 0);
}

TEST(internal_GCObject3) {
  s_count.reset();

  []() {
    CHECK_EQ(s_count.get(), 0);
    auto object = new Object(&s_count);

    object->handleScopes_.push_back(new HandleScope());
    object->handleScopes_.back()->add(new ObjectBasic(&s_count));

    CHECK_EQ(s_count.get(), 2);

    // @check remove this
    object->handleScopes_.back()->clear();
    object->handleScopes_.pop_back();

    // @check
    // CcTest::CollectGarbage();
    // CHECK_EQ(s_count.get(), 1);
  }();

  CcTest::CollectAllGarbage();
  // @check
  // CcTest::CollectGarbage();
  CHECK_EQ(s_count.get(), 0);
}

TEST(internal_GCVector) {
  s_count.reset();

  []() {
    GCVector<ObjectBasic*> vector;
    vector.push_back(new ObjectBasic(&s_count));
    vector.push_back(new ObjectBasic(&s_count));

    CHECK_EQ(s_count.get(), 2);

    // @check
    // vector.pop_back();
    // CcTest::CollectGarbage();
    // CHECK_EQ(s_count.get(), 0);
  }();

  // @check
  // CcTest::CollectGarbage();
  CcTest::CollectAllGarbage();

  CHECK_EQ(s_count.get(), 0);
}
