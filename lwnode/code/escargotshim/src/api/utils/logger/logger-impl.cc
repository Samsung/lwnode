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

#include "logger-impl.h"
#include <iomanip>  // for setfill and setw
#include "color.h"
#include "logger.h"

// --- Formatter ---

#define PADDING(N) std::left << std::setfill(' ') << std::setw(N)
#define DISPLAY_TYPE_LENGTH_LIMIT 5
#define DISPLAY_TRACE_ID_LENGTH_LIMIT 10

std::string LogFormatter::header() {
  std::stringstream stream;
  printHeader(stream);
  return stream.str();
}

void LogTYPED::printHeader(std::stringstream& stream) {
  switch (type_) {
    case Type::INFO:
      stream << PADDING(DISPLAY_TYPE_LENGTH_LIMIT) << "INFO"
             << " ";
      break;
    case Type::WARN:
      stream << CLR_YELLOW << PADDING(DISPLAY_TYPE_LENGTH_LIMIT) << "WARN"
             << " ";
      break;
    case Type::ERROR:
      stream << CLR_RED << PADDING(DISPLAY_TYPE_LENGTH_LIMIT) << "ERROR"
             << " ";
      break;

    default:
      break;
  }
}

LogTRACE::LogTRACE(std::string id,
                   const char* functionName,
                   const char* filename,
                   const int line) {
  id_ = id;
  functionName_ = createCodeLocation(functionName, filename, line);
}

void LogTRACE::printHeader(std::stringstream& stream) {
  stream << CLR_DIM << PADDING(DISPLAY_TYPE_LENGTH_LIMIT) << "TRACE ("
         << PADDING(DISPLAY_TRACE_ID_LENGTH_LIMIT)
         << std::string(id_).substr(0, DISPLAY_TRACE_ID_LENGTH_LIMIT).c_str()
         << ") ";

  stream << IndentCounter::getString(id_) << functionName_ << " " << CLR_RESET;
}

// --- Logger ---
thread_local std::shared_ptr<StdOut> s_loggerOutput;

Logger::Logger(const std::string& header, std::shared_ptr<Output> out)
    : out_(out) {
  if (out_ == nullptr) {
    if (s_loggerOutput == nullptr) {
      s_loggerOutput = std::make_shared<StdOut>();
    }
    out_ = s_loggerOutput;
  }

  stream_ << header;
}

Logger::~Logger() {
  stream_ << CLR_RESET << std::endl;
  out_->flush(stream_);
}

Logger& Logger::print(const char* string_without_format_specifiers) {
  while (*string_without_format_specifiers) {
    if (*string_without_format_specifiers == '%' &&
        *(++string_without_format_specifiers) != '%') {
      assert(((void)"runtime error: invalid format-string", false));
    }
    stream_ << *string_without_format_specifiers++;
  }
  return *this;
}

Logger& Logger::flush() {
  out_->flush(stream_);
  stream_.str("");
  return *this;
}
// --- Output ---

void StdOut::flush(std::stringstream& stream) {
  std::cout << stream.str();
}
