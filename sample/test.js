/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
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

// var md5 = require("md5");
const DB_SOURCE = "sqlite3.db";
const DB_HOSTNAME = "localhost";
const DB_PORT = 8140;
const DB_PATH = "/";

async function test1() {
  var url = "ws://" + DB_HOSTNAME + ":" + DB_PORT + DB_PATH;

  let sqlite3 = new SQLite3(url);
  let db = await sqlite3.open(DB_SOURCE, (err) => {
    if (err) {
      console.log("open: error");
      return;
    }
    console.log("open: ok");
  });

  console.log("db: " + JSON.stringify(db));

  {
    let q = `CREATE TABLE user (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name text,
            email text UNIQUE,
            password text,
            CONSTRAINT email_unique UNIQUE (email)
            )`;

    await db.run(q, [], (err) => {
      if (err) {
        console.log("run: error: " + JSON.stringify(err));
        return;
      }
      console.log("run: ok");
    });
  }

  let q = 'INSERT INTO user (name, email, password) VALUES (?,?,?)';
  {
    let params = ["admin1", "admin@example.com", "admin123456"]; // need md5();
    await db.run(q, params, (err) => {
      if (err) {
        console.log("run: error: " + JSON.stringify(err));
        return;
      }
      console.log("run: ok");
    });
  }

  {
    let params = ["user1", "user@example.com", "user123456"]; // need md5();
    await db.run(q, params, (err) => {
      if (err) {
        console.log("run: error: " + JSON.stringify(err));
        return;
      }
      console.log("run: ok");
    });
  }

  {
    let q = "select * from user";
    await db.all(q, [], (err, rows) => {
      if (err) {
        console.log("all: error");
        return;
      }
      console.log("all: ok: " + JSON.stringify(rows));
    });

    await db.get(q, [], (err, row) => {
      if (err) {
        console.log("get: error");
        return;
      }
      console.log("get: ok: " + JSON.stringify(row));
    });
  }

  console.log("Testing Each");
  {
    let q = "select * from user";
    await db.each(q, [], (err, row) => {
      if (err) {
        console.log("each: error");
        return;
      }
      console.log("each: ok: " + JSON.stringify(row));
    }, (err, rowCount) => {
      if (err) {
        console.log("each: complete error");
        return;
      }
      console.log("each: complete ok: " + rowCount);
    });
  }

  {
    let q = "select * from user";
    await db.exec(q, (err) => {
      if (err) {
        console.log("exec: error");
        return;
      }
      console.log("exec: ok");
    });
  }

  {
    let q = "select * from user";
    let stmt = await db.prepare(q, (err) => {
      if (err) {
        console.log("prepare: error");
        return;
      }
      console.log("prepare: ok");
    });

    console.log("Prepare: client " + JSON.stringify(stmt));

    await stmt.finalize((err) => {
      if (err) {
        console.log("finalize: error");
        return;
      }
      console.log("finalize: ok");
    });
  }

  await db.close((err) => {
    if (err) {
      console.log("error: close: " + JSON.stringify(err));
      return;
    }
    console.log("close: ok");
  });
}
