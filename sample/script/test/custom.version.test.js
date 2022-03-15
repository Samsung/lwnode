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

var assert = require('assert');
var helper = require('./support/helper');
helper.loadSqlite3();

describe('libsqlite3 version', function () {

  before(function () {
  });

  it('+ve: check libsqlite3 version', async () => {
    let num = await sqlite3.VERSION_NUMBER;
    assert(num);
  });

  it ('-ve: custom: incorrect use of VERSION_NUMBER1', async () => {
    let num = await sqlite3.VERSION_NUMBER1;
    assert(num === undefined);
  });
});
