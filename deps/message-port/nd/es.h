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

#include <EscargotPublic.h>

#define ES_LIKELY(x) __builtin_expect((x), 1)
#define ES_UNLIKELY(x) __builtin_expect((x), 0)
#define ES_ASSERT(expr)                                                        \
  ((expr) ? (void)0 : Abort(__FILE__ ":assertion failed: " #expr))

using AbortHandler = void (*)(const char* message);
void Abort(const char* message);
void SetAbortHandler(AbortHandler handler);

namespace Escargot {

struct CompileResult
    : public Escargot::ScriptParserRef::InitializeScriptResult {
  CompileResult(const Escargot::ScriptParserRef::InitializeScriptResult& result)
      : Escargot::ScriptParserRef::InitializeScriptResult(result) {}
  CompileResult(
      const Escargot::ScriptParserRef::InitializeScriptResult&& result)
      : Escargot::ScriptParserRef::InitializeScriptResult(std::move(result)) {}

  CompileResult& check() {
    ES_ASSERT(isSuccessful());
    return *this;
  }
};

struct ExecResult : public Escargot::Evaluator::EvaluatorResult {
  ExecResult(const Escargot::Evaluator::EvaluatorResult& src)
      : Escargot::Evaluator::EvaluatorResult(src) {}
  ExecResult(Escargot::Evaluator::EvaluatorResult&& src)
      : Escargot::Evaluator::EvaluatorResult(std::move(src)) {}

  ExecResult& check() {
    ES_ASSERT(isSuccessful());
    return *this;
  }

  Escargot::ValueRef* checkedValue() {
    check();
    return result;
  }

  operator bool() const { return isSuccessful(); }

  Escargot::ValueRef* returned_value() { return result; }
  Escargot::ValueRef* returned_value_or(Escargot::ValueRef* default_value) {
    return isSuccessful() ? result : default_value;
  }

  // Note: The following are declarations and must be implemented to be used.
  void setPendingException(Escargot::ContextRef* context);
  void reportPendingException(Escargot::ContextRef* context);
};

}  // namespace Escargot

class StringView {
 public:
  // Note: This constructors create a StringView that references the data of the
  // input string, similar to how std::string_view (C++17) works. If the input
  // string is destroyed or modified, the data_ pointer in StringView will
  // become invalid.
  StringView(const char* str) : data_(str), size_(strnlen(str, kMaxLength)) {}
  StringView(const char* str, size_t len) : data_(str), size_(len) {}
  StringView(const std::string& str) : data_(str.data()), size_(str.size()) {}
  StringView() = delete;

  const char* data() const { return data_; }
  const char* c_str() const { return data_; }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

 private:
  const char* data_;
  size_t size_;

  static constexpr int kMaxLength =
      sizeof(void*) == 4 ? (1 << 28) - 16 : (1 << 29) - 24;
};

inline Escargot::StringRef* OneByteString(const StringView literal) {
  return Escargot::StringRef::createFromASCII(literal.data(), literal.size());
}

inline Escargot::StringRef* UTF8String(const StringView literal) {
  return Escargot::StringRef::createFromUTF8(literal.data(), literal.size());
}
