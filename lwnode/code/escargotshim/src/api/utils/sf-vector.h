/*
 * Copyright (c) 2017-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#pragma once

#include <math.h>  // log21
#include <cassert>
#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Starfish {

template <typename T>
struct VectorAllocInfo {
 public:
  VectorAllocInfo(T* buffer, size_t capacity)
      : m_buffer(buffer), m_capacity(capacity) {}

  T* m_buffer;
  size_t m_capacity;
};

template <typename T, typename Allocator, bool isEraseStrategyStrict = false>
class Vector {
 protected:
  typedef typename Allocator::template rebind<T>::other TAllocType;

  // Types:
 public:
  typedef typename std::char_traits<T> traits_type;
  typedef typename TAllocType::size_type size_type;
  typedef typename TAllocType::difference_type difference_type;
  typedef typename TAllocType::pointer pointer;
  typedef typename TAllocType::const_pointer const_pointer;

  typedef T* iterator;
  typedef const T* const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  Vector() : m_buffer(nullptr), m_size(0), m_capacity(0) { makeEmpty(); }

  Vector(Vector<T, Allocator>&& other)
      : m_buffer(other.m_buffer),
        m_size(other.m_size),
        m_capacity(other.m_capacity) {
    other.m_buffer = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  Vector(const Vector<T, Allocator>& other)
      : m_buffer(nullptr), m_size(0), m_capacity(0) {
    if (other.m_size > 0) {
      construct(other);
    } else {
      makeEmpty();
    }
  }

  template <typename _InputIterator>
  Vector(_InputIterator start, _InputIterator end)
      : m_buffer(nullptr), m_size(0), m_capacity(0) {
    construct(start, end);
  }

  Vector<T, Allocator>& operator=(const Vector<T, Allocator>& other) {
    if (other.m_size > 0) {
      deallocate();
      construct(other);
    } else {
      clear();
    }
    return *this;
  }

  ~Vector() { deallocate(); }

  void push_back(const T& val) { pushBack(val); }

  void push_back(T&& val) { pushBack(val); }

  template <class... Args>
  void emplace_back(Args&&... args) {
    pushBack(T(args...));
  }

  void emplace_back(T&& val) { pushBack(val); }

  void insert(size_t pos, const T& val) {
    assert(pos <= m_size);
    insertImpl(pos, val);
  }

  template <class _Iterator>
  void insert(_Iterator pos, const T& val) {
    size_t npos = std::distance(begin(), pos);
    insertImpl(npos, val);
  }

  template <class _Iterator, class _InputIterator>
  void insert(_Iterator pos, _InputIterator first, _InputIterator last) {
    size_t npos = std::distance(begin(), pos);
    size_t l = std::distance(first, last);
    for (size_t i = 0; i < l; i++) {
      insertImpl(i + npos, *first);
      first++;
    }
  }

  template <class _InputIterator>
  void assign(_InputIterator first, _InputIterator last) {
    clear();
    insert(begin(), first, last);
  }

  template <class _Iterator>
  _Iterator erase(_Iterator first, _Iterator last) {
    size_t start = first - _Iterator(data());
    size_t end = last - _Iterator(data());

    erase(start, end);

    return _Iterator(this->m_buffer + start);
  }

  void erase(size_t pos) { erase(pos, pos + 1); }

  void erase(size_t start, size_t end) {
    if (start == end) return;
    assert(start >= 0);
    assert(end <= m_size);

    size_t howMuch = end - start;
    size_t newLen = m_size - howMuch;

    if (howMuch == 0) {
      return;
    } else if (newLen == 0) {
      clear();
    } else {
      if (isEraseStrategyStrict) {
        eraseStrictImpl(start, howMuch);
      } else {
        eraseImpl(start, howMuch);
      }
    }
  }

  template <class _Iterator>
  _Iterator erase(_Iterator pos) {
    return erase(pos, pos + 1);
  }

  void erase(reverse_iterator pos) {
    size_t start = size() - std::distance(rbegin(), pos) - 1;
    size_t end = start + 1;
    erase(start, end);
  }

  void erase(reverse_iterator pos, reverse_iterator pos2) {
    size_t start = size() - std::distance(rbegin(), pos) - 1;
    size_t end = size() - std::distance(rbegin(), pos2) - 1;
    erase(start, end);
  }

  size_t size() const { return m_size; }

  size_t capacity() const { return m_capacity; }

  void shrink_to_fit() {}

  iterator begin() { return iterator(data()); }

  const_iterator cbegin() { return const_iterator(data()); }

  const_iterator begin() const { return const_iterator(data()); }

  T& front() { return *begin(); }

  const T& front() const { return *begin(); }

  iterator end() { return iterator(data() + m_size); }

  const_iterator cend() { return const_iterator(data() + m_size); }

  const_iterator end() const { return const_iterator(data() + m_size); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }

  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  T& back() { return *iterator(data() + m_size - 1); }

  const T& back() const { return *const_iterator(data() + m_size - 1); }

  bool empty() const { return m_size == 0; }

  void pop_back() { erase(m_size - 1); }

  T& operator[](const size_t& idx) {
    assert(idx < m_size);
    return m_buffer[idx];
  }

  const T& operator[](const size_t& idx) const {
    assert(idx < m_size);
    return m_buffer[idx];
  }

  T& at(const size_t& idx) {
    assert(idx < m_size);
    return m_buffer[idx];
  }

  const T& at(const size_t& idx) const {
    assert(idx < m_size);
    return m_buffer[idx];
  }

  void clear() { makeEmpty(); }

  T* data() const { return m_buffer; }

  void swap(Vector<T, Allocator>& other) {
    std::swap(m_buffer, other.m_buffer);
    std::swap(m_capacity, other.m_capacity);
    std::swap(m_size, other.m_size);
  }

  bool operator==(const Vector<T, Allocator>& other) {
    if (m_size != other.m_size) {
      return false;
    }
    if (other.empty()) {
      return true;
    }
    return std::equal(begin(), end(), other.begin());
  }

  bool operator!=(const Vector<T, Allocator>& other) {
    return !(this == other);
  }

  void resize(size_t newSize) {
    reserve(newSize);
    for (size_t i = m_size; i < newSize; i++) {
      new (&m_buffer[i]) T;
    }
    setLen(newSize);
  }

  void reserve(size_t newSize) {
    if (newSize <= m_capacity) {
      return;
    }

    size_t oldSize = m_size;
    VectorAllocInfo<T> allocInfo = allocate(newSize);
    for (size_t i = 0; i < m_size; i++) {
      new (&allocInfo.m_buffer[i]) T(std::move(m_buffer[i]));
    }
    resetBuffer(allocInfo);
    setLen(oldSize);
  }

 protected:
  void makeEmpty() { deallocate(); }

  size_t toCapacity(size_t size) const {
    if (size == 0) {
      return 1;
    }
    size_t base = log2l(size);
    size_t capacity = 1 << (base + 1);
    return capacity;
  }

  VectorAllocInfo<T> allocate(size_t size) const {
    size_t capacity = toCapacity(size);
    T* ret = Allocator().allocate(capacity);
    return VectorAllocInfo<T>(ret, capacity);
  }

  void resetBuffer(const VectorAllocInfo<T>& info) {
    deallocate();
    m_buffer = info.m_buffer;
    m_capacity = info.m_capacity;
  }

  template <typename _InputIterator>
  void construct(_InputIterator start, _InputIterator end) {
    typedef typename std::is_integral<_InputIterator> _Integral;
    _construct(start, end, _Integral());
  }

  template <class _InIterator>
  void _construct(_InIterator start, _InIterator end, std::false_type) {
    typedef typename std::iterator_traits<_InIterator>::iterator_category _Tag;
    return _construct(start, end, _Tag());
  }

  template <class _InIterator>
  void _construct(_InIterator start,
                  _InIterator end,
                  std::forward_iterator_tag) {
    const size_t distance = static_cast<size_t>(std::distance(start, end));
    VectorAllocInfo<T> allocInfo = allocate(distance);
    resetBuffer(allocInfo);
    size_t len = 0;
    for (; len < distance; len++) {
      new (&m_buffer[len]) T(*(start + len));
    }
    setLen(len);
  }

  void construct(const Vector<T, Allocator>& other) {
    construct(other.begin(), other.end());
  }

  void eraseStrictImpl(size_t start, size_t sizeToErase) {
    // @note this drops unused memory immediately

    size_t end = start + sizeToErase;
    assert(start < end);
    assert(start >= 0);
    assert(end <= m_size);

    size_t c = end - start;
    size_t newLen = m_size - c;

    if (newLen > 0) {
      VectorAllocInfo<T> newBuffer = allocate(newLen);
      for (size_t i = 0; i < start; i++) {
        new (&newBuffer.m_buffer[i]) T(m_buffer[i]);
      }

      for (size_t i = end; i < m_size; i++) {
        new (&newBuffer.m_buffer[i - c]) T(m_buffer[i]);
      }

      for (size_t i = 0; i < m_size; i++) {
        m_buffer[i].~T();
      }

      m_buffer = newBuffer.m_buffer;
      m_capacity = newBuffer.m_capacity;
      m_size = newLen;
    } else {
      clear();
    }
  }

  void eraseImpl(size_t start, size_t sizeToErase) {
    // @note this drops unused memory when newLen < (m_capacity / 2)

    size_t end = start + sizeToErase;
    assert(start < end);
    assert(start >= 0);
    assert(end <= m_size);

    size_t c = end - start;
    size_t newLen = m_size - c;
    if (newLen == 0) {
      clear();
    } else if (newLen < (m_capacity / 2)) {
      VectorAllocInfo<T> newBuffer = allocate(newLen);
      for (size_t i = 0; i < start; i++) {
        new (&newBuffer.m_buffer[i]) T(m_buffer[i]);
      }

      for (size_t i = end; i < m_size; i++) {
        new (&newBuffer.m_buffer[i - c]) T(m_buffer[i]);
      }

      for (size_t i = 0; i < m_size; i++) {
        m_buffer[i].~T();
      }

      m_buffer = newBuffer.m_buffer;
      m_capacity = newBuffer.m_capacity;
      m_size = newLen;
    } else {
      for (size_t i = 0; i < sizeToErase; i++) {
        size_t idx = i + start;
        size_t nextIdx = i + start + sizeToErase;
        if (nextIdx < m_size) {
          m_buffer[idx] = std::move(m_buffer[nextIdx]);
          m_buffer[nextIdx].~T();
        } else {
          m_buffer[idx].~T();
        }
      }

      for (size_t i = end; i < m_size; i++) {
        size_t idx = i;
        size_t nextIdx = i + sizeToErase;
        if (nextIdx < m_size) {
          m_buffer[idx] = std::move(m_buffer[nextIdx]);
          m_buffer[nextIdx].~T();
        } else {
          m_buffer[idx].~T();
        }
      }

      m_size = newLen;
    }
  }

  void insertImpl(size_t pos, const T& v) {
    assert(pos <= m_size);

    size_t newLen = m_size + 1;

    if (capacity() < newLen) {
      reserve(newLen);
    }

    for (size_t i = m_size; i > pos; i--) {
      m_buffer[i] = m_buffer[i - 1];
    }

    new (&m_buffer[pos]) T(v);
    setLen(newLen);
  }

  void setLen(size_t newLen) {
    for (size_t i = newLen; i < m_size; i++) {
      m_buffer[i].~T();
    }
    m_size = newLen;
  }

  void pushBack(const T& val) {
    size_t newLen = m_size + 1;
    reserve(newLen);
    new (&m_buffer[newLen - 1]) T(val);
    setLen(newLen);
  }

  void pushBack(T&& val) {
    size_t newLen = m_size + 1;
    reserve(newLen);
    new (&m_buffer[newLen - 1]) T(std::move(val));
    setLen(newLen);
  }

  // Important! `m_size` update should follow `deallocate()`
  void deallocate() {
    for (size_t i = 0; i < m_size; i++) {
      m_buffer[i].~T();
    }
    if (m_buffer) {
      Allocator().deallocate(m_buffer, m_capacity);
    }
    m_buffer = nullptr;
    m_size = 0;
    m_capacity = 0;
  }

  T* m_buffer;
  size_t m_size;
  size_t m_capacity;
};

}  // namespace Starfish
