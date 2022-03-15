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

describe('scheduling', async function() {
    it('-ve: scheduling after the database was closed', async function() {
        var db = await new sqlite3.Database(':memory:');
        await db.close();

        try {
          await db.run("CREATE TABLE foo (id int)");
          assert(false);
        } catch (err) {
          assert.ok(err);
        }
    });

    it('-ve: scheduling a query with callback after the database was closed', async function() {
        var db = await new sqlite3.Database(':memory:');
        await db.close();

        try {
          await db.run("CREATE TABLE foo (id int)");
          assert(false);
        } catch (err) {
          assert.ok(err);
          assert.ok(err.message && err.message.indexOf("SQLITE_MISUSE: Database is closed") > -1);
        }
    });

    it('-ve: running a query after the database was closed', async function() {
        var db = await new sqlite3.Database(':memory:');
        var stmt = await db.prepare("SELECT * FROM sqlite_master");

        try {
          await db.close();
          assert(false);
        } catch (err) {
          assert.ok(err);
          assert.ok(err.message && err.message.indexOf("SQLITE_BUSY: unable to close due to") > -1);
        }

        // Running a statement now should not fail.
        await stmt.run();
    });
});
