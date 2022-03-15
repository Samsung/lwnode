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

import type { DatabaseInfo, UserEventCallback } from './database';
import type { StatementInfo } from './statement';
import {
  currentRequestId,
  nextRequestId,
  isFunction,
  setPromiseTimeout,
  splitParameter,
  printLog,
  printError,
  restoreArrayBuffer,
  ab2str,
} from './utils';

// FIXME: Get this type info from backend
export type Param = string | number | ArrayBuffer | StatementInfo | ((msg: OperationMessage) => void);

interface OperationRequest { // Sending a request
  jsonrpc: string;
  method: string;
  params: Param[];
  id: number;
}

interface OperationResult {
  method: string;
  hasMoreData: boolean;
  data: {
    m_dbInfo?: DatabaseInfo;    // should be either m_dbInfo or m_stmtInfo
    m_stmtInfo?: StatementInfo;
    m_data; // Type depends on the operation return type
  };
}

interface OperationResponse {
  jsonrpc: string;
  id: number;
  result: string | OperationResult; // 'FIN' or OperationResult;
  error?: {
    code: number;    // libsqlite3 error code
    message: string; // libsqlite3 message
    data: {
      type: string;
      method: string; // e.g., Database#open
      code: string;
      errno: number;
      filename: string;
      data; // DBInfo?
    };
  }
}

export interface OperationMessage {
  classname: string;
  method: string;
  hasMoreData?: boolean;
  data: {
    m_dbInfo?: DatabaseInfo;    // should be either m_dbInfo or m_stmtInfo
    m_stmtInfo?: StatementInfo;
    m_data; // Type depends on the operation return type
  }
  status: string;   // 'ok' or 'error'
  message?: string; // optional error message
  error?: {         // libsqlite3 error object
    code: string;
    errno: number;
    message: string;
  }
}

type SocketEventListner = (e) => void;
export const events = ['trace', 'profile'];

let g_socket: Socket = null;
export class Socket {
  private m_url: string;
  private m_socket: WebSocket;
  private m_eventListeners: Map<string, Map<UserEventCallback, SocketEventListner>>;


  constructor(url: string) {
    if (g_socket) {
      return;
    }

    g_socket = this;
    this.m_url = url;
    this.m_socket = null;
    this.m_eventListeners = new Map<string, Map<UserEventCallback, SocketEventListner>>();

    events.forEach((e) => {
      this.m_eventListeners.set(e, new Map<UserEventCallback, SocketEventListner>());
    });
  }

  static singleton(): Socket {
    return g_socket;
  }

  get url(): string {
    return this.m_url;
  }

  open(): Promise<Socket> {
    let self = this;
    let promise = new Promise<Socket>((resolve, reject) => {
      if (self.m_socket != null) {
        resolve(self);
        return;
      }

      self.m_socket = new WebSocket(self.m_url);
      self.m_socket.addEventListener('open', () => {
        printLog('socket: create a new socket');
        resolve(self);
      });

      self.m_socket.addEventListener('close', () => {
        self.m_socket.close();
        self.m_socket = null;
      });
    });

    return promise;
  }

  close(): void {
    if (this.m_socket) {
      this.m_socket.close();
    }
  }

  async sendRequest(method: string, ...args: Param[]) {
    let { params = [], callback } = splitParameter(args);

    try {
      let socket = await this.open();
      this.onMessage(currentRequestId(), callback);
      let msgRequest: OperationRequest = {
        jsonrpc: '2.0',
        method,
        params,
        id: currentRequestId(),
      };

      socket.m_socket.send(JSON.stringify(msgRequest));
      nextRequestId();
      return;
    } catch (error) {
      throw error;
    }
  }

  parseJsonRpcObject(jsonRpcObject: OperationResponse): OperationMessage {
    let error = jsonRpcObject.error;
    let payload = error ? jsonRpcObject.error.data : (jsonRpcObject.result as OperationResult);
    let errData = error ? jsonRpcObject.error.data : null;
    let okData = error ? null: jsonRpcObject.result as OperationResult;

    let [classname, method] = payload.method.split('#');
    let msg: OperationMessage = {
      classname,
      method,
      hasMoreData: error ? undefined: okData.hasMoreData,
      data: payload.data,
      status: error ? 'error' : 'ok',
      message: error ? error.message : undefined,
      error: error
        ? {
            code: errData.code,
            errno: errData.errno,
            message: error.message,
          }
        : undefined,
    };

    {
      // Restore ArrayBuffers if exist
      let packet = msg.data;
      if (packet && packet.m_data) {
        // 'data' is a return value for an operation on the server.
        // It can be of any type.
        let data = packet.m_data;
        if (Array.isArray(data)) {
          for (let i in data) {
            restoreArrayBuffer(data);
          }
        } else {
          restoreArrayBuffer(data);
        }
      }
    }
    return msg;
  }

  addEventListener(event: string, callback: UserEventCallback): void {
    if (!callback) {
      return;
    }

    let self = this;
    let handler: SocketEventListner = function (e) {
      let parsed: OperationResponse = JSON.parse(e.data);
      if (parsed && parsed.jsonrpc != '2.0') {
        return;
      }

      if (parsed.result === 'FIN') {
        return;
      }

      let msg: OperationMessage = self.parseJsonRpcObject(parsed);
      let { error, classname, method, data } = msg;

      if (error) {
        printError(`Error: ${method}(): ${error.message}`);
      }

      if (method === 'on' && data.m_data) {
        let eventData = data.m_data;
        if (eventData.event === 'trace') {
          callback(eventData.query);
        } else if (eventData.event === 'profile') {
          callback(eventData.query, eventData.time);
        }
      }
    };

    let listeners = this.m_eventListeners.get(event);
    listeners.set(callback, handler);

    this.m_socket.addEventListener('message', handler);
  }

  removeEventListener(event: string, callback: UserEventCallback): void {
    let listeners = this.m_eventListeners.get(event);
    let handler: SocketEventListner = listeners.get(callback);
    listeners.delete(callback);

    this.m_socket.removeEventListener('message', handler);
  }

  removeAllListeners(event: string): void {
    let listeners = this.m_eventListeners.get(event);
    listeners.forEach((value, key) => {
      this.m_socket.removeEventListener('message', value);
    });
    listeners.clear();
  }

  onMessage(msgId: number, callback: (msg: OperationMessage) => void): void {
    if (!callback) {
      return;
    }

    let self = this;

    let cb: SocketEventListner = function (e) {
      let parsed = JSON.parse(e.data);
      if (parsed && parsed.jsonrpc != '2.0') {
        return;
      }

      // check the matched handler based on relaying handlers logic
      if (parsed.id != msgId) {
        return;
      }

      // `FIN` means that a single operation, which can be identified with
      // the unique `id`, requested is finished. No more response should come with the id.
      if (parsed.result === 'FIN') {
        if (parsed.id == msgId) {
          self.m_socket.removeEventListener('message', cb);
        }
        return;
      }

      let msg = self.parseJsonRpcObject(parsed);
      let { error, classname, method, data } = msg;

      if (error) {
        printError(`Error: ${method}(): ${error.message}`);
      }

      callback(msg);
    };

    self.m_socket.addEventListener('message', cb);
  }
}
