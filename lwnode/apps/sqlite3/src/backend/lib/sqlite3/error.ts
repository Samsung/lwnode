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

import { JSONRPCError } from '../rpcerror';
import type { DatabaseInfo, ErrorSqlite3 } from './database';
import type { StatementInfo } from './statement';
import { Response } from '../response';

export class SQLiteError extends JSONRPCError {
  code: number;
  type: string;
  method: string;
  filename: string;
  data: {
    m_dbInfo?;
    m_stmtInfo?;
    m_data;
  }

  // e: Error | string
  constructor(e: Error | string, info?: DatabaseInfo | StatementInfo, data?) {
    super(e);

    this.type = this.constructor.name;
    this.method = Response.option.AutoFill;

    // FIXME: Is e a sqlite error object?
    if (e instanceof Error) {
      // @ts-ignore
      this.code = e.code;
      // @ts-ignore
      this.errno = e.errno;
    }

    if (info) {
      let type = info.type();
      this.data = {
        m_dbInfo: type === 'DatabaseInfo' ? info : undefined,
        m_stmtInfo: type === 'StatementInfo' ? info : undefined,
        m_data: data,
      };
    }
  }

  static isError(e) {
    return (e.code && e.errno) || e instanceof SQLiteError;
  }
}
