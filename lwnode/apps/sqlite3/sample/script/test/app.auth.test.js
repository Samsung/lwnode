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
const { SERVER_URL } = require('./support/variables');

const data = {
  email: 'unittest@samsung.com',
  password: '123456',
  displayName: 'Test',
};

const options = {
  headers: { 'content-type': 'application/json' },
};

const url = {
  register: `${SERVER_URL}/api/auth/register/local`,
  delete: `${SERVER_URL}/api/auth/local`,
  login: `${SERVER_URL}/api/auth/login/local`,
  logout: `${SERVER_URL}/api/auth/logout`,
};

function name(url) {
  return url.split(`${SERVER_URL}/api/auth`).pop();
}

describe('api/auth', function () {
  describe(name(url.register), function () {
    beforeEach(async () => {
      // clean up
      await fetch(url.delete, {
        ...options,
        method: 'DELETE',
        body: JSON.stringify(data),
      });
    });

    it('register', async () => {
      const res = await fetch(url.register, {
        ...options,
        method: 'POST',
        body: JSON.stringify(data),
      });
      assert.equal(res.status, 200);
    });

    it('register with no email', async () => {
      const res = await fetch(url.register, {
        ...options,
        method: 'POST',
        body: JSON.stringify({ ...data, email: '' }),
      });

      assert.equal(res.status, 400);
    });
  });

  describe(name(url.login), function () {
    before(async () => {
      await fetch(url.delete, {
        ...options,
        method: 'DELETE',
        body: JSON.stringify(data),
      });

      const res = await fetch(url.register, {
        ...options,
        method: 'POST',
        body: JSON.stringify(data),
      });

      assert.equal(res.status, 200);
    });

    it('login', async () => {
      const res = await fetch(url.login, {
        ...options,
        method: 'POST',
        body: JSON.stringify(data),
      });

      assert.equal(res.status, 200);

      try {
        const cookie = res.headers.get('Set-Cookie');
        const { 0: key, 1: value } = cookie.split(';')[0].split('=');
        assert.equal(key, 'access_token');
        assert.ok(value);
      } catch (e) {
        assert.fail('invalid jwt cookie');
      }
    });

    it('login with no password', async () => {
      const res = await fetch(url.login, {
        ...options,
        method: 'POST',
        body: JSON.stringify({ ...data, password: '' }),
      });

      assert.equal(res.status, 400);
    });
  });

  describe(name(url.logout), function () {
    it('logout', async () => {
      const res = await fetch(url.logout, {
        ...options,
        method: 'GET',
      });

      assert.equal(res.status, 200);

      try {
        const cookie = res.headers.get('Set-Cookie');
        const { 0: key, 1: value } = cookie.split(';')[0].split('=');
        assert.equal(key, 'access_token');
        assert.ok(!value);
      } catch (e) {
        assert.fail('invalid jwt cookie');
      }
    });
  });

  describe(name(url.delete) + ' in delete method', function () {
    before(async () => {
      await fetch(url.delete, {
        ...options,
        method: 'DELETE',
        body: JSON.stringify(data),
      });

      const res = await fetch(url.register, {
        ...options,
        method: 'POST',
        body: JSON.stringify(data),
      });

      assert.equal(res.status, 200);
    });

    it('delete an acount', async () => {
      const res = await fetch(url.delete, {
        ...options,
        method: 'DELETE',
        body: JSON.stringify(data),
      });

      assert.equal(res.status, 200);

      const res2 = await fetch(url.login, {
        ...options,
        method: 'POST',
        body: JSON.stringify(data),
      });

      assert.equal(res2.status, 403);
    });
  });
});
