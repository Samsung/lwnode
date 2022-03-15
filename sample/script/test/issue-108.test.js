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

describe('buffer', function () {
  var db;

  it('+ve: should insert blobs', async () => {
    db = await new sqlite3.Database(':memory:');

    await db.run("CREATE TABLE lorem (info BLOB)");
    var stmt = await db.prepare("INSERT INTO lorem VALUES (?)");

    // The API doc does not specify what kind of events that we need to support
    // await stmt.on('error', function (err) {
    //   throw err;
    // });

    var buff = new ArrayBuffer(2);
    await stmt.run(buff);
    await stmt.finalize();
    await db.close();
  });

  it('-ve: custom: incorrect parameter', async () => {
    try {
    let db = await new sqlite3.Database(':memory:');
    await db.run("CREATE TABLE lorem (info BLOB)");
    var stmt = await db.prepare("INSERT INTO lorem VALUES (??)");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "?": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: incorrect table', async () => {
    try {
    let db = await new sqlite3.Database(':memory:');
    await db.run("CREATE TABLE lorem (info BLOB)");
    var stmt = await db.prepare("INSERT INTO lorem1 VALUES (??)");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "?": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: incorrect sql', async () => {
    try {
    let db = await new sqlite3.Database(':memory:');
    await db.run("CREATE TABLE lorem (info BLOB)");
    var stmt = await db.prepare("INSERT1 INTO lorem VALUES (?)");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "INSERT1": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });
});
