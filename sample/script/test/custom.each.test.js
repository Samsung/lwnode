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

describe('custom each', function () {
  var db;

  before(async () => {
    db = await new sqlite3.Database(':memory:');
  });

  it('+ve: should create the table', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      await db.run("CREATE TABLE foo (txt TEXT, num INT)");
    } catch (err) {
      throw err;
    }
  });

  let userData = [
    {
      $id: 1,
      $text: "Lorem Ipsum"
    },
    {
      $id: 2,
      $text: "Lorem Ipsum2"
    }
  ];

  let resultData = [
    {
      num: 1,
      txt: "Lorem Ipsum"
    },
    {
      num: 2,
      txt: "Lorem Ipsum2"
    }
  ];

  it('+ve: should insert a value', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      await db.run("INSERT INTO foo VALUES($text, $id)", userData[0]);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should insert a value', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      await db.run("INSERT INTO foo VALUES($text, $id)", userData[1]);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: incorrect insert to an unknown table', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      await db.run("INSERT INTO foo1 VALUES($text, $id)", userData[1]);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: check table', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      let rows = await db.all("select * from foo");
      assert.deepStrictEqual(rows, resultData);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: running all() from an incorrect table', async () => {
    // NOTE: added assert(), as the original TC does not check for validity
    try {
      let rows = await db.all("select * from foo1");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should retrieve the values', async () => {
    let counter = 0;

    try {
      let rowCount = await db.each("SELECT * from foo", (row) => {
        assert.deepStrictEqual(row, resultData[counter]);
        counter++;
      });
      assert(rowCount, resultData.length);
    } catch (err) {
      throw err;
    };
  });

  it('-ve: custom: running each() from an incorrect table', async () => {
    let counter = 0;

    try {
      let rowCount = await db.each("SELECT * from foo1", (row) => {
        assert.deepStrictEqual(row, resultData[counter]);
        counter++;
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    };
  });

  it('+ve: should be able to retrieve rowid of last inserted value', async () => {
    let row = await db.get("SELECT last_insert_rowid() as last_id FROM foo");
    assert.equal(row.last_id, resultData.length);

  });

  it('-ve: custom: running get() from an incorrect table', async () => {
    try {
    let row = await db.get("SELECT last_insert_rowid() as last_id FROM foo1");
    assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: running get() from an incorrect function call', async () => {
    try {
    let row = await db.get("SELECT last_insert_rowid1() as last_id FROM foo");
    assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such function: last_insert_rowid1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async () => { await db.close(); });
});
