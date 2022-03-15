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

let filename = 'prepare.db';

describe('prepare', function () {
  describe('invalid SQL', function () {
    var db;
    before(async () => { db = await new sqlite3.Database(':memory:'); });

    var stmt;
    it('-ve: should fail preparing a statement with invalid SQL', async () => {
      try {
        stmt = await db.prepare('CRATE TALE foo text bar)');
      } catch (err) {
        if (err && err.errno == sqlite3.ERROR &&
          err.message === 'SQLITE_ERROR: near "CRATE": syntax error') {
          assert(true);
        } else {
          throw err;
        }
      }
    });

    after(async () => { await db.close(); });
  });

  describe('simple prepared statement', function () {
    var db;
    before(async () => { db = await new sqlite3.Database(':memory:'); });

    it('+ve: should prepare, run and finalize the statement', async function () {
      // NOTE: chainings were split
      let stmt = await db.prepare("CREATE TABLE foo (text bar)");
      await stmt.run();
      await stmt.finalize();
    });

    after(async () => { await db.close(); });
  });

  describe('inserting and retrieving rows', function () {
    var db;
    before(async () => { db = await new sqlite3.Database(':memory:'); });

    var inserted = 0;
    var retrieved = 0;

    // We insert and retrieve that many rows.
    var count = 1000;
    if (helper.isTizen) {
      count = 100;
    }

    it('+ve: should create the table', async () => {
      let stmt = await db.prepare("CREATE TABLE foo (txt text, num int, flt float, blb blob)");
      await stmt.run();
      await stmt.finalize();
    });

    it('+ve: should insert ' + count + ' rows', async () => {
      let promises = [];
      for (var i = 0; i < count; i++) {
        let p = new Promise((resolve, reject) => {
          (function (i) {
            let stmt;
            db.prepare("INSERT INTO foo VALUES(?, ?, ?, ?)").then((stmt1) => {
              stmt = stmt1;
              return stmt.run('String ' + i, i, i * Math.PI);
            }).then(() => {
              inserted++;
              return stmt.finalize();
            }).then(() => {
              resolve();
            }).catch((err) => {
              reject(err);
            });
          })(i);
        });
        promises.push(p);
      }

      await Promise.all(promises);
      assert(inserted === count);
    });

    it('+ve: should prepare a statement and run it ' + (count + 5) + ' times', async () => {
      let stmt = null;
      try {
        stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo ORDER BY num");
        assert.equal(stmt.sql, 'SELECT txt, num, flt, blb FROM foo ORDER BY num');
      } catch (err) {
        throw err;
      }

      let promises = [];
      for (var i = 0; i < count + 5; i++) {
        let p = new Promise((resolve, reject) => {
          (function (i) {
            stmt.get().then((row) => {
              if (retrieved >= count) {
                assert.equal(row, undefined);
              } else {
                assert.equal(row.txt, 'String ' + i);
                assert.equal(row.num, i);
                assert.equal(row.flt, i * Math.PI);
                assert.equal(row.blb, null);
              }

              retrieved++;
              resolve();
            }).catch((err) => {
              reject(err);
            });
          })(i);
        });
        promises.push(p);
      }

      await Promise.all(promises);
      await stmt.finalize();
    });

    it('+ve: should have retrieved ' + (count + 5) + ' rows', function () {
      assert.equal(count + 5, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('inserting with accidental undefined', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(':memory:');
    });

    var inserted = 0;
    var retrieved = 0;

    it('+ve: should create the table', async function () {
      let stmt = await db.prepare("CREATE TABLE foo (num int)");
      await stmt.run();
      await stmt.finalize();
    });

    it('+ve: should insert two rows', async function() {
      let stmt = await db.prepare('INSERT INTO foo VALUES(4)');
      try {
        await stmt.run();
        inserted++;
      } catch (err) {
        throw err;
      }

      try {
        // The second time we pass undefined as a parameter. This is
        // a mistake, but it should either throw an error or be ignored,
        // not silently fail to run the statement.
        await stmt.run(); // undefined: NOTE: in this case, we throw an error if incorrect param is given
        inserted++;
        await stmt.finalize();
      } catch (err) {
        throw err;
      }

      assert.equal(inserted, 2);
    });

    it('+ve: should retrieve the data', async function () {
      var stmt = await db.prepare("SELECT num FROM foo");

      for (var i = 0; i < 2; i++) (async function (i) {
        let row = await stmt.get();
        assert(row);
        assert.equal(row.num, 4);
        retrieved++;
      })(i);

      await stmt.finalize();
    });

    it('-ve: custom: cannot retrieve data from an incorrect table', async function () {
      let stmt;
      try {
        stmt = await db.prepare("SELECT num FROM foo1");
        assert(false);
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    it('+ve: should have retrieved two rows', function () {
      assert.equal(2, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('retrieving reset() function', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    var retrieved = 0;

    it('+ve: should retrieve the same row over and over again', async function () {
      var stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo ORDER BY num");

      for (var i = 0; i < 10; i++) {
        await stmt.reset();
        let row = await stmt.get();
        assert.equal(row.txt, 'String 0');
        assert.equal(row.num, 0);
        assert.equal(row.flt, 0.0);
        assert.equal(row.blb, null);
        retrieved++;
      }
      await stmt.finalize();
    });

    it('+ve: should have retrieved 10 rows', function () {
      assert.equal(10, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('multiple get() parameter binding', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    var retrieved = 0;

    it('+ve: should retrieve particular rows', async function () {
      var stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo WHERE num = ?");

      for (var i = 0; i < 10; i++) (async function (i) {
        let row = await stmt.get(i * 10 + 1);
        var val = i * 10 + 1;
        assert.equal(row.txt, 'String ' + val);
        assert.equal(row.num, val);
        assert.equal(row.flt, val * Math.PI);
        assert.equal(row.blb, null);
        retrieved++;
      })(i);

      await stmt.finalize();
    });

    it('+ve: should have retrieved 10 rows', function () {
      assert.equal(10, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('prepare() parameter binding', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    var retrieved = 0;

    it('+ve: should retrieve particular rows', async function() {
      let stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo WHERE num = ? AND txt = ?", 10, 'String 10');
      let row = await stmt.get();
      assert.equal(row.txt, 'String 10');
      assert.equal(row.num, 10);
      assert.equal(row.flt, 10 * Math.PI);
      assert.equal(row.blb, null);
      retrieved++;
      await stmt.finalize();
    });

    it('+ve: should have retrieved 1 row', function () {
      assert.equal(1, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('all()', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    var retrieved = 0;
    var count = 1000;

    it('+ve: should retrieve particular rows', async function () {
      let stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo WHERE num < ? ORDER BY num", count);
      let rows = await stmt.all();
      for (var i = 0; i < rows.length; i++) {
        assert.equal(rows[i].txt, 'String ' + i);
        assert.equal(rows[i].num, i);
        assert.equal(rows[i].flt, i * Math.PI);
        assert.equal(rows[i].blb, null);
        retrieved++;
      }
      await stmt.finalize();
    });

    it('+ve: should have retrieved all rows', function () {
      assert.equal(count, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('all()', async function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    it('+ve: should retrieve particular rows', async function() {
      let stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo WHERE num > 5000");
      let rows = await stmt.all();
      assert.ok(rows.length === 0);
      await stmt.finalize();
    });

    after(async () => { await db.close(); });
  });

  describe('high concurrency', function () {
    var db;
    before(async () => { db = await new sqlite3.Database(':memory:'); });

    function randomString() {
      var str = '';
      for (var i = Math.random() * 300; i > 0; i--) {
        str += String.fromCharCode(Math.floor(Math.random() * 256));
      }
      return str;
    }

    // Generate random data.
    var data = [];
    var length = Math.floor(Math.random() * 1000) + 200;
    for (var i = 0; i < length; i++) {
      data.push([randomString(), i, i * Math.random(), null]);
    }

    var inserted = 0;
    var retrieved = 0;

    it('+ve: should create the table', async function () {
      let stmt = await db.prepare("CREATE TABLE foo (txt text, num int, flt float, blb blob)");
      await stmt.run();
      await stmt.finalize();
    });

    it('+ve: should insert all values', async function () {
      for (var i = 0; i < data.length; i++) {
        (async function (i) {
          var stmt = await db.prepare("INSERT INTO foo VALUES(?, ?, ?, ?)");
          await stmt.run(data[i][0], data[i][1], data[i][2], data[i][3]);
          inserted++;
          await stmt.finalize();
        })(i);
      }
    });

    it('+ve: should retrieve all values', async function() {
      let stmt = await db.prepare("SELECT txt, num, flt, blb FROM foo");
      let rows = await stmt.all();

      for (var i = 0; i < rows.length; i++) {
        assert.ok(data[rows[i].num] !== true);

        assert.equal(rows[i].txt, data[rows[i].num][0]);
        assert.equal(rows[i].num, data[rows[i].num][1]);
        assert.equal(rows[i].flt, data[rows[i].num][2]);
        assert.equal(rows[i].blb, data[rows[i].num][3]);

        // Mark the data row as already retrieved.
        data[rows[i].num] = true;
        retrieved++;
      }

      assert.equal(retrieved, data.length);
      assert.equal(retrieved, inserted);
      await stmt.finalize();
    });

    after(async () => { await db.close(); });
  });

  describe('test Database#get()', async function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
    });

    var retrieved = 0;

    it('+ve: should get a row', async function () {
      let row = await db.get("SELECT txt, num, flt, blb FROM foo WHERE num = ? AND txt = ?", 10, 'String 10');
      assert.equal(row.txt, 'String 10');
      assert.equal(row.num, 10);
      assert.equal(row.flt, 10 * Math.PI);
      assert.equal(row.blb, null);
      retrieved++;
    });

    it('+ve: should have retrieved all rows', function () {
      assert.equal(1, retrieved, "Didn't retrieve all rows");
    });

    after(async () => { await db.close(); });
  });

  describe('Database#run() and Database#all()', function () {
    var db;
    before(async () => {
      db = await new sqlite3.Database(':memory:');
    });

    var inserted = 0;
    var retrieved = 0;

    // We insert and retrieve that many rows.
    var count = 1000;

    it('+ve: should create the table', async () => {
      await db.run("CREATE TABLE foo (txt text, num int, flt float, blb blob)");
    });

    it('+ve: should insert ' + count + ' rows', async () => {
      for (var i = 0; i < count; i++) {
        try {
          await db.run("INSERT INTO foo VALUES(?, ?, ?, ?)",
                       'String ' + i,
                       i,
                       i * Math.PI
                       // null (SQLite sets this implicitly)
                       );
          inserted++;
        } catch (err) {
          throw err;
        }
      }

      if (inserted == count) {
        assert(true);
      } else {
        assert(false);
      }
    });

    it('+ve: should retrieve all rows', async function () {
      try {
        let rows = await db.all("SELECT txt, num, flt, blb FROM foo ORDER BY num");
        for (var i = 0; i < rows.length; i++) {
          assert.equal(rows[i].txt, 'String ' + i);
          assert.equal(rows[i].num, i);
          assert.equal(rows[i].flt, i * Math.PI);
          assert.equal(rows[i].blb, null);
          retrieved++;
        }

        assert.equal(retrieved, count);
        assert.equal(retrieved, inserted);
      } catch (err) {
        throw err;
      }
    });

    after(async () => { await db.close(); });
  });
});
