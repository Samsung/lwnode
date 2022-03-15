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

import { log } from '../log';
import { str2ab, isNumber, isString, restoreBufferFromArrayBufferInString } from '../utils';
import type { Context } from '../rpc';
import type { Socket } from '../socket';
import { StatementInfo } from './statement';

import { DatabasePacket, StatementPacket } from './packet';
import { SQLiteError } from './error';
const Joi = require('joi');

const debug = (() => {
  let debug = require('debug')('sql');
  return {
    database: debug.extend('database'),
  };
})();

let sqlite3 = null;

try {
  const versionString = 'v14.14.0';
  if (process.version != versionString) {
    log.warn(
      `(warn) compatible version: ${versionString} (cur: ${process.version})`,
    );
    log.warn(`(warn) we'll try using the original sqlite3`);
    sqlite3 = require('sqlite3');
  } else {
    sqlite3 = require('sqlite3');
  }
} catch (e) {
  log.error(`(exit) ${e.message}`);
  process.exit(1);
}

let g_nextDbId = -1; // A counter for generating DB Ids

interface DatabaseSqlite3 { // Obtained from libsqlite
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

export class ErrorSqlite3 extends Error {
  errno: number;
  code: string;
  constructor(errno, code, msg) {
    super(`${code}: ${msg}`);
    this.errno = errno;
    this.code = code;
  }
}

export class DatabaseManager {
  private m_clients: Map<Socket, DatabaseClient>;
  constructor() {
    this.m_clients = new Map<Socket, DatabaseClient>();
  }

  open(ctx: Context, filename: string, dbId: number, mode: number = undefined): Promise<DatabaseInfo> {
    let self = this;

    let executor = function (resolve, reject) {
      let client = self.getClient(ctx.response.socket);
      let dbInfo = client.databaseInfo(dbId);
      if (mode === undefined && dbInfo) {
        resolve(dbInfo);
        return;
      }

      if (dbId >= 0) {
        let err = new ErrorSqlite3(sqlite3.MISUSE, 'SQLITE_MISUSE', 'Database is closed');
        reject(new SQLiteError(err, dbInfo));
        return;
      }

      client
        .createDatabaseInfo(filename, mode)
        .then((dbInfo) => resolve(dbInfo))
        .catch((err) => reject(err));
    };

    return new Promise<DatabaseInfo>(executor);
  }

  verbose(): void {
    sqlite3.verbose();
  }

  getClient(socket: Socket): DatabaseClient {
    if (!this.m_clients.has(socket)) {
      let self = this;
      this.m_clients.set(socket, new DatabaseClient());

      socket.onClose(() => {
        self.m_clients.get(socket).destroy();
        self.m_clients.delete(socket);
      });
    }
    return this.m_clients.get(socket);
  }

  databaseInfo(ctx: Context, dbId: number): DatabaseInfo {
    let dbInfo = this.getClient(ctx.response.socket).databaseInfo(dbId);
    if (!dbInfo) {
      // A DB has been closed, but there is a request to access to
      // this closed DB. SQLite3 returns an error for this invalid use case.
      // But, since we remove the DB instance from the server when
      // a DB is closed, we cannot get the error message from the DB.
      // We imitate the error message by sending the same error
      // message to the client.
      let err = new ErrorSqlite3(sqlite3.MISUSE, 'SQLITE_MISUSE', 'Database is closed');
      throw new SQLiteError(err);
    }
    return dbInfo;
  }

  deleteDatabaseInfo(ctx: Context, dbId: number): void {
    this.getClient(ctx.response.socket).deleteDatabaseInfo(dbId);
  }
}

class DatabaseClient {
  private m_dbInfos: Map<number, DatabaseInfo>;

  constructor() {
    this.m_dbInfos = new Map<number, DatabaseInfo>();
  }

  createDatabaseInfo(filename: string, mode: number): Promise<DatabaseInfo> {
    let self = this;
    return new Promise<DatabaseInfo>((resolve, reject) => {
      let db = new sqlite3.Database(filename, mode, function (err) {
        // err != null, if an error occurred.
        if (err) {
          reject(err);
          return;
        }
      });

      let dbInfo = new DatabaseInfo(db);
      dbInfo.assignId();
      self.m_dbInfos.set(dbInfo.id(), dbInfo);
      resolve(dbInfo);
    });
  }

  databaseInfo(dbId: number): DatabaseInfo {
    return this.m_dbInfos.get(dbId);
  }

  deleteDatabaseInfo(dbInfoId: number): void {
    this.m_dbInfos.get(dbInfoId).destroy();
    this.m_dbInfos.delete(dbInfoId);
  }

  destroy(): void {
    this.m_dbInfos.forEach((value, key) => {
      value.destroy();
    });
    this.m_dbInfos.clear();
  }
}

export class DatabaseInfo {
  private m_id: number;
  private m_db: DatabaseSqlite3;
  private m_stmtInfos: Map<number, StatementInfo>;
  private m_nextStmtId: number;
  m_eventListenerCounters: { [key: string]: number };

  constructor(db: DatabaseSqlite3 = null) {
    this.m_id = -1;
    this.m_db = db;
    this.m_stmtInfos = new Map<number, StatementInfo>();
    this.m_nextStmtId = -1;
    this.m_eventListenerCounters = { trace: 0, profile: 0 }; // listener counters
  }

  type() {
    return this.constructor.name;
  }

  db() {
    return this.m_db;
  }

  id() {
    return this.m_id;
  }

  assignId() {
    g_nextDbId++;
    this.m_id = g_nextDbId;
  }

  genStmtId() {
    this.m_nextStmtId++;
    return this.m_nextStmtId;
  }

  addStmtInfo(stmtInfo: StatementInfo) {
    this.m_stmtInfos.set(stmtInfo.id(), stmtInfo);
  }

  getStmtInfo(stmtInfoId: number): StatementInfo {
    return this.m_stmtInfos.get(stmtInfoId);
  }

  removeStmtInfo(stmtInfoId: number) {
    this.m_stmtInfos.delete(stmtInfoId);
  }

  destroy() {
    this.closeDb();
    delete this.m_db;
  }

  closeDb() {
    // Try to close the DB, if it is still open, before deleting this DatabaseInfo
    this.m_stmtInfos.forEach((value, key) => {
      value.closeStmt();
    });
    this.m_stmtInfos.clear();

    this.m_db.close((err) => {
      if (err) {
        // DB has already been closed.
      }
    });
  }
}

export interface EncodedArrayBuffer {
  arrayBuffer: string;
}

export type Param = string | number | Buffer | EncodedArrayBuffer;

interface UserDataDatabase {
  filename: string;
  id: number;
  params: Param[];
}

class DatabaseUtils {
  static parseUserData(paramsArray: Param[]): UserDataDatabase {
    if (!(isString(paramsArray[0]) && isNumber(paramsArray[1]))) {
      throw new SQLiteError('Error: Invalid data received from client');
    }

    let data: UserDataDatabase = {
      filename: paramsArray[0] as string,
      id: paramsArray[1] as number,
      params: paramsArray.slice(2),
    };

    restoreBufferFromArrayBufferInString(data.params);

    return data;
  }

  static printLog(str) {
    debug.database(str);
  }
}

export const dbManager = new DatabaseManager();
exports.DatabaseUtils = DatabaseUtils;

exports.ws = (function () {
  return {
    ping(ctx: Context, param: Param[]) {
      DatabaseUtils.printLog(`ping: ${param}`);
      ctx.response.write(`pong: ${param}`);
    },

    verbose(ctx: Context) {
      dbManager.verbose();
      ctx.response.write(
        new DatabasePacket(new DatabaseInfo()),
      );
    },

    async versionNumber(ctx: Context, params: Param[]) {
      try {
        let ver: number = sqlite3.VERSION_NUMBER;
        ctx.response.write(new DatabasePacket(new DatabaseInfo(), ver));
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async on(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);
        const schema = Joi.array()
          .length(1)
          .items(Joi.string().allow('trace', 'profile').required());
        const result = schema.validate(data.params);

        if (result.error) {
          throw new SQLiteError(result.error, new DatabaseInfo());
        }

        let event = data.params[0] as string;
        let dbInfo = dbManager.databaseInfo(ctx, data.id);
        let db = dbInfo.db();

        if (dbInfo.m_eventListenerCounters[event] == 0) {
          if (event == 'trace') {
            db.on(event, function (query) {
              let data = { event, query };
              ctx.response.write(new DatabasePacket(dbInfo, data));
            });
          } else if (event == 'profile') {
            db.on(event, function (query, time) {
              let data = { event, query, time };
              ctx.response.write(new DatabasePacket(dbInfo, data));
            });
          }
        }

        dbInfo.m_eventListenerCounters[event]++;
        ctx.response.write(new DatabasePacket(dbInfo));
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    removeListener(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);
        const schema = Joi.array()
          .length(1)
          .items(Joi.string().allow('trace', 'profile').required());
        const result = schema.validate(data.params);

        if (result.error) {
          throw new SQLiteError(result.error, new DatabaseInfo());
        }

        let event = data.params[0] as string;
        let dbInfo = dbManager.databaseInfo(ctx, data.id);
        let db = dbInfo.db();

        dbInfo.m_eventListenerCounters[event]--;
        if (dbInfo.m_eventListenerCounters[event] == 0) {
          db.removeAllListeners(event);
        }

        ctx.response.write(new DatabasePacket(dbInfo));
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    removeAllListeners(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);
        const schema = Joi.array()
          .length(1)
          .items(Joi.string().allow('trace', 'profile').required());
        const result = schema.validate(data.params);

        if (result.error) {
          throw new SQLiteError(result.error, new DatabaseInfo());
        }

        let event = data.params[0] as string;
        let dbInfo = dbManager.databaseInfo(ctx, data.id);
        let db = dbInfo.db();
        db.removeAllListeners(event);
        dbInfo.m_eventListenerCounters[event] = 0;
        ctx.response.write(new DatabasePacket(dbInfo));
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async configure(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);
        const schema = Joi.array()
          .length(2)
          .items(
            Joi.string().allow('busyTimeout').required(),
            Joi.number().required(),
          );
        const result = schema.validate(data.params);

        if (result.error) {
          throw new SQLiteError(result.error, new DatabaseInfo());
        }

        let [option, value] = data.params;

        let dbInfo = dbManager.databaseInfo(ctx, data.id);
        let db = dbInfo.db();

        if (option == 'busyTimeout') {
          db.configure(option, value);
          ctx.response.write(new DatabasePacket(dbInfo));
        }
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    open(ctx: Context, params: Param[]): Promise<void> {
      let data;
      try {
        data = DatabaseUtils.parseUserData(params);
      } catch (err) {
        ctx.response.error(err);
        return new Promise((resolve, reject) => { reject(); });
      }

      const schema = Joi.array().length(1).items(Joi.number().required());
      const result = schema.validate(data.params);

      if (result.error) {
        ctx.response.error(
          new SQLiteError(result.error, new DatabaseInfo(data.filename)),
        );
        return new Promise((resolve, reject) => { reject(); });
      }

      let [mode] = data.params;

      return dbManager
        .open(ctx, data.filename, data.id, mode)
        .then((dbInfo) => {
          ctx.response.write(new DatabasePacket(dbInfo));
        })
        .catch((err) => {
          if (!SQLiteError.isError(err)) {
            throw err;
          }
          ctx.response.error(
            new SQLiteError(err, new DatabaseInfo(data.filename)),
          );
        });
    },

    async close(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.close((err) => {
            if (err) {
              reject(new SQLiteError(err, dbInfo));
              return;
            }

            ctx.response.write(new DatabasePacket(dbInfo));
            dbManager.deleteDatabaseInfo(ctx, data.id);
            resolve(undefined);
          });
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async wait(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);
        let dbInfo = dbManager.databaseInfo(ctx, data.id);
        let packet = await new Promise((resolve, reject) => {
          dbInfo.db().wait(function (err) {
            err
              ? reject(new SQLiteError(err, dbInfo))
              : resolve(new DatabasePacket(dbInfo));
          });
        });

        ctx.response.write(packet);
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async run(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.run(...data.params, function (err) {
            if (err) {
              reject(new SQLiteError(err, dbInfo));
              return;
            }

            // debug.database(
            //   `${ctx.request.payload.method} >> ${data.params} >> lastID: ${this.lastID} changes: ${this.changes}`,
            // );

            // statement id is null, as this statment is a tmp statement,
            // we do not need to make an instance and keep track of it.
            let stmtInfo = new StatementInfo(dbInfo.id(), null, this);
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async get(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.get(...data.params, function (err, rows) {
            if (err) {
              reject(new SQLiteError(err, dbInfo));
              return;
            }

            if (rows == undefined) {
              rows = {};
            }

            ctx.response.write(new DatabasePacket(dbInfo, rows));
            resolve(undefined);
          });
        }).catch((err) => {
          throw err;
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async all(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.all(...data.params, function (err, rows) {
            if (err) {
              reject(new SQLiteError(err, dbInfo));
              return;
            }

            ctx.response.write(new DatabasePacket(dbInfo, rows));
            resolve(undefined);
          });
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async each(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.each(
            ...data.params,
            function (err, row) {
              if (err) {
                reject(new SQLiteError(err, dbInfo));
                return;
              }

              ctx.response.write(
                new DatabasePacket(dbInfo, row, true),
              );
            },
            function (err, rowCount) {
              if (err) {
                reject(new SQLiteError(err, dbInfo));
                return;
              }

              ctx.response.write(new DatabasePacket(dbInfo, rowCount));
              resolve(undefined);
            },
          );
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async exec(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.exec(...data.params, function (err) {
            if (err) {
              reject(new SQLiteError(err, dbInfo));
              return;
            }

            ctx.response.write(new DatabasePacket(dbInfo));
            resolve(undefined);
          });
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },

    async prepare(ctx: Context, params: Param[]) {
      try {
        let data = DatabaseUtils.parseUserData(params);

        await new Promise((resolve, reject) => {
          let dbInfo = dbManager.databaseInfo(ctx, data.id);
          let db = dbInfo.db();
          db.prepare(...data.params, function (err) {
            let stmt = this;
            let stmtId = dbInfo.genStmtId();
            if (stmtId < 0) {
              reject(new SQLiteError('Too many statements'));
              return;
            }

            if (err) {
              reject(new SQLiteError(err, new StatementInfo(dbInfo.id(), stmtId, null)));
              return;
            }

            let stmtInfo = new StatementInfo(dbInfo.id(), stmtId, stmt);
            dbInfo.addStmtInfo(stmtInfo);
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });
      } catch (err) {
        if (!SQLiteError.isError(err)) {
          throw err;
        }
        ctx.response.error(err);
      }
    },
  };
})();
