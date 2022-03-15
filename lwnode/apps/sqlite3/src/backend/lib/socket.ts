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

const chalk = require('chalk');
const WebSocket = require('ws');
const log = require('./log');
const debug = require('debug')('rpc:socket');
const jsonrpc = require('jsonrpc-lite');
const wsSymbol = Symbol('ws');

export class Socket {
  inspector;

  constructor(ws, inspector) {
    if (ws instanceof WebSocket && ws.readyState === WebSocket.OPEN) {
      debug(chalk.bold('OPEN'));
    }

    this[wsSymbol] = ws;
    this[wsSymbol].on('close', (closeCode) => {
      debug(chalk.bold('CLOSE'), `${closeCode}`);
    });

    this.inspector = inspector;
  }

  onMessage(callback) {
    if (typeof callback !== 'function') {
      return;
    }

    this[wsSymbol].on('message', (msg) => {
      let { payload } = jsonrpc.parse(msg);

      this.inspector && this.inspector.onRecv(payload);

      callback(msg);
    });
  }

  onClose(callback) {
    if (typeof callback !== 'function') {
      return;
    }

    this[wsSymbol].on('close', callback);
  }

  send(payload) {
    let encoded = payload;

    this.inspector && this.inspector.onSend(payload);

    try {
      if (typeof encoded !== 'string') {
        encoded = JSON.stringify(payload);
      }

      this[wsSymbol].send(encoded);
    } catch (e) {
      log.error(e);
    }
  }
}
