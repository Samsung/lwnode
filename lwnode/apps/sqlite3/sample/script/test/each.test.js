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

describe('each', function () {
  var db;

  before(async () => {
    let filename = 'big.db';
    db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
  });

  let total = 100000;
  if (helper.isBrowser) {
    total = 10000;
  }

  it(`+ve: retrieve ${total} rows with Statement#each`, async () => {
    var retrieved = 0;

    try {
      let num = await db.each('SELECT id, txt FROM foo LIMIT 0, ?', total, (row) => {
        retrieved++;
        if (retrieved === total) {
          assert.equal(retrieved, total, "Only retrieved " + retrieved + " out of " + total + " rows.");
        }
      });
    } catch (err) {
      throw err;
    }
  });

  it('+ve: Statement#each with complete callback', async () => {
    var retrieved = 0;

    try {
      let num = await db.each('SELECT id, txt FROM foo LIMIT 0, ?', total, (row) => {
        retrieved++;
      });
      assert.equal(retrieved, num);
      assert.equal(retrieved, total, "Only retrieved " + retrieved + " out of " + total + " rows.");
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: incorrect database filename', async () => {
    let filename = 'big1.db';
    try {
      let db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
      assert(false);
    } catch (err) {
      assert(true);
    }
  });

  it('-ve: custom: retrieve rows from an incorrect table: with Statement#each', async () => {
    try {
      let num = await db.each('SELECT id, txt FROM foo1 LIMIT 0, ?', total, (row) => {
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve rows from an incorrect column: with Statement#each', async () => {
    try {
      let num = await db.each('SELECT id1, txt FROM foo LIMIT 0, ?', total, (row) => {
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: id1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async function () { await db.close(); });
});
