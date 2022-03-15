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

const requestSymbol = Symbol('request');
const payloadSymbol = Symbol('payload');

import { JSONRPCError } from './rpcerror';
import type { Request } from './rpc';
import type { Socket } from './socket';

export class Response {
  static readonly option = { AutoFill: 'auto' };

  constructor(request: Request, payload) {
    this[requestSymbol] = request;
    this[payloadSymbol] = payload;
  }

  get socket(): Socket {
    return this[requestSymbol].socket;
  }

  write(data) {
    if (data && data.method === Response.option.AutoFill) {
      data.method = this[requestSymbol].payload.method;
    }

    const output = jsonrpc.success(this[payloadSymbol].id, data);
    this[requestSymbol].socket.send(output);
  }

  send(data) {
    return this.write(data);
  }

  error(data, code = 0, message = '') {
    let meta = { code, message };

    if (data instanceof JSONRPCError) {
      // @ts-ignore
      meta.code = data.errno || code;
      meta.message = data.message;
    }

    if (data && data.method === Response.option.AutoFill) {
      data.method = this[requestSymbol].payload.method;
    }

    const output = jsonrpc.error(
      this[payloadSymbol].id,
      new jsonrpc.JsonRpcError(meta.message, meta.code, data),
    );

    this[requestSymbol].socket.send(output);
  }
}
