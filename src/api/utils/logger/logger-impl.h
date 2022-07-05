/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class LogFormatter {
 public:
  std::string header();
  bool isEnabled() { return isEnabled_; }

 protected:
  virtual void printHeader(std::stringstream& stream) = 0;
  bool isEnabled_{true};
};

class LogTYPED : public LogFormatter {
 public:
  enum class Type {
    RAW,
    INFO,
    WARN,
    ERROR,
  };
  LogTYPED(Type type = Type::RAW) : type_(type) {}
  void printHeader(std::stringstream& stream) override;

 protected:
  Type type_{Type::RAW};
};

class LogINTERNAL : public LogTYPED {
 public:
  LogINTERNAL(Type type = Type::RAW);
};

class LogTRACE : public LogFormatter {
 public:
  LogTRACE(std::string id,
           const char* functionName,
           const char* filename,
           const int line);
  void printHeader(std::stringstream& stream) override;

 private:
  std::string id_;
  std::string functionName_;
};

class Logger {
 public:
  class Output {
   public:
    struct Config {};
    virtual void flush(std::stringstream& ss,
                       std::shared_ptr<Output::Config> config = nullptr) = 0;
    virtual void appendEndOfLine(std::stringstream& ss) = 0;
  };

  Logger() = default;
  Logger(const std::string& header, std::shared_ptr<Output> out = nullptr);
  Logger(LogFormatter&& formatter, std::shared_ptr<Output> out = nullptr);
  ~Logger();

  template <class T>
  Logger& operator<<(const T& msg) {
    stream_ << msg;
    return *this;
  }

  template <typename T, typename... TArgs>
  Logger& log(const T& v, TArgs... args) {
    stream_ << v << " ";
    log(args...);
    return *this;
  }
  template <typename T>
  Logger& log(const T& v) {
    stream_ << v;
    return *this;
  }
  Logger& log() { return *this; }

  template <typename T, typename... Args>
  Logger& print(const char* format, T value, Args... args) {
    if (!isEnabled_) {
      return *this;
    }

    while (*format) {
      if (*format == '%' && *(++format) != '%') {
        stream_ << value;

        // handle sub-specifiers
        if ((*format == 'z')) {
          format++;
        } else if ((*format == 'l') || (*format == 'h')) {
          format++;
          if (*format == *(format + 1)) {
            format++;
          }
        }
        format++;

        print(format, args...);
        return *this;
      }
      stream_ << *format++;
    }
    assert(((void)"logical error: should not come here", false));
    return *this;
  };
  Logger& print(const char* string_without_format_specifiers = "");
  Logger& flush();

 protected:
  std::shared_ptr<Output> out_;
  std::shared_ptr<Output::Config> outConfig_;

 private:
  std::stringstream stream_;
  bool isEnabled_{true};
  void initialize(const std::string& header, std::shared_ptr<Output> out);
};

class StdOut : public Logger::Output {
 public:
  void appendEndOfLine(std::stringstream& ss) override;
  void flush(std::stringstream& ss,
             std::shared_ptr<Output::Config> config = nullptr) override;
};

using OutputInstantiator = std::function<std::shared_ptr<Logger::Output>()>;

class LogOption {
 public:
  static void setDefaultOutputInstantiator(OutputInstantiator fn) {
    s_outputInstantiator_ = fn;
  }

  static OutputInstantiator getOutputInstantiator() {
    return s_outputInstantiator_;
  }

  static std::shared_ptr<Logger::Output> getDefalutOutput() {
    if (s_outputInstantiator_ == nullptr) {
      // Set default output
      s_outputInstantiator_ = []() { return std::make_shared<StdOut>(); };
    }
    return s_outputInstantiator_();
  }

 private:
  static OutputInstantiator s_outputInstantiator_;
};
