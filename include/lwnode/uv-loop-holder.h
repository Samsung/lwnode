/*
 * Copyright (c) 2025-present Samsung Electronics Co., Ltd
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

#include <uv.h>

class LoopHolderUV {
 public:
  LoopHolderUV(uv_loop_t* loop = nullptr) { Init(loop); }
  ~LoopHolderUV() { Close(); }

  void Init(uv_loop_t* loop) {
    loop_ = loop;
    ref_count_ = 0;
    if (loop_) {
      async_ = new uv_async_t;
      uv_async_init(loop_, async_, [](uv_async_t*) {});
      uv_unref(reinterpret_cast<uv_handle_t*>(async_));
    }
  }

  void Ref() {
    // The refcount inside libuv is not a number, it's a boolean property.
    if (ref_count_++ == 0) {
      uv_ref(reinterpret_cast<uv_handle_t*>(async_));
    }
  }

  void Unref() {
    if (ref_count_ > 0 && --ref_count_ == 0) {
      uv_unref(reinterpret_cast<uv_handle_t*>(async_));
      uv_async_send(async_);
    }
  }

  void Close() {
    if (async_ == nullptr) {
      return;
    }

    if (uv_is_active(reinterpret_cast<uv_handle_t*>(async_))) {
      uv_unref(reinterpret_cast<uv_handle_t*>(async_));
      uv_close(reinterpret_cast<uv_handle_t*>(async_), [](uv_handle_t* handle) {
        delete reinterpret_cast<uv_async_t*>(handle);
      });
    } else {
      delete async_;
    }

    async_ = nullptr;
    ref_count_ = 0;
  }

  int ref_count() const { return ref_count_; }

 private:
  int ref_count_;
  uv_loop_t* loop_ = nullptr;
  uv_async_t* async_ = nullptr;
};
