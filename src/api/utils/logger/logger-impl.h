#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <sstream>
#include <string>

class LogFormatter {
 public:
  std::string header();

 protected:
  virtual void printHeader(std::stringstream& stream) = 0;
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
    virtual void flush(std::stringstream& ss) = 0;
  };

  Logger(const std::string& header = "", std::shared_ptr<Output> out = nullptr);
  ~Logger();

  template <class T>
  Logger& operator<<(const T& msg) {
    stream_ << msg;
    return *this;
  }

  template <typename T, typename... Args>
  Logger& print(const char* format, T value, Args... args) {
    while (*format) {
      if (*format == '%' && *(++format) != '%') {
        stream_ << value;
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

 private:
  std::shared_ptr<Output> out_;
  std::stringstream stream_;
};

class StdOut : public Logger::Output {
 public:
  void flush(std::stringstream& ss) override;
};
