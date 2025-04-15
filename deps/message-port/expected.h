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

#include <utility>  // for std::move

/*
Expected<T,E>

Usage:

enum class Error { NotFound, PermissionDenied };

Expected<int, Error> DoSomething(bool success) {
  if (success) return 7;
  return Error::NotFound;
}

auto result = DoSomething(false);
if (result) {
  std::cout << "Success: " << *result << std::endl;
} else {
  std::cout << "Error: " << (result.error() == Error::NotFound) << std::endl;
}

Why needed:

Expected<T, E> provides a structured way to handle both success and failure,
unlike std::optional<T> or v8::Maybe<T> which only convey whether a value
exists.

- std::optional: For values with an absence.
- std::expected: For values with potential success or known error types.
- v8::Maybe: For handling potentially missing results within the JS context.
*/

// `T != void`
template <typename T, typename E>
class Expected {
 public:
  Expected(const T& v) : value_(v), has_value_(true) {}
  Expected(T&& v) : value_(std::move(v)), has_value_(true) {}
  Expected(const E& e) : error_(e), has_value_(false) {}
  Expected(E&& e) : error_(std::move(e)), has_value_(false) {}
  ~Expected() {
    if (!has_value_) error_.~E();
  }

  bool valid() const { return has_value_; }
  T& value() { return value_; }
  const T& value() const { return value_; }
  E& error() { return error_; }
  const E& error() const { return error_; }

  operator bool() const { return valid(); }
  T& operator*() { return value(); }
  const T& operator*() const { return value(); }

  T value_or(const T& default_value) {
    return has_value_ ? value_ : default_value;
  }

 private:
  T value_;
  E error_;
  bool has_value_;
};

// `T = void`
template <typename E>
class Expected<void, E> {
 public:
  Expected() : has_value_(true) {}
  Expected(const E& e) : error_(e), has_value_(false) {}
  Expected(E&& e) : error_(std::move(e)), has_value_(false) {}

  bool valid() const { return has_value_; }
  E& error() { return error_; }
  const E& error() const { return error_; }

  operator bool() const { return valid(); }

 private:
  E error_;
  bool has_value_;
};
