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

describe('constants', function() {
    it('+ve: should have the right OPEN_* flags', function() {
        assert.ok(sqlite3.OPEN_READONLY === 1);
        assert.ok(sqlite3.OPEN_READWRITE === 2);
        assert.ok(sqlite3.OPEN_CREATE === 4);
        assert.ok(sqlite3.OPEN_URI === 0x00000040);
        assert.ok(sqlite3.OPEN_FULLMUTEX === 0x00010000);
        assert.ok(sqlite3.OPEN_SHAREDCACHE === 0x00020000);
        assert.ok(sqlite3.OPEN_PRIVATECACHE === 0x00040000);
    });

    it('+ve: should have the right error flags', function() {
        assert.ok(sqlite3.OK === 0);
        assert.ok(sqlite3.ERROR === 1);
        assert.ok(sqlite3.INTERNAL === 2);
        assert.ok(sqlite3.PERM === 3);
        assert.ok(sqlite3.ABORT === 4);
        assert.ok(sqlite3.BUSY === 5);
        assert.ok(sqlite3.LOCKED === 6);
        assert.ok(sqlite3.NOMEM === 7);
        assert.ok(sqlite3.READONLY === 8);
        assert.ok(sqlite3.INTERRUPT === 9);
        assert.ok(sqlite3.IOERR === 10);
        assert.ok(sqlite3.CORRUPT === 11);
        assert.ok(sqlite3.NOTFOUND === 12);
        assert.ok(sqlite3.FULL === 13);
        assert.ok(sqlite3.CANTOPEN === 14);
        assert.ok(sqlite3.PROTOCOL === 15);
        assert.ok(sqlite3.EMPTY === 16);
        assert.ok(sqlite3.SCHEMA === 17);
        assert.ok(sqlite3.TOOBIG === 18);
        assert.ok(sqlite3.CONSTRAINT === 19);
        assert.ok(sqlite3.MISMATCH === 20);
        assert.ok(sqlite3.MISUSE === 21);
        assert.ok(sqlite3.NOLFS === 22);
        assert.ok(sqlite3.AUTH === 23);
        assert.ok(sqlite3.FORMAT === 24);
        assert.ok(sqlite3.RANGE === 25);
        assert.ok(sqlite3.NOTADB === 26);
    });

    it('-ve: custom: incorrect OPEN_* flags', function() {
      assert.ok(sqlite3.OPEN_READONLY_X !== 0);
      assert.ok(sqlite3.OPEN_READWRITE_X !== 2);
      assert.ok(sqlite3.OPEN_CREATE_X !== 4);
      assert.ok(sqlite3.OPEN_URI_X !== 0x00000040);
      assert.ok(sqlite3.OPEN_FULLMUTEX_X !== 0x00010000);
      assert.ok(sqlite3.OPEN_SHAREDCACHE_X !== 0x00020000);
      assert.ok(sqlite3.OPEN_PRIVATECACHE_X !== 0x00040000);
    });

    it('-ve: custom: incorrect right error flags', function() {
      assert.ok(sqlite3.OK_X !== 0);
      assert.ok(sqlite3.ERROR_X !== 1);
      assert.ok(sqlite3.INTERNAL_X !== 2);
      assert.ok(sqlite3.PERM_X !== 3);
      assert.ok(sqlite3.ABORT_X !== 4);
      assert.ok(sqlite3.BUSY_X !== 5);
      assert.ok(sqlite3.LOCKED_X !== 6);
      assert.ok(sqlite3.NOMEM_X !== 7);
      assert.ok(sqlite3.READONLY_X !== 8);
      assert.ok(sqlite3.INTERRUPT_X !== 9);
      assert.ok(sqlite3.IOERR_X !== 10);
      assert.ok(sqlite3.CORRUPT_X !== 11);
      assert.ok(sqlite3.NOTFOUND_X !== 12);
      assert.ok(sqlite3.FULL_X !== 13);
      assert.ok(sqlite3.CANTOPEN_X !== 14);
      assert.ok(sqlite3.PROTOCOL_X !== 15);
      assert.ok(sqlite3.EMPTY_X !== 16);
      assert.ok(sqlite3.SCHEMA_X !== 17);
      assert.ok(sqlite3.TOOBIG_X !== 18);
      assert.ok(sqlite3.CONSTRAINT_X !== 19);
      assert.ok(sqlite3.MISMATCH_X !== 20);
      assert.ok(sqlite3.MISUSE_X !== 21);
      assert.ok(sqlite3.NOLFS_X !== 22);
      assert.ok(sqlite3.AUTH_X !== 23);
      assert.ok(sqlite3.FORMAT_X !== 24);
      assert.ok(sqlite3.RANGE_X !== 25);
      assert.ok(sqlite3.NOTADB_X !== 26);
  });

});
