/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

// Copyright Microsoft. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/* eslint-disable strict */
(function (keepAlive) {
  var global = this;

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

  function patchErrorStack() {
    Error.captureStackTrace = captureStackTrace;
  }

  // patch MapIterator

  var mapIteratorProperty = 'MapIteratorIndicator';
  function patchMapIterator() {
    var originalMapMethods = [];
    originalMapMethods.push(['entries', Map.prototype.entries]);
    originalMapMethods.push(['values', Map.prototype.values]);
    originalMapMethods.push(['keys', Map.prototype.keys]);

    originalMapMethods.forEach(function (pair) {
      Map.prototype[pair[0]] = function () {
        var result = pair[1].apply(this);
        Object.defineProperty(result, mapIteratorProperty,
          { value: true, enumerable: false, writable: false });
        return result;
      };
    });
  }

  // patch SetIterator

  var setIteratorProperty = 'SetIteratorIndicator';
  function patchSetIterator() {
    var originalSetMethods = [];
    originalSetMethods.push(['entries', Set.prototype.entries]);
    originalSetMethods.push(['values', Set.prototype.values]);

    originalSetMethods.forEach(function (pair) {
      Set.prototype[pair[0]] = function () {
        var result = pair[1].apply(this);
        Object.defineProperty(result, setIteratorProperty,
          { value: true, enumerable: false, writable: false });
        return result;
      };
    });
  }

  // patch Debug

  // Ensure global Debug object if not already exists, and patch it.
  function ensureDebug(otherGlobal) {
    if (!global.Debug) {
      Object.defineProperty(global, 'Debug', {
        value: {}, enumerable: false, configurable: false, writable: false
      });
    }

    otherProcess = otherGlobal.process;
    patchDebug(global.Debug);
  }

  var otherProcess;

  function patchDebug(Debug) {
    if (!Debug || Debug.MakeMirror) {
      return;
    }

    class Mirror {
      constructor(type) {
        this.type_ = type;
      }
      type() {
        return this.type_;
      }
    }

    class ValueMirror extends Mirror {
      constructor(type, value) {
        super(type);
        this.value_ = value;
      }
      value() {
        return this.value_;
      }
    }

    class UndefinedMirror extends ValueMirror {
      constructor() {
        super('undefined', undefined);
      }
    }
    const undefinedMirror = new UndefinedMirror();

    class ObjectMirror extends ValueMirror {
      constructor(type, value) {
        super(type || 'object', value);
      }
    }

    class PromiseMirror extends ObjectMirror {
      constructor(value) {
        super('promise', value);
      }
      status() {
        return '<unknown>';
      }
      promiseValue() {
        return new ValueMirror('<unknown>', '<unknown>');
      }
    }

    const util = otherProcess.binding('util');

    Debug.MakeMirror = (value) => {
      if (util.isPromise(value)) {
        return new PromiseMirror(value);
      }

      // Not supporting other types
      return undefinedMirror;
    };
  }

  // patch Utils

  // Simulate v8 micro tasks queue
  var microTasks = [];

  function patchUtils(utils) {
    var isUintRegex = /^(0|[1-9]\d*)$/;

    function isUint(value) {
      var result = isUintRegex.test(value);
      isUintRegex.lastIndex = 0;
      return result;
    }

    utils.cloneObject = function (source, target) {
      Object.getOwnPropertyNames(source).forEach(function (key) {
        try {
          var desc = Object.getOwnPropertyDescriptor(source, key);
          if (desc.value === source) desc.value = target;
          Object.defineProperty(target, key, desc);
        } catch (e) {
          // Catch sealed properties errors
        }
      });
    };

    utils.getEnumerableNamedProperties = function (obj) {
      var props = [];
      for (var key in obj) {
        if (!isUint(key))
          props.push(key);
      }
      return props;
    };

    utils.getEnumerableIndexedProperties = function (obj) {
      var props = [];
      for (var key in obj) {
        if (isUint(key))
          props.push(key);
      }
      return props;
    };

    utils.createEnumerationIterator = function (props) {
      var i = 0;
      return {
        next: function () {
          if (i === props.length)
            return { done: true };
          return { value: props[i++] };
        }
      };
    };

    utils.createPropertyDescriptorsEnumerationIterator = function (props) {
      var i = 0;
      return {
        next: function () {
          if (i === props.length) return { done: true };
          return { name: props[i++], enumerable: true };
        }
      };
    };

    utils.getNamedOwnKeys = function (obj) {
      var props = [];
      Object.keys(obj).forEach(function (item) {
        if (!isUint(item))
          props.push(item);
      });
      return props;
    };

    utils.getIndexedOwnKeys = function (obj) {
      var props = [];
      Object.keys(obj).forEach(function (item) {
        if (isUint(item))
          props.push(item);
      });
      return props;
    };

    utils.getStackTrace = function () {
      return captureStackTrace({}, undefined)();
    };

    utils.isMapIterator = function (value) {
      return value[mapIteratorProperty] === true;
    };

    utils.isSetIterator = function (value) {
      return value[setIteratorProperty] === true;
    };

    function compareType(o, expectedType) {
      return Object.prototype.toString.call(o) === '[object ' +
        expectedType + ']';
    }

    utils.isBooleanObject = function (obj) {
      return compareType(obj, 'Boolean');
    };

    utils.isDate = function (obj) {
      return compareType(obj, 'Date');
    };

    utils.isMap = function (obj) {
      return compareType(obj, 'Map');
    };

    utils.isNativeError = function (obj) {
      return compareType(obj, 'Error') ||
        obj instanceof Error ||
        obj instanceof EvalError ||
        obj instanceof RangeError ||
        obj instanceof ReferenceError ||
        obj instanceof SyntaxError ||
        obj instanceof TypeError ||
        obj instanceof URIError;
    };

    utils.isPromise = function (obj) {
      return compareType(obj, 'Object') && obj instanceof Promise;
    };

    utils.isRegExp = function (obj) {
      return compareType(obj, 'RegExp');
    };

    utils.isAsyncFunction = function (obj) {
      // NOTE: async isn't support yet
      return false;
    };

    utils.isSet = function (obj) {
      return compareType(obj, 'Set');
    };

    utils.isFunction = function (obj) {
      return compareType(obj, 'Function') || compareType(obj, 'GeneratorFunction');
    };

    utils.isStringObject = function (obj) {
      return compareType(obj, 'String');
    };

    utils.isNumberObject = function (obj) {
      return compareType(obj, 'Number');
    };

    utils.isArgumentsObject = function (obj) {
      return compareType(obj, 'Arguments');
    };

    utils.isGeneratorObject = function (obj) {
      return compareType(obj, 'Generator');
    };

    utils.isWeakMap = function (obj) {
      return compareType(obj, 'WeakMap');
    };

    utils.isWeakSet = function (obj) {
      return compareType(obj, 'WeakSet');
    };

    utils.isSymbolObject = function (obj) {
      return compareType(obj, 'Symbol');
    };

    utils.isDataView = function (obj) {
      return compareType(obj, 'DataView');
    };

    utils.isName = function (obj) {
      return compareType(obj, 'String') || compareType(obj, 'Symbol');
    };

    utils.getSymbolKeyFor = function (symbol) {
      return Symbol.keyFor(symbol);
    };

    utils.getSymbolFor = function (key) {
      return Symbol.for(key);
    };

    utils.jsonParse = function (text, reviver) {
      return JSON.parse(text, reviver);
    };

    utils.jsonStringify = function (value, replacer, space) {
      return JSON.stringify(value, replacer, space);
    };

    utils.enqueueMicrotask = function (task) {
      microTasks.push(task);
    };

    utils.dequeueMicrotask = function (task) {
      return microTasks.shift();
    };

    utils.isProxy = function (value) {
      // TODO:
      return false;
    };

    utils.getPropertyAttributes = function (object, value) {
      var descriptor = Object.getOwnPropertyDescriptor(object, value);
      if (descriptor === undefined) {
        return -1;
      }

      var attributes = 0;
      // taken from v8.h. Update if this changes in future
      const ReadOnly = 1,
        DontEnum = 2,
        DontDelete = 4;

      if (!descriptor.writable) {
        attributes |= ReadOnly;
      }
      if (!descriptor.enumerable) {
        attributes |= DontEnum;
      }
      if (!descriptor.configurable) {
        attributes |= DontDelete;
      }
      return attributes;
    };

    utils.getOwnPropertyNames = function (obj) {
      var ownPropertyNames = Object.getOwnPropertyNames(obj);
      var i = 0;
      while (i < ownPropertyNames.length) {
        var item = ownPropertyNames[i];
        if (isUint(item)) {
          ownPropertyNames[i] = parseInt(item);
          i++;
          continue;
        }
        // As per spec, getOwnPropertyNames() first include
        // numeric properties followed by non-numeric
        break;
      }
      return ownPropertyNames;
    };

    utils.createPropertyDescriptor = function (writable, enumerable, configurable, value, getter, setter) {
      var descriptor = {
        writable,
        enumerable,
        configurable
      };

      if (value) {
        descriptor.value = value;
      }

      if (getter) {
        descriptor.getter = getter;
      }

      if (setter) {
        descriptor.setter = setter;
      }

      return descriptor;
    };
  }

  patchErrorStack();
  patchMapIterator();
  patchSetIterator();

  patchUtils(keepAlive);

});
