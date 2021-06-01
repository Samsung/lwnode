/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

(function () {
  // Simulate V8 JavaScript stack trace API
  function StackFrame(func, funcName, fileName, lineNumber, columnNumber) {
    this.column = columnNumber | 0;
    this.lineNumber = lineNumber | 0;
    this.scriptName = fileName | 'Unknown';
    this.functionName = funcName | 'Unknown';
    this.function = func | function () { };
  }

  StackFrame.prototype.getFunction = function () {
    // TODO: Fix if .stack is called from different callsite
    // from where Error() or Error.captureStackTrace was called
    return this.function;
  };

  StackFrame.prototype.getTypeName = function () {
    //TODO : Fix this
    return this.functionName;
  };

  StackFrame.prototype.getMethodName = function () {
    return this.functionName;
  };

  StackFrame.prototype.getFunctionName = function () {
    return this.functionName;
  };

  StackFrame.prototype.getFileName = function () {
    return this.scriptName;
  };

  StackFrame.prototype.getLineNumber = function () {
    return this.lineNumber;
  };

  StackFrame.prototype.getColumnNumber = function () {
    return this.column;
  };

  StackFrame.prototype.isEval = function () {
    // TODO
    return false;
  };

  StackFrame.prototype.isToplevel = function () {
    // TODO
    return false;
  };

  StackFrame.prototype.isNative = function () {
    // TODO
    return false;
  };

  StackFrame.prototype.isConstructor = function () {
    // TODO
    return false;
  };

  StackFrame.prototype.toString = function () {
    return (this.functionName || 'Anonymous function') + ' (' +
      this.scriptName + ':' + this.lineNumber + ':' + this.column + ')';
  };

  function captureStackTrace(err, func) {
    // TODO: implement error stack
    err.stack = [];
    err.stack.push(
      new StackFrame(func, undefined, undefined, undefined, undefined));
    err.stack.push(
      new StackFrame(func, undefined, undefined, undefined, undefined));
    err.stack.push(
      new StackFrame(func, undefined, undefined, undefined, undefined));
  }

  Error.captureStackTrace = captureStackTrace;

})();
