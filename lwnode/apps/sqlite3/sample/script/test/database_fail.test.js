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

describe('error handling', function () {
  var db;

  before(async () => {
    db = await new sqlite3.Database(':memory:');
  });

  // NOTE: unrelevant TC
  // it('throw when calling Database() without new', function() {
  //     assert.throws(function() {
  //         sqlite3.Database(':memory:');
  //     }, (/Class constructors cannot be invoked without 'new'/));

  //     assert.throws(function() {
  //         sqlite3.Statement();
  //     }, (/Class constructors cannot be invoked without 'new'/));
  // });

  it('-ve: should error when calling Database#get on a missing table', async () => {
    try {
      let row = await db.get('SELECT id, txt FROM foo');
      // should not be here
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#all prepare fail', async () => {
    try {
      await db.all('SELECT id, txt FROM foo');
      // should not be here
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#run prepare fail', async () => {
    try {
      await db.run('SELECT id, txt FROM foo');
      // should not be here
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#each prepare fail', async () => {
    try {
      let num = await db.each('SELECT id, txt FROM foo', (row) => {
        assert.ok(false, "this should not be called");
      });
      // should not be here
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#each prepare fail without completion handler', async () => {
    try {
      let num = await db.each('SELECT id, txt FROM foo', (row) => {
        new Error('Completed query without error, but expected error');
      });
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#get prepare fail with param binding', async () => {
    try {
      let row = await db.get('SELECT id, txt FROM foo WHERE id = ?', 1);
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#all prepare fail with param binding', async () => {
    try {
      let rows = await db.all('SELECT id, txt FROM foo WHERE id = ?', 1);
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#run prepare fail with param binding', async () => {
    try {
      await db.run('SELECT id, txt FROM foo WHERE id = ?', 1);
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#each prepare fail with param binding', async () => {
    try {
      let num = await db.each('SELECT id, txt FROM foo WHERE id = ?', 1, (row) => {
        assert.ok(false, "this should not be called");
      });
      new Error('Completed query without error, but expected error');
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: Database#each prepare fail with param binding without completion handler', async () => {
    try {
      let num = await db.each('SELECT id, txt FROM foo WHERE id = ?', 1, (row) => {
        new Error('Completed query without error, but expected error');
      });
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async () => { await db.close(); });
});
