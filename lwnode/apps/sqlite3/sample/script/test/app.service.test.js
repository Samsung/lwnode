/*
 * Copyright 2022-present Samsung Electronics Co., Ltd.
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

const assert = require('assert');
const helper = require('./support/helper');
const { SERVER_URL, BACKEND_PATH } = require('./support/variables');
const app_name = 'hello';

describe('service API', function() {
  before(function () {
  });

  it('register', async () => {
    const body = {
      name: app_name,
      path: `${__dirname}/service/hello_world`
    }

    const data = {
      method: 'post',
      body: JSON.stringify(body),
      headers: {'Content-Type': 'application/json'}
    }

    let response = await fetch(`${SERVER_URL}/api/service/register`, data);
    assert.equal(response.status, 200);

    response = await fetch(`${SERVER_URL}/api/service/register`, data);
    assert.equal(response.status, 400);
  });

  it('delete', async () => {
    let response = await fetch(`${SERVER_URL}/api/service/delete/${app_name}`, {
      method: 'DELETE'
    });
    assert.equal(response.status, 200);
  });
});
