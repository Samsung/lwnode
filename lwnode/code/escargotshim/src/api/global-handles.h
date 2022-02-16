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

#include <set>

#include <EscargotPublic.h>

#include "handle.h"
#include "utils/gc.h"

namespace v8 {
namespace internal {
class GlobalHandles : public gc {
 public:
  GlobalHandles(Isolate* isolate) : isolate_(isolate) {}

  static void MakeWeak(EscargotShim::ValueWrap* lwValue,
                       void* parameter,
                       v8::WeakCallbackInfo<void>::Callback callback);

  // void Create(EscargotShim::ValueWrap* lwValue);
  static void Destroy(EscargotShim::ValueWrap* lwValue);
  static void* ClearWeakness(EscargotShim::ValueWrap* lwValue);

  virtual size_t handles_count() const = 0;

 private:
  Isolate* const isolate_ = nullptr;
};

}  // namespace internal
}  // namespace v8

namespace EscargotShim {

class GlobalWeakHandler;

class GcObjectInfo {
 public:
  GcObjectInfo(ValueWrap* lwValue,
               void* parameter = nullptr,
               v8::WeakCallbackInfo<void>::Callback callback = nullptr)
      : lwValue_(lwValue), parameter_(parameter), callback_(callback) {}

  void setPersistent() {
    holder_.reset(lwValue_);
    isPersistent_ = true;
  }

  void unsetPersistent() {
    holder_.reset(nullptr);
    isPersistent_ = false;
  }

  bool isPersistent() { return isPersistent_; }
  ValueWrap* lwValue() const { return lwValue_; }
  void* parameter() { return parameter_; }
  bool hasCallback() { return callback_ != nullptr; }
  void runCallback(v8::WeakCallbackInfo<void>& info) { callback_(info); }

 private:
  ValueWrap* lwValue_ = nullptr;
  void* parameter_ = nullptr;
  v8::WeakCallbackInfo<void>::Callback callback_ = nullptr;
  Escargot::PersistentRefHolder<ValueWrap> holder_;
  bool isPersistent_ = false;
};

class GlobalHandles final : public v8::internal::GlobalHandles {
 public:
  GlobalHandles(IsolateWrap* isolate);

  size_t PostGarbageCollectionProcessing(
      /*const v8::GCCallbackFlags gc_callback_flags*/);

  void create(ValueWrap* lwValue);
  bool makeWeak(ValueWrap* lwValue,
                void* parameter,
                v8::WeakCallbackInfo<void>::Callback callback);
  bool clearWeakness(ValueWrap* lwValue);
  bool destroy(ValueWrap* lwValue);
  void dispose();

  void releaseWeakValues();
  size_t handles_count() const override;

  void addWeakValue(ValueWrap* lwValue,
                    void* parameter,
                    v8::WeakCallbackInfo<void>::Callback callback);
  void setPersistent(ValueWrap* lwValue);
  void clearWeakValues();
  GcObjectInfo* findGcObjectInfo(ValueWrap* value);
  void removeGcObjectInfo(ValueWrap* lwValue);

 private:
  GCUnorderedMap<ValueWrap*, size_t> persistentValues_;
  IsolateWrap* isolate_{nullptr};
  struct ObjectInfoComparator {
    bool operator()(const GcObjectInfo* a, const GcObjectInfo* b) const {
      return a->lwValue() < b->lwValue();
    }
  };

  // TODO: use std::unique_ptr
  std::set<GcObjectInfo*, ObjectInfoComparator> gcObjectInfos_;
};

}  // namespace EscargotShim
