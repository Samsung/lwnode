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

let script_sql = `CREATE TABLE IF NOT EXISTS map (
  zoom_level INTEGER,
  tile_column INTEGER,
  tile_row INTEGER,
  tile_id TEXT,
  grid_id TEXT
);

CREATE TABLE IF NOT EXISTS grid_key (
   grid_id TEXT,
   key_name TEXT
);

CREATE TABLE IF NOT EXISTS keymap (
   key_name TEXT,
   key_json TEXT
);

CREATE TABLE IF NOT EXISTS grid_utfgrid (
   grid_id TEXT,
   grid_utfgrid TEXT
);

CREATE TABLE IF NOT EXISTS images (
   tile_data blob,
   tile_id text
);

CREATE TABLE IF NOT EXISTS metadata (
   name text,
   value text
);


CREATE UNIQUE INDEX IF NOT EXISTS map_index ON map (zoom_level, tile_column, tile_row);
CREATE UNIQUE INDEX IF NOT EXISTS grid_key_lookup ON grid_key (grid_id, key_name);
CREATE UNIQUE INDEX IF NOT EXISTS keymap_lookup ON keymap (key_name);
CREATE UNIQUE INDEX IF NOT EXISTS grid_utfgrid_lookup ON grid_utfgrid (grid_id);
CREATE UNIQUE INDEX IF NOT EXISTS images_id ON images (tile_id);
CREATE UNIQUE INDEX IF NOT EXISTS name ON metadata (name);


CREATE VIEW IF NOT EXISTS tiles AS
   SELECT
       map.zoom_level AS zoom_level,
       map.tile_column AS tile_column,
       map.tile_row AS tile_row,
       images.tile_data AS tile_data
   FROM map
   JOIN images ON images.tile_id = map.tile_id;

CREATE VIEW IF NOT EXISTS grids AS
   SELECT
       map.zoom_level AS zoom_level,
       map.tile_column AS tile_column,
       map.tile_row AS tile_row,
       grid_utfgrid.grid_utfgrid AS grid
   FROM map
   JOIN grid_utfgrid ON grid_utfgrid.grid_id = map.grid_id;

CREATE VIEW IF NOT EXISTS grid_data AS
   SELECT
       map.zoom_level AS zoom_level,
       map.tile_column AS tile_column,
       map.tile_row AS tile_row,
       keymap.key_name AS key_name,
       keymap.key_json AS key_json
   FROM map
   JOIN grid_key ON map.grid_id = grid_key.grid_id
   JOIN keymap ON grid_key.key_name = keymap.key_name;
`;

describe('exec', function () {
  var db;

  before(async () => {
    try {
      db = await new sqlite3.Database(':memory:');
    } catch (err) {
      throw err;
    }
  });

  it('+ve: Database#exec', async function () {
    try {
      await db.exec(script_sql);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: running incorrect sql with Database#exec', async function () {
    try {
      await db.exec("select1 * from foo");
      assert(false);
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: near "select1": syntax error');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('+ve: retrieve database structure', async function () {
    try {
      let rows = await db.all("SELECT type, name FROM sqlite_master ORDER BY type, name");
      assert.deepEqual(rows, [
        { type: 'index', name: 'grid_key_lookup' },
        { type: 'index', name: 'grid_utfgrid_lookup' },
        { type: 'index', name: 'images_id' },
        { type: 'index', name: 'keymap_lookup' },
        { type: 'index', name: 'map_index' },
        { type: 'index', name: 'name' },
        { type: 'table', name: 'grid_key' },
        { type: 'table', name: 'grid_utfgrid' },
        { type: 'table', name: 'images' },
        { type: 'table', name: 'keymap' },
        { type: 'table', name: 'map' },
        { type: 'table', name: 'metadata' },
        { type: 'view', name: 'grid_data' },
        { type: 'view', name: 'grids' },
        { type: 'view', name: 'tiles' }
      ]);
    } catch (err) {
      throw err;
    }
  });

  it('-ve: custom: retrieve database structure from an incorrect table', async function () {
    try {
      let rows = await db.all("SELECT type, name FROM sqlite_master1 ORDER BY type, name");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such table: sqlite_master1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve database structure from an incorrect column', async function () {
    try {
      let rows = await db.all("SELECT type1, name FROM sqlite_master ORDER BY type, name");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: type1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  it('-ve: custom: retrieve database structure from an incorrect orderby', async function () {
    try {
      let rows = await db.all("SELECT type, name FROM sqlite_master ORDER BY type1, name");
    } catch (err) {
      assert.equal(err.message, 'SQLITE_ERROR: no such column: type1');
      assert.equal(err.errno, sqlite3.ERROR);
      assert.equal(err.code, 'SQLITE_ERROR');
    }
  });

  after(async () => { await db.close(); });
});
