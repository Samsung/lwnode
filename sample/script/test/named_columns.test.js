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

describe('named columns', function () {
  var db;

  before(async function () {
    db = await new sqlite3.Database(':memory:');
  });

  it('+ve: should create the table', async function () {
    // NOTE: added assert(), as the original TC does not check for validity
    await db.run("CREATE TABLE foo (txt TEXT, num INT)").then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: create a duplicate table', async function () {
    await db.run("CREATE TABLE foo (txt TEXT, num INT)").then(() => {
      assert(false);
    }).catch((err) => {
      assert.equal(err.message, 'SQLITE_ERROR: table foo already exists');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    });
  });

  it('-ve: custom: incorrect column type', async function () {
    try {
      await db.run("CREATE TABLE foo1 (txt TEXT1, num INT1)");
      assert(false);
    } catch (err) {
      assert.equal(err.name, 'AssertionError');
    }
  });

  it('+ve: should insert a value', async function () {
    // NOTE: added assert(), as the original TC does not check for validity
    await db.run("INSERT INTO foo VALUES($text, $id)", {
      $id: 1,
      $text: "Lorem Ipsum"
    }).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: incorrect number of parameters', async function () {
    try {
      await db.run("INSERT INTO foo VALUES($text, $id, $xxx)", {
        $id: 1,
        $text: "Lorem Ipsum"
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: table foo has 2 columns but 3 values were supplied');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should retrieve the values', async function () {
    await db.get("SELECT txt, num FROM foo ORDER BY num").then((row) => {
      assert.equal(row.txt, "Lorem Ipsum");
      assert.equal(row.num, 1);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: retrieve values using incorrect column name', async function () {
    try {
      await db.get("SELECT txt1, num FROM foo ORDER BY num");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: txt1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve values using incorrect number of columns', async function () {
    try {
      await db.get("SELECT txt, num, xxx FROM foo ORDER BY num");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: xxx');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve values using incorrect order by', async function () {
    try {
      await db.get("SELECT txt, num FROM foo ORDER BY num1");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: num1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve values from an incorrect table', async function () {
    try {
      await db.get("SELECT txt, num FROM foo2 ORDER BY num");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo2');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should be able to retrieve rowid of last inserted value', async function () {
    await db.get("SELECT last_insert_rowid() as last_id FROM foo").then((row) => {
      assert.equal(row.last_id, 1);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: incorrect function call in select stmt', async function () {
    try {
      await db.get("SELECT last_insert_rowid1() as last_id FROM foo");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such function: last_insert_rowid1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async function () { await db.close(); });
});
