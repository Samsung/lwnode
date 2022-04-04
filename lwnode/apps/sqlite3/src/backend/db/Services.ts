/*
 * Copyright 2022-present Samsung Electronics Co., Ltd. and other contributors
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

import { Knex } from 'knex';
const debug = require('debug')('api');

const tableName = 'services';

export type ServiceData = {
  name: string;
  path: string;
};

export class ServicesDB {
  private knex: Knex;

  constructor(knex: Knex) {
    this.knex = knex;
  }

  private ensureTable() {
    return this.knex.schema.hasTable(tableName).then((exists) => {
      if (!exists) {
        return this.knex.schema.createTable(tableName, (table) => {
          table.string('name').primary();
          table.string('path').unique();
        })
          .then(() => {
            debug(`${tableName} table is created`);
          });
      }
    });
  }

  async create(data: ServiceData) {
    await this.ensureTable();
    return this.knex(tableName).insert({
      ...data
    });
  }

  async getDataAll(): Promise<ServiceData[]> | undefined {
    await this.ensureTable();
    return this.knex(tableName).select('*');
  }

  async getDataByName(name: string): Promise<ServiceData> | undefined {
    await this.ensureTable();
    return this.knex(tableName)
      .where('name', name)
      .then((result: Array<ServiceData>) => {
        if (result.length) {
          return result[0];
        }
      });
  }
}
