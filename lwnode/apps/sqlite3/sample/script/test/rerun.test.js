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

describe('rerunning statements', function () {
  var db;

  before(async function () {
    db = await new sqlite3.Database(':memory:');
  });

  var count = 10;
  var inserted = 0;
  var retrieved = 0;

  it('+ve: should create the table', async function () {
    try {
      await db.run("CREATE TABLE foo (id int)");
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should insert repeatedly, reusing the same statement', async function () {
    try {
      var stmt = await db.prepare("INSERT INTO foo VALUES(?)");
      for (var i = 5; i < count; i++) {
        await stmt.run(i);
        inserted++;
      }
      await stmt.finalize();
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should retrieve repeatedly, resuing the same statement', async function () {
    try {
      var collected = [];
      var stmt = await db.prepare("SELECT id FROM foo WHERE id = ?");
      for (var i = 0; i < count; i++) {
        let row = await stmt.get(i);
        if (row) {
          collected.push(row);
        }
      }

      await stmt.finalize();
      retrieved += collected.length;
      assert.deepEqual(collected, [{ id: 5 }, { id: 6 }, { id: 7 }, { id: 8 }, { id: 9 }]);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should have inserted and retrieved the right amount', async function () {
    assert.equal(inserted, 5);
    assert.equal(retrieved, 5);
  });

  after(async function () { await db.close(); });
});
