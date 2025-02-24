/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#include <cassert>
#include <iostream>

struct nullopt_t {
  explicit constexpr nullopt_t() {}
};
constexpr nullopt_t nullopt{};

template <typename T>
class Optional {
 public:
  Optional() : has_value_(false) {}
  Optional(const T& value) : value_(value), has_value_(true) {}
  Optional(nullopt_t) : has_value_(false) {}

  void reset() {
    value_ = T();
    has_value_ = false;
  }

  void set_value(const T& value) {
    value_ = value;
    has_value_ = true;
  }

  bool has_value() const { return has_value_; }

  T& value() {
    assert(has_value_ && "No value present");
    return value_;
  }

  const T& value() const {
    assert(has_value_ && "No value present");
    return value_;
  }

  T value_or(T default_value) {
    if (has_value_) {
      return value_;
    }
    return default_value;
  }

  const T value_or(T default_value) const {
    if (has_value_) {
      return value_;
    }
    return default_value;
  }

  // operators

  operator bool() const { return has_value(); }

  bool operator==(const Optional<T>& other) const {
    if (has_value_ != other.hasValue()) {
      return false;
    }
    return has_value_ ? value_ == other.value_ : true;
  }

  bool operator!=(const Optional<T>& other) const {
    return !this->operator==(other);
  }

  bool operator==(const T& other) const {
    if (has_value_) {
      return value() == other;
    }
    return false;
  }

  bool operator!=(const T& other) const { return !operator==(other); }

  T* operator->() {
    assert(has_value());
    return &value_;
  }

  const T* operator->() const {
    assert(has_value());
    return &value_;
  }

 private:
  T value_;
  bool has_value_;
};
