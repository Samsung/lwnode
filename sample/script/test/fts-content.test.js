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

describe('fts', function () {
  var db;
  before(async () => {
    db = await new sqlite3.Database(':memory:');
  });

  it('+ve: should create a new fts4 table', async () => {
    try {
      await db.exec('CREATE VIRTUAL TABLE t1 USING fts4(content="", a, b, c);');
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: incorrect use of fts4 table', async () => {
    try {
      await db.exec('CREATE VIRTUAL TABLE t2 USING fts4x(content="", a, b, c);');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such module: fts4x');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });
});
