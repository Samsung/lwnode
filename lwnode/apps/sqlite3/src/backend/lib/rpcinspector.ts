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
import { log } from './log';
import { stringify, isObject } from './utils';
const debug = (() => {
  let debug = require('debug')('rpc');
  return {
    recv: debug.extend('payload:recv'),
    send: debug.extend('payload:send'),
    status: debug.extend('status'),
  };
})();

export class RPCInspector {
  lastId: number;
  registery: Set<number>;
  constructor() {
    this.lastId = -1;
    this.registery = new Set();
  }

  onRecv(payload) {
    const { id } = payload;

    debug.recv(chalk.bold.bgMagenta('RECV'), stringify(payload));

    this.registery.add(id);
  }

  onSend(payload) {
    if (!isObject(payload)) {
      log.warn(`typeof payload isn't object`);
      payload = JSON.parse(payload);
    }

    const { id, result, error } = payload;

    // check if FIN is sent
    if (result == 'FIN') {
      if (this.registery.has(id) === false) {
        log.warn(`FIN ${id} already sent.`);
      }

      this.registery.delete(id);

      debug.status('STATUS', `${this.registery.size} request(s) left`);
    }

    // print debug log
    if (result != 'FIN') {
      let tag = chalk.bold.bgCyan('SEND');

      if (error) {
        tag += ' ' + chalk.bold.bgRedBright('ERROR');
      }

      if (result == 'FIN') {
        tag += ' ' + chalk.bold.bgGrey('FIN');
      }

      if (result && result.hasMoreData === true) {
        tag += ' ' + chalk.bold.bgCyan('MORE');
      }

      debug.send(tag, stringify(payload));
    }
  }
}

export const inspector = new RPCInspector();
