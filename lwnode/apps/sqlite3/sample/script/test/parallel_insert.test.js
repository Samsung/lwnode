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

describe('parallel', function () {
  var db;
  let filename = 'test_parallel_inserts.db';
  before(async () => {
    await helper.deleteFile(filename);
    db = await new sqlite3.Database(helper.getStoreFilePath(filename));
  });

  var columns = [];
  for (var i = 0; i < 128; i++) {
    columns.push('id' + i);
  }

  it('+ve: should create the table', async () => {
    await db.run("CREATE TABLE foo (" + columns + ")");
  });

  let count = 1000;
  if (helper.isTizen) {
    count = 100;
  }

  it('+ve: should insert in parallel', async () => {
    for (var i = 0; i < count; i++) {
      for (var values = [], j = 0; j < columns.length; j++) {
        values.push(i * j);
      }
      db.run("INSERT INTO foo VALUES (" + values + ")");
    }

    await db.wait();
  });

  // @lwnode improve verification
  it('+ve: should retrieve the values inserted: each()', async () => {
    let rowCount = await db.each("select * from foo");
    assert(rowCount === count);
  });

  it('+ve: should retrieve the values inserted: all()', async () => {
    let rowCount = await db.all("select * from foo");
    assert(rowCount.length === count);
  });

  it('+ve: should close the database', async () => {
    await db.close();
  });

  it('+ve: should verify that the database exists', async () => {
    await assert.fileExists(filename);
  });

  after(async () => {
    await helper.deleteFile(filename);
  });
});
