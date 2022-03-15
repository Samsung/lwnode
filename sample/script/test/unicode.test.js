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

describe('unicode', function () {
  var first_values = [],
    trailing_values = [],
    chars = [],
    subranges = new Array(2),
    len = subranges.length,
    db,
    i;

  before(async function () {
    db = await new sqlite3.Database(':memory:');
  });

  for (i = 0x20; i < 0x80; i++) {
    first_values.push(i);
  }

  for (i = 0xc2; i < 0xf0; i++) {
    first_values.push(i);
  }

  for (i = 0x80; i < 0xc0; i++) {
    trailing_values.push(i);
  }

  for (i = 0; i < len; i++) {
    subranges[i] = [];
  }

  for (i = 0xa0; i < 0xc0; i++) {
    subranges[0].push(i);
  }

  for (i = 0x80; i < 0xa0; i++) {
    subranges[1].push(i);
  }

  function random_choice(arr) {
    return arr[Math.random() * arr.length | 0];
  }

  function random_utf8() {
    var first = random_choice(first_values);

    if (first < 0x80) {
      return String.fromCharCode(first);
    } else if (first < 0xe0) {
      return String.fromCharCode((first & 0x1f) << 0x6 | random_choice(trailing_values) & 0x3f);
    } else if (first == 0xe0) {
      return String.fromCharCode(((first & 0xf) << 0xc) | ((random_choice(subranges[0]) & 0x3f) << 6) | random_choice(trailing_values) & 0x3f);
    } else if (first == 0xed) {
      return String.fromCharCode(((first & 0xf) << 0xc) | ((random_choice(subranges[1]) & 0x3f) << 6) | random_choice(trailing_values) & 0x3f);
    } else if (first < 0xf0) {
      return String.fromCharCode(((first & 0xf) << 0xc) | ((random_choice(trailing_values) & 0x3f) << 6) | random_choice(trailing_values) & 0x3f);
    }
  }

  function randomString() {
    var str = '',
      i;

    for (i = Math.random() * 300; i > 0; i--) {
      str += random_utf8();
    }

    return str;
  }


  // Generate random data.
  var data = [];
  var length = Math.floor(Math.random() * 1000) + 200;
  for (var i = 0; i < length; i++) {
    data.push(randomString());
  }

  var inserted = 0;
  var retrieved = 0;

  it('+ve: should create the table', async function () {
    await db.run("CREATE TABLE foo (id int, txt text)");
  });

  it('-ve: custom: cannot create a duplicate table', async function () {
    try {
      await db.run("CREATE TABLE foo (id int, txt text)");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: table foo already exists');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should insert all values', async function () {
    var stmt = await db.prepare("INSERT INTO foo VALUES(?, ?)");
    for (var i = 0; i < data.length; i++) {
      await stmt.run([i, data[i]]).then(() => {
        inserted++;
      }).catch((err) => {
        throw err;
      });
    }
    await stmt.finalize();
  });

  it('+ve: should retrieve all values', async function () {
    await db.all("SELECT txt FROM foo ORDER BY id").then((rows) => {
      for (var i = 0; i < rows.length; i++) {
        assert.equal(rows[i].txt, data[i]);
        retrieved++;
      }
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot retrieve from an incorrect table', async function () {
    try {
      await db.all("SELECT txt FROM foo1 ORDER BY id");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: foo1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: cannot retrieve from an incorrect column', async function () {
    try {
      await db.all("SELECT txt1 FROM foo ORDER BY id");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: txt1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should have inserted and retrieved the correct amount', async function () {
    assert.equal(inserted, length);
    assert.equal(retrieved, length);
  });

  after(async function () { await db.close(); });
});
