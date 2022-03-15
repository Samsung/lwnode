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

describe('profiling', function () {
  var create = false;
  var select = false;

  var db;
  before(async function () {
    db = await new sqlite3.Database(':memory:');

    await db.on('profile', function (sql, nsecs) {
      assert.ok(typeof nsecs === "number");
      if (sql.match(/^SELECT/)) {
        assert.ok(!select);
        assert.equal(sql, "SELECT * FROM foo");
        select = true;
      } else if (sql.match(/^CREATE/)) {
        assert.ok(!create);
        assert.equal(sql, "CREATE TABLE foo (id int)");
        create = true;
      } else {
        assert.ok(false);
      }
    });
  });

  it('+ve: should profile a create table', async function () {
    assert.ok(!create);
    try {
      await db.run("CREATE TABLE foo (id int)");
      setTimeout((e) => {
        assert.ok(create);
      }, 1000);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: should profile a select', async function () {
    assert.ok(!select);
    try {
      await db.run("SELECT * FROM foo");
      setTimeout((e) => {
        assert.ok(select);
      }, 1000);
    } catch (err) {
      throw err;
    }
  });

  after(async function () {
    await db.close();
  });
});
