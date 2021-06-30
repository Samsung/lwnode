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

#include <EscargotPublic.h>
#include <cstdarg>

#include "gc.h"
#include "misc.h"

template <typename T>
class GCContainer : public gc {
 public:
  GCContainer() {}

  explicit GCContainer(const size_t size, ...) {
    if (size) {
      buffer_ = (T*)Escargot::Memory::gcMalloc(sizeof(T) * size);
      size_ = size;
      for (size_t i = 0; i < size; i++) {
        new (&buffer_[i]) T();
      }

      std::va_list args;
      va_start(args, size);
      for (size_t i = 0; i < size; ++i) {
        buffer_[i] = va_arg(args, T);
      }
      va_end(args);
    } else {
      buffer_ = nullptr;
      size_ = 0;
    }
  }

  GCContainer(GCContainer<T>&& other) {
    size_ = other.size();
    buffer_ = other.buffer_;
    other.buffer_ = nullptr;
    other.size_ = 0;
  }

  GCContainer(const GCContainer<T>& other) = delete;
  const GCContainer<T>& operator=(const GCContainer<T>& other) = delete;

  ~GCContainer() {
    LWNODE_CALL_TRACE_ID(GCDEBUG);
    if (buffer_) {
      for (size_t i = 0; i < size_; i++) {
        buffer_[i].~T();
      }
      Escargot::Memory::gcFree(buffer_);
    }
  }

  size_t size() const { return size_; }

  T& operator[](const size_t idx) {
    LWNODE_CHECK(size_ > idx);
    return buffer_[idx];
  }

  const T& operator[](const size_t idx) const {
    LWNODE_CHECK(size_ > idx);
    return buffer_[idx];
  }

  T get(const size_t idx) const {
    LWNODE_CHECK(size_ > idx);
    return buffer_[idx];
  }

  void set(const size_t idx, T val) {
    LWNODE_CHECK(size_ > idx);
    buffer_[idx] = val;
  }

  void remove(const size_t idx) {
    LWNODE_CHECK(size_ > idx);
    buffer_[idx] = nullptr;
  }

  void clear() {
    if (buffer_) {
      Escargot::Memory::gcFree(buffer_);
    }
    size_ = 0;
    buffer_ = nullptr;
  }

  void* operator new(size_t size) { return Escargot::Memory::gcMalloc(size); }
  void* operator new(size_t, void* ptr) { return ptr; }
  void* operator new[](size_t size) = delete;

  void operator delete(void* ptr) { Escargot::Memory::gcFree(ptr); }
  void operator delete[](void* obj) = delete;

 private:
  T* buffer_ = nullptr;
  size_t size_ = 0;
};
