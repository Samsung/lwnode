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

if (!global.fetch) {
  global.fetch = require('node-fetch');
}

var assert = require('assert');
var helper = require('./support/helper');
helper.loadSqlite3();

const { SERVER_URL } = require('./support/variables');
const FILENAME = 'text.txt';

describe('internal apis', function() {
  before(function () {
    if (global.process) {
      const fs = require('fs');
      fs.open(`/tmp/${FILENAME}`, 'w', function (err, file) {
        if (err) throw err;
      });
    }
  });

  it('/api/download/libsqlite.js should return 200', async () => {
    const response = await fetch(`${SERVER_URL}/api/download/libsqlite.js`);
    assert.equal(200, response.status);
  });

  it(`/api/exist should return true if ${FILENAME} exist`, async () => {
    const response = await fetch(`${SERVER_URL}/api/exist/${FILENAME}`)
      .then(res => {
        assert.equal(200, res.status);
        return res.text();
      }).then((text) => {
        if (text === 'true') {
          assert(true);
        } else if (text == 'false') {
          console.warn(`${FILENAME} doesn't exist.`);
        } else {
          assert(false);
        }
      });
  });

  it('/api/delete should return the request result', async () => {
    const text = await fetch(`${SERVER_URL}/api/exist/${FILENAME}`)
      .then(res => res.text());

    if (text === 'true') {
      const response = await fetch(`${SERVER_URL}/api/delete/${FILENAME}`, {
        method: 'DELETE'
      });
      assert.equal(200, response.status);
    } else {
      console.warn(`${FILENAME} doesn't exist.`);
    }
  });
});
