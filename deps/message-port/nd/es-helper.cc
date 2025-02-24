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

#include "es-helper.h"
#include <unistd.h>  // dprintf
#include <iomanip>   // setw
#include <iostream>
#include <sstream>
#include <string>
#include "nd-debug.h"   // CHECK
#include "nd-logger.h"  // LOG_ERROR

#if __cplusplus >= 201703L
#include <optional>
#define Optional std::optional
#else
#include "utils/optional.h"
#endif

namespace Escargot {

static bool StringEndsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

ValueRef* BuiltinPrint(ExecutionStateRef* state,
                       ValueRef* this_value,
                       size_t argc,
                       ValueRef** argv,
                       bool is_construct_call) {
  if (argc >= 1) {
    StringRef* print_msg;
    if (argv[0]->isSymbol()) {
      print_msg = argv[0]->asSymbol()->symbolDescriptiveString();
      puts(print_msg->toStdUTF8String().data());
      state->context()->printDebugger(print_msg);
    } else {
      print_msg = argv[0]->toString(state);
      puts(print_msg->toStdUTF8String().data());
      state->context()->printDebugger(print_msg->toString(state));
    }
  } else {
    puts("undefined");
  }
  return ValueRef::createUndefined();
}

OptionalRef<StringRef> BuiltinHelperFileRead(
    OptionalRef<ExecutionStateRef> state,
    const char* file_name,
    const char* builtin_name,
    bool should_throw_on_error) {
  FILE* fp = fopen(file_name, "r");
  if (fp) {
    StringRef* src = StringRef::emptyString();
    std::string utf8_str;
    std::basic_string<unsigned char, std::char_traits<unsigned char>> str;
    char buf[512];
    bool has_non_latin1_content = false;
    size_t read_len;
    while ((read_len = fread(buf, 1, sizeof buf, fp))) {
      if (!has_non_latin1_content) {
        for (size_t i = 0; i < read_len; i++) {
          unsigned char ch = buf[i];
          if (ch & 0x80) {
            // check non-latin1 character
            has_non_latin1_content = true;
            fseek(fp, 0, SEEK_SET);
            break;
          }
          str += ch;
        }
      } else {
        utf8_str.append(buf, read_len);
      }
    }
    fclose(fp);

    if (StringRef::isCompressibleStringEnabled()) {
      if (state) {
        if (has_non_latin1_content) {
          src = StringRef::createFromUTF8ToCompressibleString(
              state->context()->vmInstance(),
              utf8_str.data(),
              utf8_str.length(),
              false);
        } else {
          src = StringRef::createFromLatin1ToCompressibleString(
              state->context()->vmInstance(), str.data(), str.length());
        }
      } else {
        if (has_non_latin1_content) {
          src = StringRef::createFromUTF8(
              utf8_str.data(), utf8_str.length(), false);
        } else {
          src = StringRef::createFromLatin1(str.data(), str.length());
        }
      }
    } else {
      if (has_non_latin1_content) {
        src = StringRef::createFromUTF8(
            utf8_str.data(), utf8_str.length(), false);
      } else {
        src = StringRef::createFromLatin1(str.data(), str.length());
      }
    }
    return src;
  } else {
    if (should_throw_on_error && state) {
      const size_t max_name_length = 980;
      if ((strnlen(builtin_name, max_name_length) +
           strnlen(file_name, max_name_length)) < max_name_length) {
        char msg[1024];
        snprintf(msg,
                 sizeof(msg),
                 "GlobalObject.%s: cannot open file %s",
                 builtin_name,
                 file_name);
        state->throwException(URIErrorObjectRef::create(
            state.get(),
            StringRef::createFromUTF8(msg, strnlen(msg, sizeof msg))));
      } else {
        state->throwException(URIErrorObjectRef::create(
            state.get(), StringRef::createFromASCII("invalid file name")));
      }
    } else {
      TRACE(ESCARGOT, "File Not Found:", file_name);
    }
    return nullptr;
  }
}

ValueRef* BuiltinLoad(ExecutionStateRef* state,
                      ValueRef* this_value,
                      size_t argc,
                      ValueRef** argv,
                      bool is_construct_call) {
  // parameter: (path[, use_clean_context: bool][, use_module: bool])

  if (argc > 0) {
    // id
    std::string file_name, parent_url;

    bool is_module = true;
    bool use_empty_context = false;

    // path
    CHECK(argv[0]->isString());
    file_name = argv[0]->toString(state)->toStdUTF8String();
    TRACE(LOADER, KV(file_name));

    // useEmptyContext
    if (argc > 1) {
      CHECK(argv[1]->isBoolean());
      use_empty_context = argv[1]->asBoolean();
    }

    // isModule
    if (argc > 2) {
      CHECK(argv[2]->isBoolean());
      is_module = argv[2]->asBoolean();
    }

    ContextRef* context =
        use_empty_context
            ? ContextRef::create(state->context()->vmInstance()).get()
            : state->context();

    OptionalRef<StringRef> maybe_source =
        BuiltinHelperFileRead(state, file_name.c_str(), "load");
    CHECK(maybe_source.hasValue());

    ExecResult result = CompileAndExecution(state->context(),
                                            maybe_source.value(),
                                            argv[0]->toString(state),
                                            is_module);
    if (!result) {
      TRACE(ERROR, "[SetPendingException]");
      result.setPendingException(state->context());
      result.reportPendingException(state->context());
      state->throwException(result.error.value());
    }
    return result.checkedValue();
  } else {
    return ValueRef::createUndefined();
  }
}

ValueRef* BuiltinRead(ExecutionStateRef* state,
                      ValueRef* this_value,
                      size_t argc,
                      ValueRef** argv,
                      bool is_construct_call) {
  if (argc >= 1) {
    auto f = argv[0]->toString(state)->toStdUTF8String();
    const char* file_name = f.data();
    StringRef* src = BuiltinHelperFileRead(state, file_name, "read").value();
    return src;
  } else {
    return StringRef::emptyString();
  }
}

ValueRef* BuiltinRun(ExecutionStateRef* state,
                     ValueRef* this_value,
                     size_t argc,
                     ValueRef** argv,
                     bool is_construct_call) {
  if (argc >= 1) {
    double start_time = DateObjectRef::currentTime();

    auto f = argv[0]->toString(state)->toStdUTF8String();
    const char* file_name = f.data();
    StringRef* src = BuiltinHelperFileRead(state, file_name, "run").value();
    bool is_module = StringEndsWith(f, "mjs");
    auto script =
        state->context()
            ->scriptParser()
            ->initializeScript(src, argv[0]->toString(state), is_module)
            .fetchScriptThrowsExceptionIfParseError(state);
    script->execute(state);
    return ValueRef::create(DateObjectRef::currentTime() - start_time);
  } else {
    return ValueRef::create(0);
  }
}

ValueRef* BuiltinGc(ExecutionStateRef* state,
                    ValueRef* this_value,
                    size_t argc,
                    ValueRef** argv,
                    bool is_construct_call) {
  Memory::gc();
  return ValueRef::createUndefined();
}

bool EvalScript(ContextRef* context,
                StringRef* source,
                StringRef* src_name,
                bool should_print_script_result,
                bool is_module) {
  if (StringEndsWith(src_name->toStdUTF8String(), "mjs")) {
    is_module = is_module || true;
  }

  ScriptParserRef::InitializeScriptResult script_initialize_result =
      context->scriptParser()->initializeScript(source, src_name, is_module);
  if (!script_initialize_result.script) {
    fprintf(stderr, "Script parsing error: ");
    switch (script_initialize_result.parseErrorCode) {
      case Escargot::ErrorObjectRef::Code::SyntaxError:
        fprintf(stderr, "SyntaxError");
        break;
      case Escargot::ErrorObjectRef::Code::EvalError:
        fprintf(stderr, "EvalError");
        break;
      case Escargot::ErrorObjectRef::Code::RangeError:
        fprintf(stderr, "RangeError");
        break;
      case Escargot::ErrorObjectRef::Code::ReferenceError:
        fprintf(stderr, "ReferenceError");
        break;
      case Escargot::ErrorObjectRef::Code::TypeError:
        fprintf(stderr, "TypeError");
        break;
      case Escargot::ErrorObjectRef::Code::URIError:
        fprintf(stderr, "URIError");
        break;
      default:
        break;
    }
    fprintf(
        stderr,
        ": %s\n",
        script_initialize_result.parseErrorMessage->toStdUTF8String().data());
    return false;
  }

  auto eval_result = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      script_initialize_result.script.get());

  if (!eval_result.isSuccessful()) {
    fprintf(
        stderr,
        "Uncaught %s:\n",
        eval_result.resultOrErrorToString(context)->toStdUTF8String().data());
    for (size_t i = 0; i < eval_result.stackTrace.size(); i++) {
      fprintf(stderr,
              "%s (%d:%d)\n",
              eval_result.stackTrace[i].srcName->toStdUTF8String().data(),
              (int)eval_result.stackTrace[i].loc.line,
              (int)eval_result.stackTrace[i].loc.column);
    }
    return false;
  }

  if (should_print_script_result) {
    puts(eval_result.resultOrErrorToString(context)->toStdUTF8String().data());
  }

  bool result = true;
  while (context->vmInstance()->hasPendingJob() ||
         context->vmInstance()->hasPendingJobFromAnotherThread()) {
    if (context->vmInstance()->waitEventFromAnotherThread(10)) {
      context->vmInstance()->executePendingJobFromAnotherThread();
    }
    if (context->vmInstance()->hasPendingJob()) {
      auto job_result = context->vmInstance()->executePendingJob();
      if (should_print_script_result || job_result.error) {
        if (job_result.error) {
          fprintf(stderr,
                  "Uncaught %s:\n",
                  job_result.resultOrErrorToString(context)
                      ->toStdUTF8String()
                      .data());
          result = false;
        } else {
          fprintf(stderr,
                  "%s\n",
                  job_result.resultOrErrorToString(context)
                      ->toStdUTF8String()
                      .data());
        }
      }
    }
  }
  return result;
}

void AttachBuiltinPrint(ContextRef* context, ObjectRef* target) {
  static auto builtinPrint = [](ExecutionStateRef* state,
                                ValueRef* thisValue,
                                size_t argc,
                                ValueRef** argv,
                                bool isConstructCall) -> ValueRef* {
    if (argc > 0) {
      std::stringstream ss;

      for (size_t i = 0; i < argc; ++i) {
        if (argv[i]->isSymbol()) {
          ss << argv[i]
                    ->asSymbol()
                    ->symbolDescriptiveString()
                    ->toStdUTF8String();
        } else {
          ss << argv[i]
                    ->toStringWithoutException(state->context())
                    ->toStdUTF8String();
        }
        ss << " ";
      }

      std::cout << ss.str() << std::endl;
    }
    return ValueRef::createUndefined();
  };

  static auto builtinPrintFd = [](ExecutionStateRef* state,
                                  ValueRef* thisValue,
                                  size_t argc,
                                  ValueRef** argv,
                                  bool isConstructCall) -> ValueRef* {
    if (argc > 1 && argv[0]->isNumber()) {
      int32_t fd = argv[0]->asInt32();
      std::string msg = argv[1]
                            ->toStringWithoutException(state->context())
                            ->toStdUTF8String();
      dprintf(fd, "%s", msg.c_str());
    }
    return ValueRef::createUndefined();
  };

  static auto builtinPrintAddress = [](ExecutionStateRef* state,
                                       ValueRef* thisValue,
                                       size_t argc,
                                       ValueRef** argv,
                                       bool isConstructCall) -> ValueRef* {
    if (argc > 0) {
      std::stringstream ss;

      for (size_t i = 0; i < argc; ++i) {
        if (argv[i]->isString()) {
          ss << argv[i]
                    ->toStringWithoutException(state->context())
                    ->toStdUTF8String();
        } else {
          ss << "(" << argv[i] << ")";
        }
        ss << " ";
      }

      std::cout << ss.str() << std::endl;
    }
    return ValueRef::createUndefined();
  };

  static auto builtinPrintCallStack = [](ExecutionStateRef* state,
                                         ValueRef* thisValue,
                                         size_t argc,
                                         ValueRef** argv,
                                         bool isConstructCall) -> ValueRef* {
    size_t maxStackSize = 5;

    if (argc == 1 && argv[0]->isUInt32()) {
      maxStackSize = argv[0]->toUint32(state);
    }

    std::cout << ExecResultHelper::GetCallStackString(
                     state->computeStackTrace(), maxStackSize)
              << std::endl;

    return ValueRef::createUndefined();
  };

  Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ObjectRef* target) -> ValueRef* {
        auto esPrint =
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrint,
                                          1,
                                          true,
                                          false));

        esPrint->defineDataProperty(
            state,
            StringRef::createFromASCII("fd"),
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrintFd,
                                          1,
                                          true,
                                          false)),
            true,
            true,
            true);

        esPrint->defineDataProperty(
            state,
            StringRef::createFromASCII("ptr"),
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrintAddress,
                                          1,
                                          true,
                                          false)),
            true,
            true,
            true);

        esPrint->defineDataProperty(
            state,
            StringRef::createFromASCII("stack"),
            FunctionObjectRef::create(state,
                                      FunctionObjectRef::NativeFunctionInfo(
                                          AtomicStringRef::emptyAtomicString(),
                                          builtinPrintCallStack,
                                          1,
                                          true,
                                          false)),
            true,
            true,
            true);

        target->defineDataProperty(state,
                                   StringRef::createFromASCII("print"),
                                   esPrint,
                                   true,
                                   true,
                                   true);

        return ValueRef::createUndefined();
      },
      target);
}

ExecResult CompileAndExecution(ContextRef* context,
                               StringRef* source,
                               StringRef* source_name,
                               bool isModule) {
  ScriptParserRef::InitializeScriptResult compile_result =
      context->scriptParser()->initializeScript(source, source_name, isModule);

  if (!compile_result.isSuccessful()) {
    LOG_ERROR("Script parsing error: ");
    switch (compile_result.parseErrorCode) {
      case Escargot::ErrorObjectRef::Code::SyntaxError:
        LOG_ERROR("SyntaxError");
        break;
      case Escargot::ErrorObjectRef::Code::EvalError:
        LOG_ERROR("EvalError");
        break;
      case Escargot::ErrorObjectRef::Code::RangeError:
        LOG_ERROR("RangeError");
        break;
      case Escargot::ErrorObjectRef::Code::ReferenceError:
        LOG_ERROR("ReferenceError");
        break;
      case Escargot::ErrorObjectRef::Code::TypeError:
        LOG_ERROR("TypeError");
        break;
      case Escargot::ErrorObjectRef::Code::URIError:
        LOG_ERROR("URIError");
        break;
      default:
        break;
    }

    TRACEF(ESCARGOT,
           "Compile: %s",
           compile_result.parseErrorMessage->toStdUTF8String());

    Evaluator::EvaluatorResult result;
    result.error =
        ExceptionHelper::CreateErrorObject(context,
                                           compile_result.parseErrorCode,
                                           compile_result.parseErrorMessage);

    TRACEF(ESCARGOT,
           "Compile: %s",
           result.resultOrErrorToString(context)->toStdUTF8String().c_str());
    return result;
  }

  auto execute_result = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        ValueRef* value = script->execute(state);
        return script->isModule() ? script->moduleNamespace(state) : value;
      },
      compile_result.script.get());
  if (execute_result.isSuccessful() == false) {
    TRACEF(ESCARGOT,
           "\nExecute:\n%s",
           ExecResultHelper::GetErrorString(context, execute_result).c_str());
  }
  return execute_result;
}

// --- ExecResultHelper ---
bool ExecResultHelper::showInternalCode = true;
ExecResultHelper::CallStackFilter ExecResultHelper::callStackFilter;
std::function<std::string()> ExecResultHelper::internalSourcePatternGetter;

void ExecResultHelper::SetCallStackFilter(CallStackFilter filter) {
  callStackFilter = filter;
}

void ExecResultHelper::ShowInternalCode(bool visible) {
  showInternalCode = visible;
}

std::string ExecResultHelper::GetInternalSourcePattern() {
  if (!internalSourcePatternGetter) {
    return "";
  }
  return internalSourcePatternGetter();
}

std::string ExecResultHelper::GetErrorString(
    ContextRef* context, const Evaluator::EvaluatorResult& result) {
  const auto& minimal_reason =
      result.resultOrErrorToString(context)->toStdUTF8String();
  const std::string detailed_reason =
      GetStackTraceString(result.stackTrace, minimal_reason);
  return detailed_reason;
}

std::string ExecResultHelper::GetStackTraceString(
    const GCManagedVector<Evaluator::StackTraceData>& traceData,
    const std::string& reasonString,
    size_t max_stack_size) {
  TRACE(ERROR, KV(traceData.size()));
  const std::string separator = "  ";
  std::ostringstream oss;

  size_t maxPrintStackSize =
      std::min((int)max_stack_size, (int)traceData.size());

  Optional<size_t> nonInternalTraceFound;
  Optional<size_t> validTraceFound;
  const std::string internalSourcePattern = GetInternalSourcePattern();

  for (size_t i = 0; i < maxPrintStackSize; ++i) {
    const auto& sourcePath = traceData[i].srcName->toStdUTF8String();
    const int errorLine = traceData[i].loc.line;
    const int errorColumn = traceData[i].loc.column;
    if (errorLine == -1 || errorColumn == -1) {
      // Skip Native JavaScript
      continue;
    }
    if (!validTraceFound) {
      validTraceFound = i;
    }
    if (sourcePath.find(internalSourcePattern) == std::string::npos) {
      // Found the last non-internal code.
      nonInternalTraceFound = i;
      break;
    }
  }

  const size_t locationIdx = showInternalCode
                                 ? validTraceFound.value_or(0)
                                 : nonInternalTraceFound.value_or(0);

  if (traceData.size()) {
    // Format the last traceData in detail
    const auto& lastTraceData = traceData[locationIdx];
    const auto& codePath = lastTraceData.srcName->toStdUTF8String();
    const auto& codeString = lastTraceData.sourceCode->toStdUTF8String();
    const int errorLine = lastTraceData.loc.line;
    const int errorColumn = lastTraceData.loc.column;
    const int marginLine = 5;

    oss << "Module: " << std::endl;
    oss << separator << (codePath == "" ? "(empty name)" : codePath)
        << std::endl;
    oss << "Reason: " << std::endl;
    oss << separator << "(" << errorLine << ":" << errorColumn << ") "
        << reasonString << std::endl;

    oss << "Source: " << std::endl;
    std::stringstream sstream(codeString);
    int startLine = std::max(0, errorLine - marginLine);
    int endLine = errorLine + marginLine;
    int curLine = 1;
    for (std::string line; std::getline(sstream, line); ++curLine) {
      if (startLine <= curLine) {
        oss << separator << "L" << curLine << ": " << line;
        if (curLine == errorLine) {
          oss << "\t<--";
        }
        oss << std::endl;

        if (endLine <= curLine) {
          break;
        }
      }
    }

    // Format call stacks
    oss << ExecResultHelper::GetCallStackString(traceData, max_stack_size);
  } else {
    oss << reasonString << " (No trace data)" << std::endl;
  }

  return oss.str();
}

static std::string getCodeLine(const std::string& codeString, int errorLine) {
  if (errorLine < 1 || codeString.empty()) {
    return "";
  }

  std::stringstream sstream(codeString);
  std::string result;
  int curLine = 1;

  for (std::string line; std::getline(sstream, line); ++curLine) {
    if (curLine == errorLine) {
      result = line;
      break;
    }
  }
  return result;
}

std::string ExecResultHelper::GetCallStackString(
    const GCManagedVector<Evaluator::StackTraceData>& traceData,
    size_t maxStackSize) {
  std::ostringstream oss;
  const std::string separator = "  ";
  size_t maxPrintStackSize = std::min((int)maxStackSize, (int)traceData.size());

  oss << "Call Stack:" << std::endl;
  for (size_t i = 0, idx = 0; i < maxPrintStackSize; ++i) {
    const auto& iter = traceData[i];
    auto codePath = iter.srcName->toStdUTF8String();
    auto codeString = iter.sourceCode->toStdUTF8String();
    const int errorLine = iter.loc.line;
    const int errorColumn = iter.loc.column;

    auto sourceOnStack = getCodeLine(codeString, errorLine);

    // Trim left spaces
    auto pos = sourceOnStack.find_first_not_of(' ');
    auto errorCodeLine =
        sourceOnStack.substr(pos != std::string::npos ? pos : 0);

    // Call filter
    if (callStackFilter &&
        callStackFilter(codePath, errorCodeLine, errorLine, errorColumn)) {
      if (!errorCodeLine.empty()) {
        oss << separator << std::setw(2) << ++idx << ": " << errorCodeLine
            << std::endl;
      }
      continue;
    }

    oss << separator << std::setw(2) << ++idx << ": "
        << (errorCodeLine == "" ? "?" : errorCodeLine) << " ";
    oss << "(" << (codePath == "" ? "?" : codePath) << ":" << errorLine << ":"
        << errorColumn << ")" << std::endl;
  }

  return oss.str();
}

// --- ExecResult ---
void ExecResult::setPendingException(Escargot::ContextRef* context) {
  ExceptionHelper::SetPendingException(
      context,
      error.value(),
      Escargot::ExecResultHelper::GetErrorString(context, *this));
}

void ExecResult::reportPendingException(Escargot::ContextRef* context) {
  ExceptionHelper::ReportPendingException(context);
}

// --- TryCatchScope ---
thread_local TryCatchScope* TryCatchScope::current_scope;

void TryCatchScope::ReportPendingException(Escargot::ContextRef* context) {
  ExceptionHelper::SetPendingException(context, exception_, reason_);
  ExceptionHelper::ReportPendingException(context);
}

// --- ExceptionHelper ---
ExceptionHelper::ExceptionSetter ExceptionHelper::pending_exception_setter_;
ExceptionHelper::ExceptionReporter ExceptionHelper::exception_reporter_;

ErrorObjectRef* ExceptionHelper::CreateErrorObject(ContextRef* context,
                                                   ErrorObjectRef::Code code,
                                                   StringRef* errorMessage) {
  ExecResult r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ErrorObjectRef::Code code,
         StringRef* errorMessage) -> ValueRef* {
        auto errorObject = ErrorObjectRef::create(state, code, errorMessage);
        ExceptionHelper::AddDetailsToErrorObject(state, errorObject);
        return errorObject;
      },
      code,
      errorMessage);
  return r.check().result->asErrorObject();
}

void ExceptionHelper::AddDetailsToErrorObject(ExecutionStateRef* state,
                                              ErrorObjectRef* error) {
  const std::string minimal_reason =
      error->toStringWithoutException(state->context())->toStdUTF8String();

  // Note: Calls to computeStackTrace() are expensive, and for errors that are
  // resolved by try-catch, we don't need to compute stack trace. It's a good
  // idea to write a detailed reason at exit time, if possible.
  GCManagedVector<Evaluator::StackTraceData> stackTrace =
      state->computeStackTrace();

  TRACE(ERROR, "SetExtraData");
  StringRef* detailed_reason = UTF8String(
      ExecResultHelper::GetStackTraceString(stackTrace, minimal_reason));
  error->setExtraData(detailed_reason);

  // TODO: add `stack` property in Error link the following:
  // error->defineNativeDataAccessorProperty(
  //     state,
  //     StringRef::createFromUTF8("stack"),
  //     new NativeAccessorProperty(
  //         true, false, true, StackTraceGetter, StackTraceSetter,
  //         stackTrace));
}

std::string ExceptionHelper::GetErrorReason(ExecutionStateRef* state,
                                            Escargot::ValueRef* maybe_error) {
  if (maybe_error->isErrorObject()) {
    ErrorObjectRef* error = maybe_error->asErrorObject();
    if (error->extraData()) {
      auto data = static_cast<Escargot::StringRef*>(error->extraData());
      TRACE(ERROR, "ExtraData Found");
      return data->toStdUTF8String();
    }
    TRACE(ERROR, "No ExtraData");
  }
  return maybe_error->toStringWithoutException(state->context())
      ->toStdUTF8String();
}

// --- StringHelper ---
bool StringHelper::IsAsciiString(StringRef* string) {
  auto bufferData = string->stringBufferAccessData();

  if (!bufferData.has8BitContent) {
    return false;
  }

  bool isAscii = true;
  for (size_t i = 0; i < bufferData.length; i++) {
    char16_t c = bufferData.charAt(i);
    if (c > 127) {  // including all 7 bit code
      isAscii = false;
      break;
    }
  }

  return isAscii;
}

void SetMethod(ExecutionStateRef* state,
               ObjectRef* target,
               StringRef* name,
               FunctionObjectRef::NativeFunctionPointer native_function) {
  target->defineDataProperty(
      state,
      name,
      FunctionObjectRef::create(state,
                                FunctionObjectRef::NativeFunctionInfo(
                                    AtomicStringRef::emptyAtomicString(),
                                    native_function,
                                    0,
                                    true,
                                    false)),
      true,
      true,
      true);
}

void SetMethod(ExecutionStateRef* state,
               ObjectRef* target,
               const char* name,
               FunctionObjectRef::NativeFunctionPointer native_function) {
  SetMethod(state, target, OneByteString(name), native_function);
}

void SetMethod(ContextRef* context,
               ObjectRef* target,
               const char* name,
               FunctionObjectRef::NativeFunctionPointer native_function) {
  Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* target,
         StringRef* name,
         FunctionObjectRef::NativeFunctionPointer native_function)
          -> ValueRef* {
        SetMethod(state, target, name, native_function);
        return ValueRef::createUndefined();
      },
      target,
      OneByteString(name),
      native_function);
}

void SetMethod(ObjectTemplateRef* target,
               const char* name,
               size_t argument_count,
               FunctionTemplateRef::NativeFunctionPointer fn) {
  target->set(OneByteString(name),
              // TemplateRef*
              FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                          argument_count,
                                          false,  // isStrict
                                          false,  // isConstructor
                                          fn),
              false,  // isWritable
              true,   // isEnumerable
              false   // isConfigurable
  );
}

void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 StringRef* name,
                 ValueRef* value,
                 bool isWritable,
                 bool isEnumerable,
                 bool isConfigurable) {
  bool result = target->defineDataProperty(
      state, name, value, isWritable, isEnumerable, isConfigurable);
  CHECK(result);
}

void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 const char* name,
                 ValueRef* value,
                 bool isWritable,
                 bool isEnumerable,
                 bool isConfigurable) {
  SetProperty(state,
              target,
              OneByteString(name),
              value,
              isWritable,
              isEnumerable,
              isConfigurable);
}

struct NativeAccessorProperty
    : public ObjectRef::NativeDataAccessorPropertyData {
 public:
  NativeAccessorProperty(bool isWritable,
                         bool isEnumerable,
                         bool isConfigurable,
                         ObjectRef::NativeDataAccessorPropertyGetter getter,
                         ObjectRef::NativeDataAccessorPropertySetter setter)
      : NativeDataAccessorPropertyData(
            isWritable, isEnumerable, isConfigurable, getter, setter) {}

  void* operator new(size_t size) { return Memory::gcMalloc(size); }
};

struct NativeAccessorPropertyWithData : public NativeAccessorProperty {
 public:
  NativeAccessorPropertyWithData(
      bool isWritable,
      bool isEnumerable,
      bool isConfigurable,
      ObjectRef::NativeDataAccessorPropertyGetter getter,
      ObjectRef::NativeDataAccessorPropertySetter setter,
      void* data)
      : NativeAccessorProperty(
            isWritable, isEnumerable, isConfigurable, getter, setter),
        data_(data) {}

  void* operator new(size_t size) { return Memory::gcMalloc(size); }

  void* data_;
};

void SetProperty(ExecutionStateRef* state,
                 ObjectRef* target,
                 const char* name,
                 ObjectRef::NativeDataAccessorPropertyGetter getter,
                 ObjectRef::NativeDataAccessorPropertySetter setter,
                 bool isWritable,
                 bool isEnumerable,
                 bool isConfigurable,
                 void* data) {
  bool result = target->defineNativeDataAccessorProperty(
      state,
      OneByteString(name),
      data != nullptr
          ? new NativeAccessorPropertyWithData(
                isWritable, isEnumerable, isConfigurable, getter, setter, data)
          : new NativeAccessorProperty(
                isWritable, isEnumerable, isConfigurable, getter, setter),
      false);
  CHECK(result);
}

static ValueRef* EmptySetter(ExecutionStateRef* state,
                             ValueRef* this_value,
                             size_t argc,
                             ValueRef** argv,
                             OptionalRef<ObjectRef> new_target) {
  // We may throw an exception by default.
  // state->throwException(ErrorObjectRef::create(
  //     state, ErrorObjectRef::Code::None, OneByteString("")));
  return ValueRef::createUndefined();
};

void SetProperty(ObjectTemplateRef* target,
                 const char* name,
                 FunctionTemplateRef::NativeFunctionPointer getter,
                 FunctionTemplateRef::NativeFunctionPointer setter,
                 bool isEnumerable,
                 bool isConfigurable) {
  target->setAccessorProperty(
      OneByteString(name),
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                  0,      // argument_count
                                  false,  // strict
                                  false,  // constructor
                                  getter),
      FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                  0,      // argument_count
                                  false,  // strict
                                  false,  // constructor
                                  setter ?: EmptySetter),
      isEnumerable,
      isConfigurable);
}

bool GetProperty(ContextRef* context,
                 ObjectRef* target,
                 ValueRef* key,
                 ValueRef*& out) {
  auto r = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* target,
         ValueRef* key) -> ValueRef* { return target->get(state, key); },
      target,
      key);

  if (r.error.hasValue()) {
    return false;
  }

  out = r.result;
  return true;
}

FunctionTemplateRef* NewFunctionTemplate(
    size_t argument_count,
    FunctionTemplateRef::NativeFunctionPointer fn,
    bool is_constructor) {
  // Create a default constructor as static
  static auto default_constructor =
      [](ExecutionStateRef* state,
         ValueRef* this_value,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> new_target) -> ValueRef* {
    return new_target.hasValue() ? this_value : ValueRef::createUndefined();
  };

  // Create a FunctionTemplateRef
  return FunctionTemplateRef::create(AtomicStringRef::emptyAtomicString(),
                                     argument_count,
                                     false,  // isStrict
                                     is_constructor,
                                     fn ? fn : default_constructor);
}

Maybe<ValueRef*> CallFunction(ContextRef* context,
                              ValueRef* receiver,
                              FunctionObjectRef* callback,
                              const size_t argc,
                              ValueRef** argv) {
  CHECK(context != nullptr);
  CHECK(callback != nullptr);

#ifdef USE_CONVENTIONAL_EVAL
  ExecResult result = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ValueRef* receiver,
         FunctionObjectRef* callback,
         const size_t argc,
         ValueRef** argv) -> ValueRef* {
        return callback->call(state, receiver, argc, argv);
      },
      receiver,
      callback,
      argc,
      argv);
#else
  // This is an experimental way to the above usage which code bloat may occur.

  ExecResult result = Eval::execute(context, [&](ExecutionStateRef* state) {
    return callback->call(state, receiver, argc, argv);
  });
#endif

  if (!result) {
    return Nothing<ValueRef*>();
  }
  return Just(result.returned_value());
}

Maybe<ValueRef*> CallFunction(ContextRef* context,
                              ObjectRef* receiver,
                              StringRef* functionName,
                              const size_t argc,
                              ValueRef** argv) {
#if USE_CONVENTIONAL_EVAL
  ExecResult result = Evaluator::execute(
      context,
      [](ExecutionStateRef* state,
         ObjectRef* receiver,
         StringRef* functionName,
         const size_t argc,
         ValueRef** argv) -> ValueRef* {
        ValueRef* value = receiver->get(state, functionName);
        CHECK_MSG(
            value->isFunctionObject(), "%s", functionName->toStdUTF8String());
        return value->asFunctionObject()->call(state, receiver, argc, argv);
      },
      receiver,
      functionName,
      argc,
      argv);
#else
  // This is an experimental way to the above usage which code bloat may occur.

  ExecResult result = Eval::execute(context, [&](ExecutionStateRef* state) {
    ValueRef* value = receiver->get(state, functionName);
    if (!value->isFunctionObject()) {
      state->throwException(
          OneByteString(functionName->toStdUTF8String() + "Error"));
    }
    return value->asFunctionObject()->call(state, receiver, argc, argv);
  });
#endif

  if (!result) {
    return Nothing<ValueRef*>();
  }

  return Just(result.returned_value());
}

}  // namespace Escargot
