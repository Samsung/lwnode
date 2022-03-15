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

var assert = require('assert');
var helper = require('./support/helper');
helper.loadSqlite3();

if (process.env.NODE_SQLITE3_JSON1 === 'no') {
  describe('json', function () {
    it('+ve: skips JSON tests when --sqlite=/usr (or similar) is tested', function () {
    });
  });
} else {
  describe('json', function () {
    var db;

    before(async () => {
      db = await new sqlite3.Database(':memory:');
    });

    it('+ve: should select JSON', async () => {
      await db.run('SELECT json(?)', JSON.stringify({ ok: true }));
    });

    it('-ve: custom: incorrect parameter', async () => {
      try {
        await db.run('SELECT json(??)', JSON.stringify({ ok: true }));
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: near "?": syntax error');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    it('-ve: custom: incorrect function call', async () => {
      try {
        await db.run('SELECT json1(?)', JSON.stringify({ ok: true }));
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such function: json1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    after(async () => { await db.close(); });
  });
}
