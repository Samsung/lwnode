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
import config from '../config';
import crypto from 'crypto';

const { privateKey: secret } = config.db;

export function encryptPassword(password: string) {
  return crypto.createHmac('sha256', secret).update(password).digest('hex');
}

export interface User {
  email: string;
  password: string;
  displayName: string;
}

class Users {
  private knex: Knex;

  constructor(knex: Knex) {
    this.knex = knex;
  }

  private async ensureTable() {
    let that = this;

    return this.knex.schema.hasTable('users').then(function (exists) {
      if (!exists) {
        // one user to many social ids
        return that.knex.schema
          .createTable('users', (table) => {
            table.increments('id').primary();
            table.string('email').unique();
            table.string('password'); // todo: should be encrypted
            table.string('displayName');
          })
          .createTable('socials', (table) => {
            table.increments('id').primary();
            table.string('provider');
            table.string('accessToken');
            table.integer('user_id').unsigned().references('users.id');
          })
          .then(() => {
            console.log('table is created');
          });
      }
    });
  }

  async create(user: User) {
    await this.ensureTable();
    return this.knex('users').insert(user);
  }

  // read
  async getByEmail(email: string) {
    await this.ensureTable();
    return this.knex('users')
      .where('email', email)
      .then((result) => result[0]);
  }

  async update(user: User) {
    await this.ensureTable();
    return this.knex('users').where('email', user.email).update(user);
  }

  async delete(email: string) {
    await this.ensureTable();
    return this.knex('users').where('email', email).del();
  }

  validatePassword(user: User, password: string) {
    // note: consider returning a User instance including this function
    return user.password === encryptPassword(password);
  }
}

export default Users;
