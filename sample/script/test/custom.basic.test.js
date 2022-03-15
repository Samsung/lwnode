/*
 * Copyright 2021-present Samsung Electronics Co., Ltd.
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

function Logger(str) {
  // console.log(str);
}

describe('basic usage', function() {
  let db;

  it('+ve: verbose', async () => {
    try {
      await sqlite3.verbose();
    } catch (err) {
      throw err;
    }
  });

  it('+ve: open a database', async () => {
    try {
      db = await new sqlite3.Database(':memory:', sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: trace', async () => {
    try {
      await db.on('trace', (query) => {
        Logger(`trace: ${query}`);
      });
    } catch (err) {
      throw err;
    }
  });

  it('+ve: profile', async () => {
    try {
      await db.on('profile', (query, time) => {
        Logger(`profile: ${query}, time: ${time}`);
      });
    } catch (err) {
      throw err;
    }
  });

  it('+ve: busyTimeout', async () => {
    try {
      await db.configure('busyTimeout', 0);
    } catch (err) {
      throw err;
    }
  });

  it('+ve: create a table', async () => {
    const q = `CREATE TABLE user (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      name text,
      email text UNIQUE,
      password text,
      CONSTRAINT email_unique UNIQUE (email)
      );`;
    try {
      await db.run(q);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: incorrect create a table stmt', async () => {
    const q = `CREATE TABLE1 user (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      name text,
      email text UNIQUE,
      password text,
      CONSTRAINT email_unique UNIQUE (email)
      );`;
    try {
      await db.run(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "TABLE1": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: insert to the table', async () => {
    const q = "INSERT INTO user (name, email, password) VALUES (?,?,?)";
    const params1 = ["admin1", "admin@example.com", "admin123456"]; // need md5();
    const params2 = ["user1", "user@example.com", "user123456"]; // need md5();
    try {
      await db.run(q, params1);
      await db.run(q, params2);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: insert to an unknown table', async () => {
    const q = "INSERT INTO user1 (name, email, password) VALUES (?,?,?)";
    const params1 = ["admin1", "admin@example.com", "admin123456"]; // need md5();
    const params2 = ["user1", "user@example.com", "user123456"]; // need md5();
    try {
      await db.run(q, params1);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: select from the table: all', async () => {
    const q = "select * from user";
    try {
      let rows = await db.all(q);
      assert(rows.length === 2);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: select from an unknown table: all', async () => {
    const q = "select * from user1";
    try {
      let rows = await db.all(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: select from the table: get', async () => {
    const q = "select name from user";
    try {
      let row1 = await db.get(q);
      assert(row1.name === 'admin1');

      let row2 = await db.get(q);
      assert(row2.name === 'admin1');
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: select from an unknown table: get', async () => {
    const q = "select name from user1";
    try {
      let row1 = await db.get(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: select from an unknown column: get', async () => {
    const q = "select name1 from user";
    try {
      let row1 = await db.get(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: name1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: select from the table: each', async () => {
    const q = "select * from user";
    try {
      let rowCount = await db.each(q, (row)=>{
        assert(row);
      });
      assert(rowCount === 2);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: select from an unknown table: each', async () => {
    const q = "select * from user1";
    try {
      let rowCount = await db.each(q, (row)=>{
        assert(row);
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: select from the table: exec', async () => {
    const q = "select * from user";
    try {
      await db.exec(q);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: select from an unknown table: exec', async () => {
    const q = "select * from user1";
    try {
      await db.exec(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: statement 1', async () => {
    const q = "select * from user";
    try {
      let stmt = await db.prepare(q);;
      await stmt.run();
      await stmt.reset();

      let row = await stmt.get();
      assert(row);

      let rows = await stmt.all();
      assert(rows.length === 2);

      let rowCount = await stmt.each((row) => {
        assert(row);
      });
      assert(rowCount === 2);
      await stmt.finalize();
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: statement 1 from an unknown table', async () => {
    const q = "select * from user1";
    try {
      let stmt = await db.prepare(q);;
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });


  it('+ve: statement 2', async () => {
    const q = "select name from user where name = $n";
    try {
      let stmt = await db.prepare(q);
      await stmt.bind('admin1');

      let row = await stmt.get();
      assert(row.name === 'admin1');

      let rows = await stmt.all();
      assert(rows.length === 1);

      let rowCount = await stmt.each((row) => {
        assert(row);
      });
      assert(rowCount === 1);

      await stmt.run();
      await stmt.reset();

      await stmt.bind('user1');
      rowCount = await stmt.each((row) => {
        assert(row);
      });
      assert(rowCount === 1);

      await stmt.finalize();
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: statement 2 from an unknown table', async () => {
    const q = "select name from user1 where name = $n";
    try {
      let stmt = await db.prepare(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: user1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: statement 2 from an unknown column', async () => {
    const q = "select name1 from user where name = $n";
    try {
      let stmt = await db.prepare(q);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: name1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async () => {
    try {
      await db.close();
    } catch (err) {
      throw err;
    }
  });
});
