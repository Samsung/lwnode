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

import { Database } from './database';
import { Statement } from './statement';
import { Socket, OperationMessage } from './socket';
import { constant } from './variables';
import {
  printLog,
  SQLError,
} from './utils';

/**
 * This class represents SQLite3
 *
 * @file
 *
 * sqlite3 is registered as a global object. The following example shows
 * how to create an DB object. For more DB API usage, please refer to the
 * DB test files in sample/script/test.
 *
 * @example
 * // 1. Setup the global sqlite3 object with URL once
 * let url = 'ws://127.0.0.1:8140/' // where sqlite3 is running
 * sqlite3.configure(url);
 *
 * // 2. Create an DB object
 * let db = await new sqlite3.Database('filename.db'); // DB is now ready to use
 *
 */
class SQLite3 {
  private m_url: string;
  private m_socket: Socket;

  /**
   * The constructor for a sqlite3 object.
   *
   * @access public
   * @hideconstructor
   * @param {string} url - A host's url
   */
  constructor(url: string) {
    this.m_url = url;
    this.m_socket = new Socket(url);
  }

  /**
   * Open a database connection.
   *
   * @access private
   * @ignore
   * @param {string} filename - A database filename to create
   * @param {int} mode - One or more of sqlite3.OPEN_READONLY,
   *   sqlite3.OPEN_READWRITE and sqlite3.OPEN_CREATE. The default
   *   value is OPEN_READWRITE | OPEN_CREATE.
   * @return {Promise<Database>} - A promise containing a database object.
   */
  async open(filename: string, mode = constant.OPEN_READWRITE | constant.OPEN_CREATE): Promise<Database> {
    await this.openSocket();
    let db = new Database(filename);

    let promise = new Promise<Database>((resolve, reject) => {
      db.open(mode)
        .then((newDb) => {
          resolve(newDb);
        })
        .catch((err) => {
          reject(err);
        });
    });

    return promise;
  }

  /**
   * Sets the execution mode to verbose to produce long stack traces.
   * There is no way to reset this.
   *
   * @access public
   * @return {Promise<undefined>}
   */
  async verbose(): Promise<undefined> {
    let socket = await this.openSocket();
    let promise = new Promise<undefined>((resolve, reject) => {
      socket.sendRequest('Database#verbose', (msg: OperationMessage) => {
        resolve(undefined);
      });
    });

    return promise;
  }

  async versionNumber(): Promise<number> {
    let socket = await this.openSocket();
    let promise = new Promise<number>((resolve, reject) => {
      socket.sendRequest('Database#versionNumber', (msg: OperationMessage) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let versionNumber: number = msg.data.m_data;
        resolve(versionNumber);
      });
    });

    return promise;
  }

  /**
   * The sqlite3 version number
   *
   * @access public
   * @example
   *  let ver = await sqlite3.VERSION_NUMBER;
   * @return {Promise<number>}
   */
  get VERSION_NUMBER(): Promise<number> {
    return this.versionNumber();
  }

  socket(): Socket {
    return this.m_socket;
  }

  /**
   * @access private
   * @ignore
   */
  async openSocket(): Promise<Socket> {
    // Socket object
    if (!this.m_socket) {
      this.m_socket = new Socket(this.m_url);
    }
    await this.m_socket.open();
    return this.m_socket;
  }

  /**
   * Disconnect a connection to the server.
   *
   * @access private
   * @ignore
   */
  closeSocket(): void {
    printLog('Closing a socket');
    if (this.m_socket) {
      this.m_socket.close();
      this.m_socket = null;
    }
  }
}

module.exports = (function () {
  let instance: SQLite3;

  function configure(url): void {
    if (instance) {
      throw new Error("Reconfiguration isn't allowed.");
    }
    instance = new SQLite3(url);
  }

  function verbose(): Promise<undefined> {
    if (!instance) {
      throw new Error("Error: sqlite3 not exist");
    }

    return instance.verbose();
  }

  function closeSocket(): void {
    if (!instance) {
      throw new Error("Error: sqlite3 not exist");
    }

    instance.closeSocket();
  }

  const globalSqlite3Object = {
    Database,
    Statement,
    verbose,
    ...constant,
    configure,
    closeSocket,
  };

  Object.defineProperty(globalSqlite3Object, 'VERSION_NUMBER', {
    get: function(): Promise<number> {
      return instance.VERSION_NUMBER;
    }
  });

  Object.freeze(globalSqlite3Object);

  return globalSqlite3Object;
})();
