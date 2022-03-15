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

describe('tracing', async function () {
  it('+ve: Database tracing', async function () {
    var db = await new sqlite3.Database(':memory:');
    var create = false;
    var select = false;
    try {
      await db.on('trace', function (sql) {
        if (sql.match(/^SELECT/)) {
          assert.ok(!select);
          assert.equal(sql, "SELECT * FROM foo");
          select = true;
        }
        else if (sql.match(/^CREATE/)) {
          assert.ok(!create);
          assert.equal(sql, "CREATE TABLE foo (id int)");
          create = true;
        }
        else {
          assert.ok(false);
        }
      });

      await db.run("CREATE TABLE foo (id int)");
      await db.run("SELECT * FROM foo");

      await db.close();

      setTimeout((e) => {
        assert.ok(create);
        assert.ok(select);
      }, 1000);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: test disabling tracing #1', async function () {
    var db = await new sqlite3.Database(':memory:');

    await db.on('trace', function (sql) {
      assert.ok(false);
    });
    await db.removeAllListeners('trace');
    // NOTE: we do not support adding an event handler to _events directly
    // db._events['trace'] = function (sql) {
    // };

    await db.run("CREATE TABLE foo (id int)");
    await db.close();
  });

  it('-ve: test disabling tracing #2', async function () {
    var db = await new sqlite3.Database(':memory:');

    var trace = function (sql) {
      assert.ok(false);
    };
    await db.on('trace', trace);
    await db.removeListener('trace', trace);
    // NOTE: we do not support adding an event handler to _events directly
    // db._events['trace'] = function (sql) {
    //   assert.ok(false);
    // };

    await db.run("CREATE TABLE foo (id int)");
    await db.close();
  });
});
