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

const debug = require('debug')('packet');
import type { DatabaseInfo } from './database';
import { StatementSqlite3, StatementInfo } from './statement';
import { Response } from '../response';
import { ab2str, isObject, serializeBufferToArrayBuffer } from '../utils';

export class SQLPacket {
  method;
  hasMoreData?: boolean;

  constructor(hasMoreData: boolean = undefined) {
    this.method = Response.option.AutoFill;
    this.hasMoreData = hasMoreData;
  }

  data() {
    return this.data;
  }
}

export class DatabasePacket extends SQLPacket {
  constructor(dbInfo: DatabaseInfo, data = undefined, hasMoreData: boolean = false) {
    super(hasMoreData);

    debug(
      `${this.constructor.name}, ${dbInfo}, ${data}, ${hasMoreData}`,
    );

    // data is a return value for a db operation.
    // It can be of any type.
    if (Array.isArray(data)) {
      for (let i in data) {
        serializeBufferToArrayBuffer(data[i]);
      }
    } else {
      serializeBufferToArrayBuffer(data);
    }

    this.data = {
      // @ts-ignore
      m_dbInfo: dbInfo,
      m_data: data ? data : null,
    };
  }
}

export class StatementPacket extends SQLPacket {
  constructor(stmtInfo: StatementInfo, data = undefined, hasMoreData: boolean = false) {
    super(hasMoreData);

    debug(
      `${this.constructor.name}, ${stmtInfo}, ${data}, ${hasMoreData}`,
    );

    if (Array.isArray(data)) {
      for (let i in data) {
        serializeBufferToArrayBuffer(data[i]);
      }
    } else {
      serializeBufferToArrayBuffer(data);
    }

    let packetedStmtInfo = stmtInfo;

    // Because attribute of the stmt is enumerable, copy to remove this attribute.
    if (stmtInfo.stmt()) {
      packetedStmtInfo = new StatementInfo(stmtInfo.dbId(), stmtInfo.id(), null);
      let stmt: StatementSqlite3;
      stmt = { sql: stmtInfo.stmt().sql, lastID: stmtInfo.stmt().lastID, 
        changes: stmtInfo.stmt().changes };
      packetedStmtInfo.setStmt(stmt);
    }

    this.data = {
      // @ts-ignore
      m_stmtInfo: packetedStmtInfo,
      m_data: data ? data : null,
    };
  }
}
