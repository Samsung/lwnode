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

#include <GCUtil.h>
#include <EscargotPublic.h>
#include "misc.h"

namespace EscargotShim {

template <typename T>
class GCContainer {
 public:
  GCContainer() {}

  explicit GCContainer(size_t size) {
    if (size) {
      m_buffer = (T*)Escargot::Memory::gcMalloc(sizeof(T) * size);
      m_size = size;
      for (size_t i = 0; i < size; i++) {
        new (&m_buffer[i]) T();
      }
    } else {
      m_buffer = nullptr;
      m_size = 0;
    }
  }

  GCContainer(GCContainer<T>&& other) {
    m_size = other.size();
    m_buffer = other.m_buffer;
    other.m_buffer = nullptr;
    other.m_size = 0;
  }

  GCContainer(const GCContainer<T>& other) = delete;
  const GCContainer<T>& operator=(const GCContainer<T>& other) = delete;

  ~GCContainer() {
    if (m_buffer) {
      for (size_t i = 0; i < m_size; i++) {
        m_buffer[i].~T();
      }
      Escargot::Memory::gcFree(m_buffer);
    }
  }

  size_t size() const { return m_size; }

  T& operator[](const size_t idx) {
    LWNODE_CHECK(m_size > idx);
    return m_buffer[idx];
  }

  const T& operator[](const size_t idx) const {
    LWNODE_CHECK(m_size > idx);
    return m_buffer[idx];
  }

  void clear() {
    if (m_buffer) {
      Escargot::Memory::gcFree(m_buffer);
    }
    m_size = 0;
    m_buffer = nullptr;
  }

  void* operator new(size_t size) { return Escargot::Memory::gcMalloc(size); }
  void* operator new(size_t, void* ptr) { return ptr; }
  void* operator new[](size_t size) = delete;

  void operator delete(void* ptr) { Escargot::Memory::gcFree(ptr); }
  void operator delete[](void* obj) = delete;

 private:
  T* m_buffer = nullptr;
  size_t m_size = 0;
};

}  // namespace EscargotShim
