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

const jsonrpc = require('jsonrpc-lite');
import { Socket } from './socket';
import { Response } from './response';
import {
  isFunction,
  isValidationError,
  getValidationErrorMessage,
} from './utils';
import { log } from './log';
const debug = require('debug')('rpc:router');
const inspector = require('./rpcinspector').inspector;

/*
 Usage:
  const rpc = RPC.create();

  // sync
  rpc.set('ping', (ws)=> { ws.send()});

  // async
  rpc.set('ping', (ws)=> { return new Promise((resolve, reject) => {}); });

  // groups
  let api = { ping(ws) {ws.send()} }
  rpc.use(api);
*/


export interface Request {
  payload;
  socket: Socket;
}

export interface Context {
  request: Request;
  response: Response;
}

class RPC {
  api;
  constructor() {
    this.api = {};
  }

  handle(ws) {
    let socket = new Socket(ws, inspector);

    socket.onMessage(async (msg) => {
      try {
        let { payload } = jsonrpc.parse(msg);

        // debug('RECV', toJson(payload));

        if (payload instanceof jsonrpc.JsonRpcError) {
          throw payload;
        }

        let { id, method, params } = payload;

        if (!this.api[method]) {
          throw jsonrpc.JsonRpcError.methodNotFound();
        }

        let request: Request = { payload, socket };
        let response = new Response(request, payload);

        // NOTE: consider creating Context and Request if needed
        let context: Context = { request, response };

        debug(`${method}`);

        await this.api[method](context, params);

        // send `FIN` once a requested task is done.
        socket.send(jsonrpc.success(id, 'FIN'));
      } catch (error) {
        if (error instanceof jsonrpc.JsonRpcError) {
          socket.send(error);
        } else {
          if (isValidationError(error)) {
            socket.send(
              jsonrpc.JsonRpcError.invalidParams(
                getValidationErrorMessage(error),
              ),
            );
          } else if (!error) {
            log.warn('undefined error is thrown');
          } else {
            socket.send(jsonrpc.JsonRpcError.internalError());
            log.error(`internalError`);
            log.error(error);
          }
        }
      }
    });
  }

  use(prefix, api) {
    for (const [key, value] of Object.entries(api)) {
      if (isFunction(value)) {
        this.set(`${prefix}${key}`, value);
      }
    }
  }

  set(method, handler) {
    if (this.api[method]) {
      throw new Error('already registered');
    }
    this.api[method] = handler;
  }

  unset(method) {
    if (this.api[method]) {
      delete this.api[method];
    }
  }

  static create() {
    return new RPC();
  }
}

module.exports = RPC;
