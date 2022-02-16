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

#include <EscargotPublic.h>
#include <algorithm>

#include "api/isolate.h"
#include "base.h"
#include "global-handles.h"

#include "api/utils/gc.h"

namespace v8 {
namespace internal {
void GlobalHandles::Destroy(EscargotShim::ValueWrap* lwValue) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  if (isolate) {
    isolate->global_handles()->destroy(lwValue);
  }
}

void GlobalHandles::MakeWeak(EscargotShim::ValueWrap* lwValue,
                             void* parameter,
                             v8::WeakCallbackInfo<void>::Callback callback) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  isolate->global_handles()->makeWeak(lwValue, parameter, callback);
}

void* GlobalHandles::ClearWeakness(EscargotShim::ValueWrap* lwValue) {
  auto isolate = EscargotShim::IsolateWrap::GetCurrent();
  isolate->global_handles()->clearWeakness(lwValue);
  return nullptr;
}

}  // namespace internal
}  // namespace v8

namespace EscargotShim {

GlobalHandles::GlobalHandles(IsolateWrap* isolate)
    : v8::internal::GlobalHandles(isolate), isolate_(isolate) {}

void GlobalHandles::dispose() {
  LWNODE_CALL_TRACE_ID(GLOBALHANDLES);
  persistentValues_.clear();

  for (auto gcObjectInfo : gcObjectInfos_) {
    delete gcObjectInfo;
  }

  isolate_ = nullptr;
}

void GlobalHandles::create(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter == persistentValues_.end()) {
    persistentValues_.emplace(lwValue, 1);
  } else {
    ++iter->second;
    // TODO:
    LWNODE_CALL_TRACE_ID(GLOBALHANDLES,
                         "Persistent value was created multiple times: %p",
                         lwValue);
  }
}

bool GlobalHandles::destroy(ValueWrap* lwValue) {
  auto iter = persistentValues_.find(lwValue);
  if (iter != persistentValues_.end()) {
    if (iter->second == 1) {
      persistentValues_.erase(iter);
    } else {
      --iter->second;
    }
    return true;
  }
  return false;
}

size_t GlobalHandles::PostGarbageCollectionProcessing(
    /*const v8::GCCallbackFlags gc_callback_flags*/) {
#if defined(LWNODE_ENABLE_EXPERIMENTAL)
  clearWeakValues();
#endif

  return 0;
}

void GlobalHandles::releaseWeakValues() {
  std::set<GcObjectInfo*, ObjectInfoComparator> objectsInfoPersistent;
  std::vector<GcObjectInfo*> objectsInfoWeak;

  for (auto gcObjectInfo : gcObjectInfos_) {
    if (gcObjectInfo->isPersistent()) {
      objectsInfoPersistent.insert(gcObjectInfo);
    } else {
      // TODO: check if the weak pointer is valid
      if (gcObjectInfo->hasCallback()) {
        void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {nullptr,
                                                                   nullptr};
        v8::WeakCallbackInfo<void> info(isolate_->toV8(),
                                        gcObjectInfo->parameter(),
                                        embedderFields,
                                        nullptr);
        gcObjectInfo->runCallback(info);
      }

      // reset callback
      MemoryUtil::gcRegisterFinalizer(
          gcObjectInfo->lwValue(), [](void* self, void* data) {}, nullptr);

      objectsInfoWeak.push_back(gcObjectInfo);

      GC_invoke_finalizers();
    }
  }

  gcObjectInfos_.clear();
  gcObjectInfos_.insert(objectsInfoPersistent.begin(),
                        objectsInfoPersistent.end());
  for (auto objectInfo : objectsInfoWeak) {
    delete objectInfo;
  }
}

bool GlobalHandles::makeWeak(ValueWrap* lwValue,
                             void* parameter,  // c++ native object
                             v8::WeakCallbackInfo<void>::Callback callback) {
  addWeakValue(lwValue, parameter, callback);

  MemoryUtil::gcRegisterFinalizer(
      lwValue,
      [](void* self, void* data) {
        LWNODE_CALL_TRACE_GC_START();
        auto globalHandles = static_cast<GlobalHandles*>(data);

        LWNODE_CALL_TRACE_ID(GLOBALHANDLES,
                             "Call weak callback: %p %p",
                             self,
                             globalHandles->isolate_);
        if (!globalHandles->isolate_) {
          return;
        }

        auto gcObjectInfo = globalHandles->findGcObjectInfo((ValueWrap*)self);
        if (gcObjectInfo) {
          if (gcObjectInfo->hasCallback()) {
            void* embedderFields[v8::kEmbedderFieldsInWeakCallback] = {nullptr,
                                                                       nullptr};
            v8::WeakCallbackInfo<void> info(globalHandles->isolate_->toV8(),
                                            gcObjectInfo->parameter(),
                                            embedderFields,
                                            nullptr);
            LWNODE_CHECK_NOT_NULL(globalHandles->isolate_);
            gcObjectInfo->runCallback(info);
          }

          globalHandles->removeGcObjectInfo(gcObjectInfo->lwValue());
        }

        LWNODE_CALL_TRACE_GC_END();
      },
      isolate_->GetCurrent()->global_handles());

  return true;
}

bool GlobalHandles::clearWeakness(ValueWrap* lwValue) {
  setPersistent(lwValue);
  return true;
}

void GlobalHandles::addWeakValue(
    ValueWrap* lwValue,
    void* parameter,
    v8::WeakCallbackInfo<void>::Callback callback) {
  GcObjectInfo objInfo(lwValue, parameter, callback);
  auto itr = gcObjectInfos_.find(&objInfo);
  if (itr != gcObjectInfos_.end()) {
    (*itr)->unsetPersistent();
    return;
  }

  GcObjectInfo* newObjInfo = new GcObjectInfo(lwValue, parameter, callback);
  gcObjectInfos_.insert(newObjInfo);
}

void GlobalHandles::setPersistent(ValueWrap* lwValue) {
  GcObjectInfo objInfo(lwValue);
  auto itr = gcObjectInfos_.find(&objInfo);
  if (itr != gcObjectInfos_.end()) {
    (*itr)->setPersistent();
    return;
  }

  GcObjectInfo* newObjInfo = new GcObjectInfo(lwValue);
  gcObjectInfos_.insert(newObjInfo);
}

void GlobalHandles::clearWeakValues() {
  std::set<GcObjectInfo*, ObjectInfoComparator> objectsInfoPersistent;
  for (auto objectInfo : gcObjectInfos_) {
    if (objectInfo->isPersistent()) {
      objectsInfoPersistent.insert(objectInfo);
    } else {
      // TODO: check validness of a weak pointer, and call finalizer
    }
  }

  gcObjectInfos_.clear();
  gcObjectInfos_.insert(objectsInfoPersistent.begin(),
                        objectsInfoPersistent.end());
}

GcObjectInfo* GlobalHandles::findGcObjectInfo(ValueWrap* lwValue) {
  GcObjectInfo objInfo(lwValue);
  auto itr = gcObjectInfos_.find(&objInfo);
  if (itr != gcObjectInfos_.end()) {
    return *itr;
  }

  return nullptr;
}

void GlobalHandles::removeGcObjectInfo(ValueWrap* lwValue) {
  GcObjectInfo objInfo(lwValue);
  auto itr = gcObjectInfos_.find(&objInfo);
  if (itr != gcObjectInfos_.end()) {
    auto objectInfo = *itr;
    gcObjectInfos_.erase(itr);
    delete objectInfo;
  }
}

size_t GlobalHandles::handles_count() const {
  size_t count = 0;
  for (auto gcObjectInfo : gcObjectInfos_) {
    if (gcObjectInfo->isPersistent()) {
      count++;
    }
  }

  return count;
}

}  // namespace EscargotShim
