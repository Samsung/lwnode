/*
 * Copyright 2021-present Samsung Electronics Co., Ltd. and other contributors
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

describe('custom events', async function () {
  it('+ve: remove all trace events', async function () {
    var db = await new sqlite3.Database(':memory:');

    await db.on('trace', function (sql) {
      assert.ok(false);
    });

    await db.on('trace', function (sql) {
      assert.ok(false);
    });

    await db.removeAllListeners('trace');
    await db.run("CREATE TABLE foo (id int)");
    await db.close();
  });

  it('+ve: remove one trace event', async function () {
    var db = await new sqlite3.Database(':memory:');
    let counter = 0;

    var trace1 = function (sql) {
      counter++;
      assert.ok(false);
    };
    await db.on('trace', trace1);

    var trace2 = function (sql) {
      assert(counter == 0);
      counter++;
    };
    await db.on('trace', trace2);

    await db.removeListener('trace', trace1);
    await db.run("CREATE TABLE foo (id int)");
    await db.close();

    setTimeout((e) => {
      assert(counter == 1);
    }, 1000);
  });
});
