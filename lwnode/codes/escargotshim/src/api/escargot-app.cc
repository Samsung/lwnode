/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
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

#include "escargot-app.h"

using namespace Escargot;

namespace EscargotShim {

static const char32_t offsetsFromUTF8[6] = {
    0x00000000UL,
    0x00003080UL,
    0x000E2080UL,
    0x03C82080UL,
    static_cast<char32_t>(0xFA082080UL),
    static_cast<char32_t>(0x82082080UL)};

static char32_t readUTF8Sequence(const char*& sequence,
                                 bool& valid,
                                 int& charlen) {
  unsigned length;
  const char sch = *sequence;
  valid = true;
  if ((sch & 0x80) == 0)
    length = 1;
  else {
    unsigned char ch2 = static_cast<unsigned char>(*(sequence + 1));
    if ((sch & 0xE0) == 0xC0 && (ch2 & 0xC0) == 0x80)
      length = 2;
    else {
      unsigned char ch3 = static_cast<unsigned char>(*(sequence + 2));
      if ((sch & 0xF0) == 0xE0 && (ch2 & 0xC0) == 0x80 && (ch3 & 0xC0) == 0x80)
        length = 3;
      else {
        unsigned char ch4 = static_cast<unsigned char>(*(sequence + 3));
        if ((sch & 0xF8) == 0xF0 && (ch2 & 0xC0) == 0x80 &&
            (ch3 & 0xC0) == 0x80 && (ch4 & 0xC0) == 0x80)
          length = 4;
        else {
          valid = false;
          sequence++;
          return -1;
        }
      }
    }
  }

  charlen = length;
  char32_t ch = 0;
  switch (length) {
    case 4:
      ch += static_cast<unsigned char>(*sequence++);
      ch <<= 6;  // Fall through.
    case 3:
      ch += static_cast<unsigned char>(*sequence++);
      ch <<= 6;  // Fall through.
    case 2:
      ch += static_cast<unsigned char>(*sequence++);
      ch <<= 6;  // Fall through.
    case 1:
      ch += static_cast<unsigned char>(*sequence++);
  }
  return ch - offsetsFromUTF8[length - 1];
}

static bool stringEndsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static std::string dirnameOf(const std::string& fname) {
  size_t pos = fname.find_last_of("/");
  if (std::string::npos == pos) {
    pos = fname.find_last_of("\\/");
  }
  return (std::string::npos == pos) ? "" : fname.substr(0, pos);
}

static std::string absolutePath(const std::string& referrerPath,
                                const std::string& src) {
  std::string utf8MayRelativePath = dirnameOf(referrerPath) + "/" + src;
  auto absPath = realpath(utf8MayRelativePath.data(), nullptr);
  if (!absPath) {
    return std::string();
  }
  std::string utf8AbsolutePath = absPath ? absPath : "";
  free(absPath);

  return utf8AbsolutePath;
}

static std::string absolutePath(const std::string& src) {
  auto absPath = realpath(src.data(), nullptr);
  std::string utf8AbsolutePath = absPath;
  free(absPath);

  return utf8AbsolutePath;
}

static OptionalRef<StringRef> builtinHelperFileRead(
    OptionalRef<ExecutionStateRef> state,
    const char* fileName,
    const char* builtinName) {
  FILE* fp = fopen(fileName, "r");
  if (fp) {
    StringRef* src = StringRef::emptyString();
    std::string utf8Str;
    std::basic_string<unsigned char, std::char_traits<unsigned char>> str;
    char buf[512];
    bool hasNonLatin1Content = false;
    size_t readLen;
    while ((readLen = fread(buf, 1, sizeof buf, fp))) {
      if (!hasNonLatin1Content) {
        const char* source = buf;
        int charlen;
        bool valid;
        while (source < buf + readLen) {
          char32_t ch = readUTF8Sequence(source, valid, charlen);
          if (ch > 255) {
            hasNonLatin1Content = true;
            fseek(fp, 0, SEEK_SET);
            break;
          } else {
            str += (unsigned char)ch;
          }
        }
      } else {
        utf8Str.append(buf, readLen);
      }
    }
    fclose(fp);
    if (StringRef::isCompressibleStringEnabled()) {
      if (state) {
        if (hasNonLatin1Content) {
          src = StringRef::createFromUTF8ToCompressibleString(
              state->context()->vmInstance(), utf8Str.data(), utf8Str.length());
        } else {
          src = StringRef::createFromLatin1ToCompressibleString(
              state->context()->vmInstance(), str.data(), str.length());
        }
      } else {
        if (hasNonLatin1Content) {
          src = StringRef::createFromUTF8(utf8Str.data(), utf8Str.length());
        } else {
          src = StringRef::createFromLatin1(str.data(), str.length());
        }
      }
    } else {
      if (hasNonLatin1Content) {
        src = StringRef::createFromUTF8(utf8Str.data(), utf8Str.length());
      } else {
        src = StringRef::createFromLatin1(str.data(), str.length());
      }
    }
    return src;
  } else {
    char msg[1024];
    snprintf(msg,
             sizeof(msg),
             "GlobalObject.%s: cannot open file %s",
             builtinName,
             fileName);
    if (state) {
      state->throwException(URIErrorObjectRef::create(
          state.get(),
          StringRef::createFromUTF8(msg, strnlen(msg, sizeof msg))));
    } else {
      puts(msg);
    }
    return nullptr;
  }
}

static ValueRef* builtinPrint(ExecutionStateRef* state,
                              ValueRef* thisValue,
                              size_t argc,
                              ValueRef** argv,
                              bool isConstructCall) {
  if (argc >= 1) {
    if (argv[0]->isSymbol()) {
      puts(argv[0]
               ->asSymbol()
               ->symbolDescriptiveString()
               ->toStdUTF8String()
               .data());
    } else {
      puts(argv[0]->toString(state)->toStdUTF8String().data());
    }
  } else {
    puts("undefined");
  }
  return ValueRef::createUndefined();
}

static ValueRef* builtinLoad(ExecutionStateRef* state,
                             ValueRef* thisValue,
                             size_t argc,
                             ValueRef** argv,
                             bool isConstructCall) {
  if (argc >= 1) {
    auto f = argv[0]->toString(state)->toStdUTF8String();
    const char* fileName = f.data();
    StringRef* src = builtinHelperFileRead(state, fileName, "load").value();
    bool isModule = stringEndsWith(f, "mjs");

    auto script =
        state->context()
            ->scriptParser()
            ->initializeScript(src, argv[0]->toString(state), isModule)
            .fetchScriptThrowsExceptionIfParseError(state);
    return script->execute(state);
  } else {
    return ValueRef::createUndefined();
  }
}

static ValueRef* builtinRead(ExecutionStateRef* state,
                             ValueRef* thisValue,
                             size_t argc,
                             ValueRef** argv,
                             bool isConstructCall) {
  if (argc >= 1) {
    auto f = argv[0]->toString(state)->toStdUTF8String();
    const char* fileName = f.data();
    StringRef* src = builtinHelperFileRead(state, fileName, "read").value();
    return src;
  } else {
    return StringRef::emptyString();
  }
}

static ValueRef* builtinRun(ExecutionStateRef* state,
                            ValueRef* thisValue,
                            size_t argc,
                            ValueRef** argv,
                            bool isConstructCall) {
  if (argc >= 1) {
    double startTime = DateObjectRef::currentTime();

    auto f = argv[0]->toString(state)->toStdUTF8String();
    const char* fileName = f.data();
    StringRef* src = builtinHelperFileRead(state, fileName, "run").value();
    bool isModule = stringEndsWith(f, "mjs");
    auto script =
        state->context()
            ->scriptParser()
            ->initializeScript(src, argv[0]->toString(state), isModule)
            .fetchScriptThrowsExceptionIfParseError(state);
    script->execute(state);
    return ValueRef::create(DateObjectRef::currentTime() - startTime);
  } else {
    return ValueRef::create(0);
  }
}

static ValueRef* builtinGc(ExecutionStateRef* state,
                           ValueRef* thisValue,
                           size_t argc,
                           ValueRef** argv,
                           bool isConstructCall) {
  Memory::gc();
  return ValueRef::createUndefined();
}

void Platform::didPromiseJobEnqueued(ContextRef* relatedContext,
                                     PromiseObjectRef* obj) {
  // ignore. we always check pending job after eval script
  printf("didPromiseJobEnqueued\n");
}

void* Platform::onArrayBufferObjectDataBufferMalloc(ContextRef* whereObjectMade,
                                                    ArrayBufferObjectRef* obj,
                                                    size_t sizeInByte) {
  return calloc(sizeInByte, 1);
}

void Platform::onArrayBufferObjectDataBufferFree(ContextRef* whereObjectMade,
                                                 ArrayBufferObjectRef* obj,
                                                 void* buffer) {
  return free(buffer);
}

PlatformRef::LoadModuleResult Platform::onLoadModule(
    ContextRef* relatedContext,
    ScriptRef* whereRequestFrom,
    StringRef* moduleSrc) {
  std::string referrerPath = whereRequestFrom->src()->toStdUTF8String();

  for (size_t i = 0; i < loadedModules.size(); i++) {
    if (std::get<2>(loadedModules[i]) == whereRequestFrom) {
      referrerPath = std::get<0>(loadedModules[i]);
      break;
    }
  }

  std::string absPath =
      absolutePath(referrerPath, moduleSrc->toStdUTF8String());
  if (absPath.length() == 0) {
    std::string s = "Error reading : " + moduleSrc->toStdUTF8String();
    return LoadModuleResult(ErrorObjectRef::Code::None,
                            StringRef::createFromUTF8(s.data(), s.length()));
  }

  for (size_t i = 0; i < loadedModules.size(); i++) {
    if (std::get<0>(loadedModules[i]) == absPath &&
        std::get<1>(loadedModules[i]) == relatedContext) {
      return LoadModuleResult(std::get<2>(loadedModules[i]));
    }
  }

  OptionalRef<StringRef> source =
      builtinHelperFileRead(nullptr, absPath.data(), "");
  if (!source) {
    std::string s = "Error reading : " + absPath;
    return LoadModuleResult(ErrorObjectRef::Code::None,
                            StringRef::createFromUTF8(s.data(), s.length()));
  }

  auto parseResult = relatedContext->scriptParser()->initializeScript(
      source.value(), moduleSrc, true);
  if (!parseResult.isSuccessful()) {
    return LoadModuleResult(parseResult.parseErrorCode,
                            parseResult.parseErrorMessage);
  }

  loadedModules.push_back(std::make_tuple(
      absPath,
      relatedContext,
      PersistentRefHolder<ScriptRef>(parseResult.script.get())));
  return LoadModuleResult(parseResult.script.get());
}

void Platform::didLoadModule(ContextRef* relatedContext,
                             OptionalRef<ScriptRef> referrer,
                             ScriptRef* loadedModule) {
  std::string path;
  if (referrer) {
    path = absolutePath(referrer->src()->toStdUTF8String(),
                        loadedModule->src()->toStdUTF8String());
  } else {
    path = absolutePath(loadedModule->src()->toStdUTF8String());
  }
  loadedModules.push_back(std::make_tuple(
      path, relatedContext, PersistentRefHolder<ScriptRef>(loadedModule)));
}

void Platform::hostImportModuleDynamically(ContextRef* relatedContext,
                                           ScriptRef* referrer,
                                           StringRef* src,
                                           PromiseObjectRef* promise) {}

App::App() {}

App::~App() {}

void App::initialize() {
#ifndef NDEBUG
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
#endif

#ifdef M_MMAP_THRESHOLD
  mallopt(M_MMAP_THRESHOLD, 2048);
#endif
#ifdef M_MMAP_MAX
  mallopt(M_MMAP_MAX, 1024 * 1024);
#endif
  Globals::initialize();
  Memory::setGCFrequency(24);

  // Create a platform
  Platform* platform = new Platform();

  // Create a new VMInstance and make a Context.
  _instance = VMInstanceRef::create(platform);
  _instance->setOnVMInstanceDelete(
      [](VMInstanceRef* instance) { delete instance->platform(); });

  // _context = App::createContext(_instance.get());

  _isInitialized = true;
}

void App::deinitialize() {
  // Dispose the instance and tear down Escargot.
  _context.release();

  _instance.release();
  Globals::finalize();
}

PersistentRefHolder<ContextRef> App::createContext() {
  return ContextRef::create(_instance);
}

bool App::initializeGlobal(ContextRef* context) {
  Evaluator::execute(context, [](ExecutionStateRef* state) -> ValueRef* {
    ContextRef* context = state->context();
    {
      FunctionObjectRef::NativeFunctionInfo nativeFunctionInfo(
          AtomicStringRef::create(context, "print"),
          builtinPrint,
          1,
          true,
          false);
      FunctionObjectRef* buildFunctionObjectRef =
          FunctionObjectRef::create(state, nativeFunctionInfo);
      context->globalObject()->defineDataProperty(
          state,
          StringRef::createFromASCII("print"),
          buildFunctionObjectRef,
          true,
          true,
          true);
    }

    {
      FunctionObjectRef::NativeFunctionInfo nativeFunctionInfo(
          AtomicStringRef::create(context, "load"),
          builtinLoad,
          1,
          true,
          false);
      FunctionObjectRef* buildFunctionObjectRef =
          FunctionObjectRef::create(state, nativeFunctionInfo);
      context->globalObject()->defineDataProperty(
          state,
          StringRef::createFromASCII("load"),
          buildFunctionObjectRef,
          true,
          true,
          true);
    }

    {
      FunctionObjectRef::NativeFunctionInfo nativeFunctionInfo(
          AtomicStringRef::create(context, "read"),
          builtinRead,
          1,
          true,
          false);
      FunctionObjectRef* buildFunctionObjectRef =
          FunctionObjectRef::create(state, nativeFunctionInfo);
      context->globalObject()->defineDataProperty(
          state,
          StringRef::createFromASCII("read"),
          buildFunctionObjectRef,
          true,
          true,
          true);
    }

    {
      FunctionObjectRef::NativeFunctionInfo nativeFunctionInfo(
          AtomicStringRef::create(context, "run"), builtinRun, 1, true, false);
      FunctionObjectRef* buildFunctionObjectRef =
          FunctionObjectRef::create(state, nativeFunctionInfo);
      context->globalObject()->defineDataProperty(
          state,
          StringRef::createFromASCII("run"),
          buildFunctionObjectRef,
          true,
          true,
          true);
    }

    {
      FunctionObjectRef::NativeFunctionInfo nativeFunctionInfo(
          AtomicStringRef::create(context, "gc"), builtinGc, 0, true, false);
      FunctionObjectRef* buildFunctionObjectRef =
          FunctionObjectRef::create(state, nativeFunctionInfo);
      context->globalObject()->defineDataProperty(
          state,
          StringRef::createFromASCII("gc"),
          buildFunctionObjectRef,
          true,
          true,
          true);
    }

    return ValueRef::createUndefined();
  });

  return true;
}

bool App::evalScript(const char* str,
                     const char* fileName,
                     bool shouldPrintScriptResult,
                     bool isModule) {
  auto strRef = StringRef::createFromUTF8(str, strlen(str));
  auto fileNameRef = StringRef::createFromUTF8(fileName, strlen(fileName));
  return evalScript(strRef, fileNameRef, shouldPrintScriptResult, isModule);
}

bool App::evalScript(StringRef* str,
                     StringRef* fileName,
                     bool shouldPrintScriptResult,
                     bool isModule) {
  if (_context.get() == nullptr) {
    return false;
  }

  return App::evalScript(
      _context, str, fileName, shouldPrintScriptResult, isModule);
}

bool App::evalScript(ContextRef* context,
                     StringRef* str,
                     StringRef* fileName,
                     bool shouldPrintScriptResult,
                     bool isModule) {
  if (stringEndsWith(fileName->toStdUTF8String(), "mjs")) {
    isModule = isModule || true;
  }

  auto scriptInitializeResult =
      context->scriptParser()->initializeScript(str, fileName, isModule);
  if (!scriptInitializeResult.script) {
    printf("Script parsing error: ");
    switch (scriptInitializeResult.parseErrorCode) {
      case Escargot::ErrorObjectRef::Code::SyntaxError:
        printf("SyntaxError");
        break;
      case Escargot::ErrorObjectRef::Code::EvalError:
        printf("EvalError");
        break;
      case Escargot::ErrorObjectRef::Code::RangeError:
        printf("RangeError");
        break;
      case Escargot::ErrorObjectRef::Code::ReferenceError:
        printf("ReferenceError");
        break;
      case Escargot::ErrorObjectRef::Code::TypeError:
        printf("TypeError");
        break;
      case Escargot::ErrorObjectRef::Code::URIError:
        printf("URIError");
        break;
      default:
        break;
    }
    printf(": %s\n",
           scriptInitializeResult.parseErrorMessage->toStdUTF8String().data());
    return false;
  }

  auto evalResult = Evaluator::execute(
      context,
      [](ExecutionStateRef* state, ScriptRef* script) -> ValueRef* {
        return script->execute(state);
      },
      scriptInitializeResult.script.get());

  if (!evalResult.isSuccessful()) {
    printf("Uncaught %s:\n",
           evalResult.resultOrErrorToString(context)->toStdUTF8String().data());
    for (size_t i = 0; i < evalResult.stackTraceData.size(); i++) {
      printf("%s (%d:%d)\n",
             evalResult.stackTraceData[i].src->toStdUTF8String().data(),
             (int)evalResult.stackTraceData[i].loc.line,
             (int)evalResult.stackTraceData[i].loc.column);
    }
    return false;
  }

  if (shouldPrintScriptResult) {
    puts(evalResult.resultOrErrorToString(context)->toStdUTF8String().data());
  }

  while (context->vmInstance()->hasPendingPromiseJob()) {
    auto jobResult = context->vmInstance()->executePendingPromiseJob();
    if (shouldPrintScriptResult) {
      if (jobResult.error) {
        printf(
            "Uncaught %s:\n",
            jobResult.resultOrErrorToString(context)->toStdUTF8String().data());
      } else {
        printf(
            "%s\n",
            jobResult.resultOrErrorToString(context)->toStdUTF8String().data());
      }
    }
  }
  return true;
}
}  // namespace EscargotShim
