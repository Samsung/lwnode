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
#include <map>
#include <thread>

#include "api/global.h"
#include "color.h"
#include "flags.h"
#include "logger-util.h"

// --- Formatter ---

#define PADDING(N) std::left << std::setfill(' ') << std::setw(N)
#define DISPLAY_TYPE_LENGTH_LIMIT 5
#define DISPLAY_TRACE_ID_LENGTH_LIMIT 10

std::map<std::thread::id, int> s_thread_ids;

void printThreadIdentifier(std::stringstream& stream) {
  static int s_id = 0;

  auto id = std::this_thread::get_id();
  if (s_thread_ids.find(id) == s_thread_ids.end()) {
    s_thread_ids[id] = ++s_id;
  }

  stream << "[" << s_thread_ids[id] << "] ";
}

std::string LogFormatter::header() {
  std::stringstream stream;

  printThreadIdentifier(stream);
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

LogINTERNAL::LogINTERNAL(Type type) : LogTYPED(type) {
#if defined(NDEBUG)
  isEnabled_ = EscargotShim::Global::flags()->isOn(
      EscargotShim::Flag::Type::InternalLog);
#endif
}

LogTRACE::LogTRACE(std::string id,
                   const char* functionName,
                   const char* filename,
                   const int line) {
  if (!EscargotShim::Global::flags()->isOn(EscargotShim::Flag::Type::TraceCall,
                                           id)) {
    isEnabled_ = false;
    return;
  }

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
static thread_local std::shared_ptr<StdOut> s_loggerOutput;

Logger::Logger(const std::string& header, std::shared_ptr<Output> out)
    : out_(out) {
  initialize(header, out);
}

// TODO: Extract LogFormatter handling.
// TODO: Refactor ouput instance creation with flyweight pattern.
Logger::Logger(LogFormatter&& formatter, std::shared_ptr<Output> out)
    : out_(out) {
  isEnabled_ = formatter.isEnabled();
  if (isEnabled_) {
    initialize(formatter.header(), out_);
  }
  footer_ << CLR_RESET << std::endl;
}

Logger::~Logger() {
  if (!isEnabled_) {
    return;
  }
  if (out_ == nullptr) {
    out_ = LogOption::getDefalutOutput();
  }
  stream_ << footer_.str();
  out_->appendEndOfLine(stream_);
  out_->flush(stream_, outConfig_);
}

void Logger::initialize(const std::string& header,
                        std::shared_ptr<Output> out) {
  out_ = (out == nullptr) ? LogOption::getDefalutOutput() : out;
  stream_ << header;
}

Logger& Logger::print(const char* string_without_format_specifiers) {
  if (!isEnabled_) {
    return *this;
  }

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
  if (out_) {
    out_->appendEndOfLine(stream_);
    out_->flush(stream_, outConfig_);
  }
  stream_.str("");
  return *this;
}
// --- Output ---

void StdOut::flush(std::stringstream& stream,
                   std::shared_ptr<Output::Config> config) {
  fprintf(stderr, "%s", stream.str().c_str());
}

// --- Option ---
OutputInstantiator LogOption::s_outputInstantiator_;
