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

describe('open/close', function () {
  let version;

  before(async () => {
    version = await sqlite3.VERSION_NUMBER;
  });

  describe('open and close non-existent database', function () {
    let filename = 'test_create.db';

    before(async () => {
      await helper.deleteFile(filename);
    });

    var db;
    it('+ve: should open the database', async () => {
      console.log(helper.getStoreFilePath(filename));
      db = await new sqlite3.Database(helper.getStoreFilePath(filename));
    });

    it('+ve: should close the database', async () => {
      await db.close();
    });

    it('+ve: should have created the file', async () => {
      await assert.fileExists(filename);
    });

    // after(async () => {
    //   await helper.deleteFile(filename);
    // });
  });

  // describe('open and close non-existent shared database', function () {
  //   var filename = 'test_create_shared.db';

  //   before(async () => {
  //     await helper.deleteFile(filename);
  //   });

  //   var db;
  //   it('should open the database', async () => {
  //     db = await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_URI | sqlite3.OPEN_SHAREDCACHE | sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);
  //   });

  //   it('should close the database', async () => {
  //     await db.close();
  //   });

  //   it('should have created the file', async () => {
  //     await assert.fileExists(filename);
  //   });

  //   after(async () => {
  //     await helper.deleteFile(filename);
  //   });
  // });

  // if (helper.isNode) {
  // (version < 3008000 ? describe.skip : describe)('open and close shared memory database', function () {
  //   var db1;
  //   var db2;

  //   before(function () {
  //   });

  //   it('should open the first database', async () => {
  //     // 'file:./test/tmp/test_memory.db?mode=memory'
  //     db1 = await new sqlite3.Database('file:./test/support/test_memory.db?mode=memory', sqlite3.OPEN_URI | sqlite3.OPEN_SHAREDCACHE | sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);
  //   });

  //   it('should open the second database', async () => {
  //     // 'file:./test/tmp/test_memory.db?mode=memory'
  //     db2 = await new sqlite3.Database('file:./test/support/test_memory.db?mode=memory', sqlite3.OPEN_URI | sqlite3.OPEN_SHAREDCACHE | sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);
  //   });

  //   it('first database should set the user_version', async () => {
  //     await db1.exec('PRAGMA user_version=42');
  //   });

  //   it('second database should get the user_version', async () => {
  //     try {
  //       let row = await db2.get('PRAGMA user_version');
  //       assert.equal(row.user_version, 42);
  //     } catch (err) {
  //       throw err;
  //     }
  //   });

  //   it('should close the first database', async () => {
  //     await db1.close();
  //   });

  //   it('should close the second database', async () => {
  //     await db2.close();
  //   });
  // });
  // }

  // it('should not be unable to open an inaccessible database', async () => {
  //   // NOTE: test assumes that the user is not allowed to create new files
  //   // in /usr/bin.
  //   try {
  //     var db = await new sqlite3.Database('/test/tmp/directory-does-not-exist/test.db');
  //   } catch (err) {
  //     if (err && err.errno === sqlite3.CANTOPEN) {
  //       assert(true);
  //     } else if (err) {
  //       throw err;
  //     } else {
  //       new Error('Opened database that should be inaccessible');
  //     }
  //   }
  // });

  // describe('creating database without create flag', function () {
  //   var filename = 'test_readonly.db';

  //   before(async () => {
  //     await helper.deleteFile(filename);
  //   });

  //   it('should fail to open the database', async () => {
  //     try {
  //       await new sqlite3.Database(helper.getStoreFilePath(filename), sqlite3.OPEN_READONLY);
  //     } catch (err) {
  //       if (err && err.errno === sqlite3.CANTOPEN) {
  //         assert(true);
  //       } else if (err) {
  //         throw err;
  //       } else {
  //         new Error('Created database without create flag');
  //       }
  //     }
  //   });

  //   it('should not have created the file', async () => {
  //     await assert.fileDoesNotExist(filename);
  //   });

  //   after(async () => {
  //     await helper.deleteFile(filename);
  //   });
  // });

  // describe('open and close memory database queuing', function () {
  //   var db;
  //   it('should open the database', async () => {
  //     db = await new sqlite3.Database(':memory:');
  //   });

  //   it('should close the database', async () => {
  //     await db.close();
  //   });

  //   it('shouldn\'t close the database again', async () => {
  //     try {
  //       await db.close();
  //     } catch (err) {
  //       assert.ok(err, 'No error object received on second close');
  //       assert.ok(err.errno === sqlite3.MISUSE);
  //     }
  //   });
  // });

  // describe('closing with unfinalized statements', function () {
  //   var completed = false;
  //   var completedSecond = false;
  //   var closed = false;

  //   var db;
  //   before(async () => {
  //     db = await new sqlite3.Database(':memory:');
  //   });

  //   it('should create a table', async () => {
  //     await db.run("CREATE TABLE foo (id INT, num INT)");
  //   });

  //   var stmt;
  //   it('should prepare/run a statement', async () => {
  //     stmt = await db.prepare('INSERT INTO foo VALUES (?, ?)');
  //     await stmt.run(1, 2);
  //   });

  //   it('should fail to close the database', async () => {
  //     try {
  //       await db.close();
  //       assert(false);
  //     } catch (err) {
  //       assert.ok(err.message,
  //         "SQLITE_BUSY: unable to close due to unfinalised statements");
  //     }
  //   });

  //   it('should succeed to close the database after finalizing', async () => {
  //     await stmt.run(3, 4);
  //     await stmt.finalize();
  //     await db.close();
  //   });
  // });
});
