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

import { Statement, StatementInfo } from './statement'; // StatementInfo
import { resolve } from 'path';
import { Socket, Param, OperationMessage, events } from './socket';
import { constant } from './variables';

import {
  printLog,
  printError,
  splitParameter,
  serializeParameters,
  SQLError,
  SQLite3Error,
} from './utils';

interface DatabaseSqlite3 { // FIXME: From backend. Share with backend
  open: boolean;
  filename: string;
  mode: number;

  close(...params);
  configure(...params);
  run(...params);
  get(...params);
  all(...params);
  each(...params);
  exec(...params);
  prepare(...params);
  on(...params);
  removeAllListeners(...params);
  wait(...params);
}

export interface DatabaseInfo { // From backend. Share with backend
  m_id: number;
  m_db: DatabaseSqlite3;
  m_stmtInfos: Map<number, StatementInfo>;
  m_nextStmtId: number;
}

export type UserEventCallback = (...params) => {};

/**
 * This class represents Database.
 *
 * @access public
 * @alias sqlite3.Database
 */
export class Database {
  private m_socket: Socket;
  private m_id: number;
  private m_dbInfo: DatabaseInfo;
  private m_filename: string;

  /**
   * Returns a new Database object and automatically opens the database.
   * There is no separate method to open the database.
   * @access public
   * @hideconstructor
   * @param {string} filename - Valid values are filenames, and
   *  ":memory:" for an anonymous in-memory database
   * @param {int} [mode] - One or more of sqlite3.OPEN_READONLY,
   *   sqlite3.OPEN_READWRITE and sqlite3.OPEN_CREATE. The default
   *   value is OPEN_READWRITE | OPEN_CREATE.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Database>} - A promise containing a database object.
   * @example
   *  // 1. Setup the global sqlite3 object with URL once
   *  let url = 'ws://127.0.0.1:8140/' // where sqlite3 is running
   *  sqlite3.configure(url);
   *
   *  // 2. Create an DB object
   *  let db = await new sqlite3.Database('filename.db');
   */
  constructor(filename: string, ...args) {
    let mode = constant.OPEN_READWRITE | constant.OPEN_CREATE;
    let params: Param[] = args;

    if (Number.isInteger(params[0])) {
      mode = params[0] as number;
    }

    if (!Socket.singleton()) {
      throw SQLite3Error.uninitialized();
    } else {
      this.m_socket = Socket.singleton();
    }

    this.m_id = -1; // A unique DB ID on the server
    this.m_dbInfo = null; // A copy of the DB representation on the server
    this.m_filename = filename;

    // NOTE: this constructor returns a Promise instance
    // Once succeeded the Promise object will be resolved with `this`.
    // @ts-ignore
    return this.open(mode);
  }

  /**
   * @access private
   * @return {Promise<Database>} - A promise containing this database object.
   */
  open(mode = constant.OPEN_READWRITE | constant.OPEN_CREATE): Promise<Database> {
    let self = this;
    let promise = new Promise<Database>((resolve, reject) => {
      this.sendRequest('open', [mode], (msg) => {
        printLog('db connected: ' + self.m_socket.url);
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        let dbInfo = msg.data.m_dbInfo;
        self.m_id = dbInfo.m_id;
        resolve(self);
      });
    });

    return promise;
  }

  /**
   * Close the database.
   *
   * @access public
   * @return {Promise<undefined>}
   */
  close(): Promise<undefined> {
    let promise = new Promise<undefined>((resolve, reject) => {
      this.sendRequest('close', [], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        events.forEach((e) => {
          this.socket().removeAllListeners(e);
        });

        resolve(undefined);
      });
    });

    return promise;
  }

  wait(): Promise<undefined> {
    let promise = new Promise<undefined>((resolve, reject) => {
      this.sendRequest('wait', [], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        resolve(undefined);
      });
    });

    return promise;
  }

  toJSON(): string {
    return JSON.stringify(this.m_dbInfo);
  }

  /**
   * @access public
   * @ignore
   */
  id(): number {
    return this.m_id;
  }

  /**
   * @access public
   * @ignore
   */
  filename(): string {
    return this.m_filename;
  }

  /**
   * @access public
   * @ignore
   */
  socket(): Socket {
    return this.m_socket;
  }

  /**
   * @access private
   * @ignore
   */
  // FIXME: remove it once on() and configure() are done
  async openSocket(): Promise<Socket> {
    await this.m_socket.open();
    return this.m_socket;
  }

  async sendRequest(method: string, params: Param[], handler: (msg: OperationMessage) => void) {
    try {
      let serialized: Param[] = [this.m_filename, this.m_id];
      serialized = serialized.concat(serializeParameters(params));

      await this.socket().sendRequest(
        `Database#${method}`,
        ...serialized,
        handler,
      );
    } catch (error) {
      throw error;
    }
  }

  /**
   * Sets an event handler for the database. Valid events and callback signatures
   * are as follows:
   *
   * <table>
   * <tr>
   *   <th>Event</th>
   *   <th>Callback</th>
   *   <th>Description</th>
   * </tr>
   * <tr>
   *   <td>trace</td>
   *   <td>function(query: string):void</td>
   *   <td>Invoked when an SQL statement starts to execute, with a rendering of
   *       the statement text.</td>
   * </tr>
   * <tr>
   *   <td>profile</td>
   *   <td>function(query: string, time: number):void</td>
   *   <td>Invoked when an SQL statement finishes execution, with a rendering
   *       of the statement text, and its execution time taken</td>
   * </tr>
   * </table>
   *
   * To enable event handling, call <code>sqlite3.verbose()</code> before
   * registering callbacks.
   *
   * @access public
   * @param {string} event - A name of an event to handle.
   * @param {function(query, time):void} callback - A callback for the event.
   * @return {Promise<undefined>}
   */
  on(event: string, callback: UserEventCallback): Promise<undefined> {
    let res = new Promise<undefined>((resolve, reject) => {
      if (!callback) {
        reject();
        return;
      }

      this.socket().addEventListener(event, callback);

      this.sendRequest('on', [event], (msg) => {
        // registration always succeed
        resolve(undefined);
      });
    });

    return res;
  }

  /**
   * Removes an event handler for the database. Valid events and callback signatures
   * are the same as Database#on().
   *
   * @access public
   * @param {string} event - A name of an event to handle.
   * @param {function(query, time):void} callback - A callback for the event.
   * @return {Promise<undefined>}
   */
  removeListener(event: string, callback: UserEventCallback): Promise<undefined> {
    let res = new Promise<undefined>((resolve, reject) => {
      if (!callback) {
        reject();
        return;
      }

      this.socket().removeEventListener(event, callback);

      this.sendRequest('removeListener', [event], (msg) => {
        // registration always succeed
        resolve(undefined);
      });
    });

    return res;
  }

  /**
   * Removes all event handlers for the database. Valid events
   * are the same as Database#on().
   *
   * @access public
   * @param {string} event - A name of an event to handle.
   * @return {Promise<undefined>}
   */
  removeAllListeners(event: string): Promise<undefined> {
    let res = new Promise<undefined>((resolve, reject) => {
      this.socket().removeAllListeners(event);

      this.sendRequest('removeAllListeners', [event], (msg) => {
        // registration always succeed
        resolve(undefined);
      });
    });

    return res;
  }

  /**
   * Sets a configuration option for the database. Valid options are:
   * <ul>
   * <li>busyTimeout: provide an integer as a value.
   * Sets a busy handler that sleeps for a specified amount of time when a table is locked.
   * The handler will sleep multiple times until at least "ms" milliseconds of
   * sleeping have accumulated. After at least "ms" milliseconds of sleeping,
   * the handler returns 0 which causes sqlite3_step() to return SQLITE_BUSY.
   * Calling this routine with an argument less than or equal to zero turns off
   * all busy handlers.</li>
   * </ul>
   *
   * @access public
   * @param {string} option - A name of an option to set.
   * @param {int} value - Time in milliseconds.
   * @return {Promise<undefined>}
   */
  configure(option: string, value: number): Promise<undefined> {
    let res = new Promise<undefined>((resolve, reject)=>{
      this.sendRequest('configure', [option, value], (msg) => {
        // Always succeed
        resolve(undefined);
      })
    });

    return res;
  }

  /**
   * Runs the SQL query with the specified parameters and calls the callback
   * afterwards. It does not retrieve any result data.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here. They will be bound to the
   *   statement before it is executed.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Statement>} - A promise containing an executed statement
   */
  run(query: string, ...args): Promise<Statement> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<Statement>((resolve, reject) => {
      this.sendRequest('run', [query, ...params], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let packet = msg.data;
        // @ts-ignore: m_stmtInfo is in Object format
        let stmt = new Statement(self, new StatementInfo(packet.m_stmtInfo)); // stmtInfo: from database.js
        resolve(stmt);
      });
    });

    return promise;
  }

  /**
   * Runs the SQL query with the specified parameters and calls the callback with
   * the first result row afterwards. The parameters are the same as the Database#run
   * function.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<(undefined|Object)>} - A promise containing undefined, if the
   *   result set is empty, otherwise an object containing the
   *   values for the first row. The property names correspond to the column names
   *   of the result set. It is impossible to access them by column index; the only
   *   supported way is by column name.
   */
  get(query: string, ...args): Promise<Object> {
    let params: Param[] = args;

    let promise = new Promise<Object>((resolve, reject) => {
      this.sendRequest('get', [query, ...params], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        let packet = msg.data;
        let row = packet.m_data;
        resolve(row);
      });
    });

    return promise;
  }

  /**
   * Runs the SQL query with the specified parameters and calls the callback with
   * all result rows afterwards. The parameters are the same as the Database#run
   * function.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Object[]>} - A promise containing an array of rows.
   *   If the result set is empty,
   *   it will be an empty array, otherwise it will have an object for each result
   *   row which in turn contains the values of that row, like the Database#get
   *   function.
   */
  all(query: string, ...args): Promise<any[]> {
    let params: Param[] = args;

    let promise = new Promise<any[]>((resolve, reject) => {
      this.sendRequest('all', [query, ...params], (msg) => {
        let packet = msg.data;
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        let rows = packet.m_data;
        resolve(rows);
      });
    });

    return promise;
  }
  /**
   * Runs the SQL query with the specified parameters and calls the callback
   * once for each result row.
   *
   * <p>If you know that a query only returns a very limited number of rows,
   * it might be more convenient to use Database#all to retrieve all rows at once.
   *
   * <p>There is currently no way to abort execution.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @param {function(row):void} [callback] - (function(row):void)
   *   If the result set succeeds
   *   but is empty, the callback is never called. In all other cases, the
   *   callback is called once for every retrieved row. The order of calls correspond
   *   exactly to the order of rows in the result set.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<number>} - A promise containing the number of retrieved rows.
   */
  each(query: string, ...args): Promise<number> {
    let { params, callback : callbackRow } = splitParameter(args);

    let promise = new Promise<number>((resolve, reject) => {
      this.sendRequest('each', [query, ...params], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        let packet = msg.data;
        let rowVal = packet.m_data;
        if (msg.hasMoreData) {
          if (callbackRow && rowVal) {
            callbackRow(rowVal);
          }
        } else {
          resolve(rowVal);
        }
      });
    });

    return promise;
  }

  /**
   * Runs all SQL queries in the supplied string. No result rows are retrieved.
   * The function returns the Database object to allow for function chaining.
   * If a query fails, no subsequent statements will be executed (wrap it in
   * a transaction if you want all or none to be executed).
   *
   * <p>Note: This function will only execute statements up to the first NULL byte.
   * Comments are not allowed and will lead to runtime errors.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<undefined>}
   */
  exec(query: string, ...args): Promise<undefined> {
    let params: Param[] = args;

    let promise = new Promise<undefined>((resolve, reject) => {
      this.sendRequest('exec', [query], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }

        resolve(undefined);
      });
    });

    return promise;
  }

  /**
   * Prepares the SQL statement and optionally binds the specified parameters and
   * calls the callback when done. The function returns a Statement object.
   *
   * @access public
   * @param {string} query - The SQL query to run.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Statement>} - A promise containing a statement object.
   */
  prepare(query: string, ...args): Promise<Statement> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<Statement>((resolve, reject) => {
      this.sendRequest('prepare', [query, ...params], (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let packet = msg.data;
        // @ts-ignore: m_stmtInfo is in Object format
        let stmt = new Statement(self, new StatementInfo(packet.m_stmtInfo)); // stmtInfo: from database.js
        resolve(stmt);
      });
    });

    return promise;
  }
}
