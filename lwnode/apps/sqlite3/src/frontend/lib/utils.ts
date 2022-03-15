/*
 * Copyright 2020-present Samsung Electronics Co., Ltd. and other contributors
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

import type { Param, OperationMessage } from './socket';

let uid: number = 0;

export function currentRequestId() {
  return uid;
}

export function nextRequestId() {
  return ++uid;
}

export function isFunction(obj) {
  return typeof obj === 'function';
}

export function isDate(obj) {
  return Object.prototype.toString.call(obj) === '[object Date]';
}

export function isRegExp(obj) {
  return Object.prototype.toString.call(obj) === '[object RegExp]';
}

export function isInfinity(obj) {
  return obj === obj/0;
}

export function isArrayBuffer(obj) {
  return Object.prototype.toString.call(obj) === '[object ArrayBuffer]';
}

export function ab2str(buf: ArrayBuffer): string {
  return String.fromCharCode.apply(null, new Uint16Array(buf));
}

export function str2ab(str: string): ArrayBuffer {
  let buf = new ArrayBuffer(str.length * 2); // 2 bytes for each char
  let bufView = new Uint16Array(buf);
  for (let i = 0, strLen = str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i);
  }
  return buf;
}

function arrayBufferToBase64(buffer: ArrayBuffer): string {
  let binary = '';
  let bytes = new Uint8Array(buffer);
  let len = bytes.byteLength;
  for (let i = 0; i < len; i++) {
    binary += String.fromCharCode(bytes[i]);
  }
  return btoa(binary);
}

function base64ToArrayBuffer(base64: string): ArrayBuffer {
  let binary_string = atob(base64);
  let len = binary_string.length;
  let bytes = new Uint8Array(len);
  for (let i = 0; i < len; i++) {
    bytes[i] = binary_string.charCodeAt(i);
  }
  return bytes.buffer;
}

function arrayBufferToBase64_16(buffer: ArrayBuffer): string {
  let binary = String.fromCharCode.apply(null, new Uint16Array(buffer));
  return btoa(binary);
}

function base64ToArrayBuffer_16(base64: string): ArrayBuffer {
  let str = atob(base64);
  let buf = new ArrayBuffer(str.length * 2); // 2 bytes for each char
  let bufView = new Uint16Array(buf);
  for (let i = 0, strLen = str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i);
  }
  return buf;
}

function btoa(binaryString: string): string {
  if (Buffer) {
    return Buffer.from(binaryString, 'binary').toString('base64');
  } else {
    return window.btoa(binaryString); // browser environment
  }
}

function atob(base64String: string): string {
  if (Buffer) {
    return Buffer.from(base64String, 'base64').toString('binary');
  } else {
    return window.atob(base64String); // browser environment
  }
}

export function printLog(str) {
  // console.log(str);
}

export function printError(str) {
  // console.error(str);
}

export function splitParameter(args): { params: Param[], callback: (msg: OperationMessage) => void } {
  let params = args,
    callback = null;
  if (typeof args[args.length - 1] === 'function') {
    callback = args[args.length - 1];
    params = args.slice(0, args.length - 1);
  }
  return {
    params,
    callback,
  };
}

export function serializeParameters(params: Param[]): Param[] {
  let serialized = [];
  params.forEach((e) => {
    if (isDate(e)) {
      serialized.push(+e);
    } else if (isRegExp(e) || isInfinity(e)) {
      serialized.push(String(e));
    } else if (isArrayBuffer(e)) {
      let abObject = {
        'arrayBuffer': arrayBufferToBase64(e as ArrayBuffer),
      };
      serialized.push(abObject);
    } else {
      serialized.push(e);
    }
  });

  return serialized;
}

export function restoreArrayBuffer(obj): void {
  for (let k in obj) {
    if (obj[k] && obj[k].arrayBuffer) {
      let val = obj[k].arrayBuffer;
      let arrayBuffer = base64ToArrayBuffer(val);
      obj[k] = arrayBuffer;
    } else {
      if (typeof obj[k] == 'object' && obj[k] !== null) {
        restoreArrayBuffer(obj[k]);
      }
    }
  }
}

export class SQLError extends Error {
  code: string;
  errno: number;

  constructor(e) {
    super(e.message);
    this.code = e.code;
    this.errno = e.errno;
  }
}

export const SQLite3Error = {
  uninitialized: () =>
    new Error('sqlite3.configure(`connection_url: string`) must be called.'),
  timeout: () =>
    new Error('request timeout.'),
};

export function setPromiseTimeout(delay, promise) {
  let timeout = new Promise((resolve, reject) => {
    let id = setTimeout(() => {
      clearTimeout(id);
      reject(SQLite3Error.timeout());
    }, delay);
  });

  return Promise.race([promise, timeout]);
}
