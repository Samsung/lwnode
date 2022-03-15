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

import type { DatabaseInfo } from "./database";
import { dbManager } from "./database";
import type { Context } from '../rpc';

import { log } from '../log';
import { SQLiteError } from './error';
import { StatementPacket } from './packet';
import { isNumber, restoreBufferFromArrayBufferInString } from "../utils";

export interface StatementSqlite3 { // Obtained from libsqlite
  sql: string;
  lastID?: number;
  changes?: number;

  bind?(...params);
  reset?(...params);
  finalize?(...params);
  run?(...params);
  get?(...params);
  all?(...params);
  each?(...params);
}

export class StatementInfo {
  private m_dbId: number;           // DB ID
  private m_id: number;             // Statement ID
  private m_stmt: StatementSqlite3;

  constructor(dbId: number, id: number, stmt: StatementSqlite3) {
    this.m_dbId = dbId;
    this.m_id = id;
    this.m_stmt = stmt;
  }

  dbId() {
    return this.m_dbId;
  }

  id() {
    return this.m_id;
  }

  stmt() {
    return this.m_stmt;
  }

  setStmt(stmt: StatementSqlite3) {
    this.m_stmt = stmt;
  }

  type() {
    return this.constructor.name;
  }

  closeStmt() {
    if (this.m_stmt) {
      this.m_stmt.finalize();
      this.m_stmt = null;
    }
  }
}

interface UserDataStatement {
  dbId: number; // Our own Database Id for a sqlite3.Database instance
  stmtInfo: {   // StatementInfo received from client in object format
    m_dbId: number;
    m_id: number;
    m_stmt;     // In simple object format
  };
  params: any[];
}

class StatementUtils {
  static parseUserData(paramsArray: any[]): UserDataStatement {
    if (!isNumber(paramsArray[0])) {
      throw new SQLiteError('Error: Invalid data received from client');
    }

    let data: UserDataStatement = {
      dbId: paramsArray[0],
      stmtInfo: paramsArray[1],
      params: paramsArray.slice(2),
    };

    restoreBufferFromArrayBufferInString(data.params);

    return data;
  }

  static printLog(str) {
    log.info(str);
  }
}

exports.ws = (function () {
  return {
    async bind(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo: StatementInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();
        await new Promise((resolve, reject) => {
          stmt.bind(...data.params, (err) => {
            if (err) {
              reject(new SQLiteError(err, stmtInfo));
              return;
            }
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async reset(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();
        await new Promise((resolve, reject) => {
          stmt.reset((err) => {
            // Always succeed
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async finalize(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();
        await new Promise((resolve, reject) => {
          stmt.finalize((err) => {
            if (err) {
              reject(new SQLiteError(err, stmtInfo));
              return;
            }
            dbInfo.removeStmtInfo(stmtId);
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async run(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();

        await new Promise((resolve, reject) => {
          stmt.run(...data.params, (err) => {
            if (err) {
              reject(new SQLiteError(err, stmtInfo));
              return;
            }
            ctx.response.write(new StatementPacket(stmtInfo));
            resolve(undefined);
          });
        });

      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async get(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();
        await new Promise((resolve, reject) => {
          stmt.get(...data.params, (err, row) => {
            if (err) {
              reject(new SQLiteError(err, stmtInfo));
              return;
            }
            ctx.response.write(new StatementPacket(stmtInfo, row));
            resolve(undefined);
          });
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async all(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();

        await new Promise((resolve, reject) => {
          stmt.all(...data.params, (err, rows) => {
            if (err) {
              reject(new SQLiteError(err, stmtInfo));
              return;
            }

            ctx.response.write(new StatementPacket(stmtInfo, rows));
            resolve(undefined);
          });
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },

    async each(ctx: Context, params) {
      try {
        let data = StatementUtils.parseUserData(params);
        let dbInfo = await dbManager.open(ctx, null, data.dbId);
        let stmtInfoReceived = data.stmtInfo;
        let stmtId = stmtInfoReceived.m_id;
        let stmtInfo = dbInfo.getStmtInfo(stmtId);

        if (!stmtInfo) {
          throw new SQLiteError('Error: No valid statement', stmtInfo);
        }

        let stmt = stmtInfo.stmt();
        await new Promise((resolve, reject) => {
          stmt.each(...data.params,
            (err, row) => {
              if (err) {
                reject(new SQLiteError(err, stmtInfo));
                return;
              }
              ctx.response.write(new StatementPacket(stmtInfo, row, true));
            },
            (err, rowCount) => {
              if (err) {
                reject(new SQLiteError(err, stmtInfo));
                return;
              }
              ctx.response.write(new StatementPacket(stmtInfo, rowCount));
              resolve(undefined);
            }
          );
        });
      } catch (error) {
        if (!SQLiteError.isError(error)) {
          throw error;
        }
        ctx.response.error(error);
      }
    },
  }
})();
