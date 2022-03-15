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

describe('named parameters', function () {
  var db;

  before(async function () {
    db = await new sqlite3.Database(':memory:');
  });

  it('+ve: should create the table', async function () {
    await db.run("CREATE TABLE foo (txt TEXT, num INT)").then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: create a duplicate table', async function () {
    await db.run("CREATE TABLE foo (txt TEXT, num INT)").then(() => {
      assert(false);
    }).catch((err) => {
      assert.equal(err.message, 'SQLITE_ERROR: table foo already exists');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    });
  });

  it('+ve: should insert a value with $ placeholders', async function () {
    await db.run("INSERT INTO foo VALUES($text, $id)", {
      $id: 1,
      $text: "Lorem Ipsum"
    }).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('+ve: should insert a value with : placeholders', async function () {
    await db.run("INSERT INTO foo VALUES(:text, :id)", {
      ':id': 2,
      ':text': "Dolor Sit Amet"
    }).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot insert a value with incorrect : placeholders', async function () {
    try {
      await db.run("INSERT INTO foo VALUES(::text, :id)", {
        ':id': 2,
        ':text': "Dolor Sit Amet"
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: unrecognized token: ":"');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should insert a value with @ placeholders', async function () {
    await db.run("INSERT INTO foo VALUES(@txt, @id)", {
      "@id": 3,
      "@txt": "Consectetur Adipiscing Elit"
    }).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot insert a value with incorrect @ placeholders', async function () {
    try {
      await db.run("INSERT INTO foo VALUES(@@text, @id)", {
        "@id": 3,
        "@txt": "Consectetur Adipiscing Elit"
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: unrecognized token: "@"');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should insert a value with @ placeholders using an array', async function () {
    await db.run("INSERT INTO foo VALUES(@txt, @id)", ['Sed Do Eiusmod', 4]
    ).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot insert a value with incorrect @ placeholders using an array', async function () {
    try {
      await db.run("INSERT INTO foo VALUES(@@txt, @id)", ['Sed Do Eiusmod', 4]);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: unrecognized token: "@"');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should insert a value with indexed placeholders', async function () {
    await db.run("INSERT INTO foo VALUES(?2, ?4)",
      [null, 'Tempor Incididunt', null, 5]
    ).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot insert a value with incorrect indexed placeholders', async function () {
    try {
      await db.run("INSERT INTO foo VALUES(??2, ?4)", [null, 'Tempor Incididunt', null, 5]);
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "?2": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should insert a value with autoindexed placeholders', async function () {
    await db.run("INSERT INTO foo VALUES(?, ?)", {
      2: 6,
      1: "Ut Labore Et Dolore"
    }).then(() => {
      assert(true);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: custom: cannot insert a value with autoindexed placeholders', async function () {
    try {
      await db.run("INSERT INTO foo VALUES(??, ?)", {
        2: 6,
        1: "Ut Labore Et Dolore"
      });
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "?": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: should retrieve all inserted values', async function () {
    await db.all("SELECT txt, num FROM foo ORDER BY num").then((rows) => {
      assert.equal(rows[0].txt, "Lorem Ipsum");
      assert.equal(rows[0].num, 1);
      assert.equal(rows[1].txt, "Dolor Sit Amet");
      assert.equal(rows[1].num, 2);
      assert.equal(rows[2].txt, "Consectetur Adipiscing Elit");
      assert.equal(rows[2].num, 3);
      assert.equal(rows[3].txt, "Sed Do Eiusmod");
      assert.equal(rows[3].num, 4);
      assert.equal(rows[4].txt, "Tempor Incididunt");
      assert.equal(rows[4].num, 5);
      assert.equal(rows[5].txt, "Ut Labore Et Dolore");
      assert.equal(rows[5].num, 6);
    }).catch((err) => {
      throw err;
    });
  });

  it('-ve: cannot retrieve incorrectly named values', async function () {
    try {
      let rows = await db.all("SELECT txt, num FROM foo ORDER BY num");
      assert.equal(rows[0].txt1, undefined);
      assert.equal(rows[0].num1, undefined);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: cannot retrieve using incorrect order by', async function () {
    try {
      let rows = await db.all("SELECT txt, num FROM foo ORDER BY num1");
      assert.equal(rows[0].txt1, undefined);
      assert.equal(rows[0].num1, undefined);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: num1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async function () { await db.close(); });
});
