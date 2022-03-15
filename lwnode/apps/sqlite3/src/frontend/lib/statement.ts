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

import { Socket, Param, OperationMessage } from './socket';
import type { Database } from './database';

import {
  splitParameter,
  serializeParameters,
  printLog,
  printError,
  SQLError,
  SQLite3Error,
} from './utils';

// FIXME: Get this type info from backend
export interface StatementSqlite3 { // Statement data structure obtained directly from libsqlite3
  sql: string;
  lastID?: number;
  changes?: number;
}

// FIXME: Get this type info from backend
export class StatementInfo {
  private m_dbId: number;
  private m_id: number; // Statement ID
  private m_stmt: StatementSqlite3;

  constructor(stmtInfo: { m_dbId: number, m_id: number, m_stmt: StatementSqlite3 }) {
    this.m_dbId = stmtInfo.m_dbId;
    this.m_id = stmtInfo.m_id;
    this.m_stmt = stmtInfo.m_stmt;
  }

  type() {
    return this.constructor.name;
  }

  id() {
    return this.m_id;
  }

  stmt(): StatementSqlite3 {
    return this.m_stmt;
  }
}

/**
 * This class represents Statement
 *
 */
export class Statement {
  private m_socket: Socket;
  private m_filename: string;
  private m_dbId: number;
  private m_stmtInfo: StatementInfo;

  /**
  * For internal use only
  *
  * @hideconstructor
  */
  constructor(db: Database, stmtInfo: StatementInfo) {
    if (!Socket.singleton()) {
      throw SQLite3Error.uninitialized();
    } else {
      this.m_socket = Socket.singleton();
    }

    this.m_filename = db.filename();
    this.m_dbId = db.id();
    this.m_stmtInfo = stmtInfo;
  }

  get lastID(): number {
    return this.m_stmtInfo ? this.m_stmtInfo.stmt().lastID : undefined;
  }

  get changes(): number {
    return this.m_stmtInfo ? this.m_stmtInfo.stmt().changes : undefined;
  }

  get sql(): string {
    if (!this.m_stmtInfo) {
      return undefined;
    }

    return this.m_stmtInfo.stmt() ? this.m_stmtInfo.stmt().sql : undefined;
  }

  toJSON(): string {
    let obj = this.m_stmtInfo.stmt();
    return JSON.stringify(obj);
  }

  /**
   * @access private
   * @ignore
   */
  async openSocket() {
    await this.m_socket.open();
    return this.m_socket;
  }

  socket(): Socket {
    return this.m_socket;
  }

  async sendRequest(method: string, params: Param[], handler: (msg: OperationMessage) => void) {
    try {
      let serialized: Param[] = [this.m_dbId, this.m_stmtInfo];
      serialized = serialized.concat(serializeParameters(params));

      await this.socket().sendRequest(
        `Statement#${method}`,
        ...serialized,
        handler);
    } catch (error) {
      throw error;
    }
  }

  /**
   * Binds parameters to the prepared statement and calls the callback
   * when done or when an error occurs.
   *
   * <p>Binding parameters with this function completely resets the statement
   * object and row cursor and removes all previously bound parameters,
   * if any.
   *
   * @access public
   * @param {...string} [param] - When the SQL statement contains placeholders,
   *   you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<undefined>}
   */
  bind(...args): Promise<undefined> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<undefined>((resolve, reject) => {
      self.sendRequest('bind', params, (msg) => {
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
   * Resets the row cursor of the statement and preserves the parameter
   * bindings. Use this function to re-execute the same query with the
   * same bindings. This action will never fail and will always return null.
   *
   * @access public
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<undefined>}
   */
  reset(): Promise<undefined> {
    let self = this;
    let promise = new Promise<undefined>((resolve, reject) => {
      self.sendRequest('reset', [], (msg) => {
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
   * Finalizes the statement. It is prefer to explicitly finalizing your
   * statement, as you might experience long delays before the next query
   * is executed. After the statement is finalized, all further function
   * calls on that statement object will throw errors.
   *
   * @access public
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<undefined>}
   */
  finalize(): Promise<undefined> {
    let self = this;
    let promise = new Promise<undefined>((resolve, reject) => {
      self.sendRequest('finalize', [], (msg) => {
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
   * Binds parameters and executes the statement.
   *
   * <p>If you specify bind parameters, they will be bound to the statement
   * before it is executed. Note that the bindings and the row cursor are
   * reset when you specify even a single bind parameter.
   *
   * <p>The behavior is identical to the Database#run method with
   * the difference that the statement will not be finalized after it is run.
   * This means you can run it multiple times.
   *
   * @access public
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Statement>} - A promise containing an executed statement
   */
  run(...args): Promise<Statement> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<Statement>((resolve, reject) => {
      self.sendRequest('run', params, (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let packet = msg.data;
        // @ts-ignore
        self.m_stmtInfo = new StatementInfo(packet.m_stmtInfo);
        resolve(self);
      });
    });

    return promise;
  }

  /**
   * Binds parameters, executes the statement and retrieves the first result
   * row.
   *
   * <p>Like with Statement#run, the statement will not be finalized after
   * executing this function.
   *
   * <p>Using this method can leave the database locked, as the database
   * awaits further calls to Statement#get to retrieve subsequent rows.
   * To inform the database that you are finished retrieving rows, you
   * should either finalize (with Statement#finalize) or reset
   * (with Statement#reset) the statement.
   *
   * @access public
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Object>} - A promise containing an object that contains
   *   the values for the first row.
   */
  get(...args): Promise<Object> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<Object>((resolve, reject) => {
      self.sendRequest('get', params, (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let packet = msg.data;
        let response = packet.m_data;
        let row = response;
        resolve(row);
      });
    });

    return promise;
  }

  /**
   * Binds parameters, executes the statement and calls the callback with all
   * result rows.
   *
   * <p>Like with Statement#run, the statement will not be finalized after
   * executing this function.
   *
   * @access public
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<Object[]>} - A promise containing an array, that contains an object
   *   for each result row which in turn contains the values of that row.
   */
  all(...args): Promise<Object[]> {
    let params: Param[] = args;

    let self = this;
    let promise = new Promise<Object[]>((resolve, reject) => {
      self.sendRequest('all', params, (msg) => {
        if (msg.error) {
          reject(new SQLError(msg.error));
          return;
        }
        let packet = msg.data;
        let response = packet.m_data;
        let rows = response;
        resolve(rows);
      });
    });

    return promise;
  }

  /**
   * Binds parameters, executes the statement and calls the callback for each
   * result row.
   *
   * <p>Like with Statement#run, the statement will not be finalized after
   * executing this function.
   *
   * <p>If you know that a query only returns a very limited number of rows,
   * it might be more convenient to use Statement#all to retrieve all rows
   * at once.
   *
   * There is currently no way to abort execution!
   *
   * @access public
   * @param {...string} [param] - When the SQL statement contains
   *   placeholders, you can pass them in here.
   * @param {function(row):void} [callback] - (function(row):void)
   *   If the result set succeeds but is empty, the callback is never called.
   *   In all other cases, the callback is called once for every retrieved row.
   *   The order of calls correspond exactly to the order of rows in the
   *   result set.
   * @throws - Throw an error object if an error occurred.
   * @return {Promise<number>} - A promise containing the number of retrieved rows.
   */
  each(...args): Promise<number> {
    let { params, callback : callbackRow } = splitParameter(args);

    let self = this;
    let promise = new Promise<number>((resolve, reject) => {
      self.sendRequest('each', params, (msg) => {
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
}
