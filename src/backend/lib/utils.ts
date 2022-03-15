/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
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

const fs = require('fs');
const Joi = require('joi');

import { SQLiteError } from './sqlite3/error';
import type { Param, EncodedArrayBuffer } from './sqlite3/database';

export function isDebug() {
  if (process.env.NODE_ENV === 'production') {
    return false;
  }
  return process.env.DEBUG != undefined;
}

export function isLinux() {
  if (process.arch === 'x64') {
    return true;
  }
  return false;
}

export function isFunction(obj) {
  return typeof obj === 'function';
}

export function isObject(value) {
  return typeof value === 'object' && value !== null;
}

export function isError(e) {
  return e && e.message && typeof e.message === 'string';
}

export function isNumber(num: any): boolean {
  let schema = Joi.number().integer();
  const result = schema.validate(num);
  if (result.error) {
    return false;
  }
  return true;
}

export function isString(str: any): boolean {
  let schema = Joi.string();
  const result = schema.validate(str);
  if (result.error) {
    return false;
  }
  return true;
}

export function isValidationError(error) {
  // @ts-ignore
  return error instanceof Error && error.isJoi == true;
}

export function getValidationErrorMessage(error) {
  return error.message;
}

export function deleteFile(path) {
  return new Promise((resolve, reject) => {
    fs.access(path, fs.constants.F_OK, (err) => {
      if (err) {
        // doesn't exist already
        resolve(false);
        return;
      }

      fs.unlink(path, (err) => {
        err ? reject(err) : resolve(true);
      });
    });
  });
}

export function existFile(path) {
  return new Promise((resolve, reject) => {
    fs.stat(path, (err, stats) => {
      if (err) {
        // @ts-ignore
        if (err.errno !== process.ENOENT &&
          err.code !== 'ENOENT' &&
          err.syscall !== 'unlink') {
          reject(err);
        } else {
          resolve(false);
        }
      }
      resolve(true);
    });
  });
}

export function sliceString(v, limit = 40, left = 10) {
  let msg = '  ...sliced...  ';
  if (typeof v === 'string' && v.length > limit + left + msg.length) {
    return v.substring(0, limit) + msg + v.substring(v.length - left);
  }
  return v;
}

export function stringify(obj, indent = 2) {
  if (!isDebug()) {
    return '';
  }

  let result;

  try {
    // https://stackoverflow.com/questions/6937863/json-stringify-so-that-arrays-are-on-one-line
    result = JSON.stringify(
      obj,
      (key, value) => {
        // stringify Error including properties not-enumerable
        if (value instanceof Error) {
          let error = {};
          Object.getOwnPropertyNames(value).forEach(function (key) {
            error[key] = value[key];
          });
          return error;
        }

        // slice the given value if its length is too long.
        value = sliceString(value);

        // stringify Array items in a single line
        if (
          Array.isArray(value) &&
          !value.some((x) => x && typeof x === 'object')
        ) {
          return `\uE000${JSON.stringify(
            value.map((v) => {
              v = sliceString(v);
              return typeof v === 'string' ? v.replace(/"/g, '\uE001') : v;
            }),
          )}\uE000`;
        }
        return value;
      },
      indent,
    ).replace(/"\uE000([^\uE000]+)\uE000"/g, (match) =>
      match
        .substr(2, match.length - 4)
        .replace(/\\"/g, '"')
        .replace(/\uE001/g, '\\"'),
    );
  } catch (error) {
    console.error(error);
  }
  return result;
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

export function restoreBufferFromArrayBufferInString(params: Param[]): void {
  for (let i = 0; i < params.length; i++) {
    let param = params[i];

    if (param && param.hasOwnProperty('arrayBuffer')) {
      try {
        let arrayBuffer = base64ToArrayBuffer((param as EncodedArrayBuffer).arrayBuffer);
        let nodeBuffer = Buffer.from(arrayBuffer);
        params[i] = nodeBuffer;
      } catch (err) {
        throw new SQLiteError('Error: Invalid ArrayBuffer received from client');
      }
    }
  }
}

export function serializeBufferToArrayBuffer(obj: any): void {
  for (let k in obj) {
    let val = obj[k];
    if (val && Buffer.isBuffer(val)) {
      let arrayBuffer = nodeBufferToArrayBuffer(val);
      let abObject = {
        arrayBuffer: arrayBufferToBase64(arrayBuffer),
      };
      obj[k] = abObject;
    }
  }
}

export function nodeBufferToArrayBuffer(nodeBuffer: Buffer): ArrayBuffer {
  let arrayBuffer = Uint8Array.from(nodeBuffer).buffer;
  return arrayBuffer;
}
