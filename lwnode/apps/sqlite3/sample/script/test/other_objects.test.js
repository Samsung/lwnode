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

describe('data types', function () {
  var db;
  before(async function () {
    db = await new sqlite3.Database(':memory:');
    await db.run("CREATE TABLE txt_table (txt TEXT)");
    await db.run("CREATE TABLE int_table (int INTEGER)");
    await db.run("CREATE TABLE flt_table (flt FLOAT)");
  });

  beforeEach(async function () {
    await db.exec('DELETE FROM txt_table; DELETE FROM int_table; DELETE FROM flt_table;');
  });

  it('+ve: should serialize Date()', async function () {
    var date = new Date();
    try {
      await db.run("INSERT INTO int_table VALUES(?)", date);
      let row = await db.get("SELECT int FROM int_table");
      assert.equal(row.int, +date);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should serialize RegExp()', async function () {
    var regexp = /^f\noo/;
    try {
      await db.run("INSERT INTO txt_table VALUES(?)", regexp);
      let row = await db.get("SELECT txt FROM txt_table");
      assert.equal(row.txt, String(regexp));
    } catch (err) {
      throw err;
    }
  });

  [
    4294967296.249,
    Math.PI,
    3924729304762836.5,
    new Date().valueOf(),
    912667.394828365,
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (flt) {
    it('+ve: should serialize float ' + flt, async function () {
      try {
        await db.run("INSERT INTO flt_table VALUES(?)", flt);
        let row = await db.get("SELECT flt FROM flt_table");
        assert.equal(row.flt, flt);
      } catch (err) {
        throw err;
      }
    });
  });

  [
    4294967296.249,
    Math.PI,
    3924729304762836.5,
    new Date().valueOf(),
    912667.394828365,
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (flt) {
    it('-ve: custom: cannot insert a float using incorrect ? parameter ' + flt, async function () {
      try {
        await db.run("INSERT INTO flt_table VALUES(??)", flt);
        let row = await db.get("SELECT flt FROM flt_table");
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: near "?": syntax error');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });
  });

  [
    4294967296.249,
    Math.PI,
    3924729304762836.5,
    new Date().valueOf(),
    912667.394828365,
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (flt) {
    it('-ve: custom: cannot insert a float to an unknown table ' + flt, async function () {
      try {
        await db.run("INSERT INTO flt_table1 VALUES(?)", flt);
        let row = await db.get("SELECT flt FROM flt_table");
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such table: flt_table1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });
  });

  [
    4294967299,
    3924729304762836,
    new Date().valueOf(),
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (integer) {
    it('+ve: should serialize integer ' + integer, async function () {
      try {
        await db.run("INSERT INTO int_table VALUES(?)", integer);
        let row = await db.get("SELECT int AS integer FROM int_table");
        assert.equal(row.integer, integer);
      } catch (err) {
        throw err;
      }
    });
  });

  [
    4294967299,
    3924729304762836,
    new Date().valueOf(),
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (integer) {
    it('-ve: custom: cannot retrieve a value from an incorrect column ' + integer, async function () {
      try {
        await db.run("INSERT INTO int_table VALUES(?)", integer);
        let row = await db.get("SELECT int1 AS integer FROM int_table");
        assert();
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such column: int1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });
  });

  [
    4294967299,
    3924729304762836,
    new Date().valueOf(),
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (integer) {
    it('-ve: custom: cannot retrieve a value from an unknown table ' + integer, async function () {
      try {
        await db.run("INSERT INTO int_table VALUES(?)", integer);
        let row = await db.get("SELECT int AS integer FROM int_table1");
        assert();
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such table: int_table1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });
  });

  [
    4294967299,
    3924729304762836,
    new Date().valueOf(),
    2.3948728634826374e+83,
    9.293476892934982e+300,
    Infinity,
    -9.293476892934982e+300,
    -2.3948728634826374e+83,
    -Infinity
  ].forEach(function (integer) {
    it('-ve: custom: cannot retrieve a value using an incorrect sql stmt ' + integer, async function () {
      try {
        await db.run("INSERT INTO int_table VALUES(?)", integer);
        let row = await db.get("SELECT1 int AS integer FROM int_table");
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: near "SELECT1": syntax error');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });
  });

  after(async function () { await db.close(); });
});
