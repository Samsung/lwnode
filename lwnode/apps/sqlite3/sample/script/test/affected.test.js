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

var assert = require('assert');
var helper = require('./support/helper');
helper.loadSqlite3();

describe('query properties', function() {
    var db;

    before(async () => {
        db = await new sqlite3.Database(':memory:');
        await db.run("CREATE TABLE foo (id INT, txt TEXT)");
    });

    let stmt;
    it('+ve: should return the correct lastID', async () => {
        stmt = await db.prepare("INSERT INTO foo VALUES(?, ?)");
        var j = 1;
        for (var i = 0; i < 1000; i++) {
            let updated = await stmt.run(i, "demo");
            assert.equal(j++, updated.lastID);
        }
    });

    it('+ve: should return the correct changes count', async () => {
        let updated = await db.run("UPDATE foo SET id = id + 1 WHERE id % 2 = 0");
        assert.equal(500, updated.changes);
    });

    it('-ve: custom: incorrect insert to an unknown table', async () => {
      try {
        stmt = await db.prepare("INSERT INTO foo1 VALUES(?, ?)");
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    it('-ve: custom: incorrect update to an unknown table', async () => {
      try {
        let updated = await db.run("UPDATE foo1 SET id = id + 1 WHERE id % 2 = 0");
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    it('-ve: custom: incorrect update to an unknown column', async () => {
      try {
        let updated = await db.run("UPDATE foo SET id1 = id1 + 1 WHERE id1 % 2 = 0");
      } catch (err) {
        assert.equal(err.message, 'SQLITE_ERROR: no such column: id1');
        assert.equal(err.errno, sqlite3.ERROR);
        assert.equal(err.code, 'SQLITE_ERROR');
      }
    });

    after(async () => { await stmt.finalize(); await db.close(); });
});
