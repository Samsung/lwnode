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

const { spawn } = require('child_process');
const Mocha = require('mocha');
const fs = require('fs');
const path = require('path');
const variables = require('./script/test/support/variables');
if (!global.fetch) {
  global.fetch = require('node-fetch');
}

const sqlite3ServiceRoot = process.cwd();

global.sqlite3 = require(path.join(sqlite3ServiceRoot, 'out/frontend/sqlite3.js'));
console.log(variables.DB_URL);
global.sqlite3.configure(variables.DB_URL);
global.WebSocket = require('ws');

const server = spawn(
  'lwnode',
  [
    '--allow-code-generation-from-strings',
    path.resolve(sqlite3ServiceRoot + '/out/backend'),
  ],
  {
    stdio: 'inherit',
    env: process.env,
  },
);

setTimeout(function () {
  console.log('test start');
  const mocha = new Mocha();
  mocha.timeout(560000);
  const testDir = path.resolve(sqlite3ServiceRoot, 'sample/script/test');
  fs.readdirSync(testDir)
    .filter(function (file) {
      return file.substr(-3) === '.js';
    })
    .forEach(function (file) {
      if (file === 'test_main.js') {
        return;
      }
      // if (file === 'prepare.test.js')
      mocha.addFile(path.join(testDir, file));
    });

  const runner = mocha.run(function (failures) {
    process.exitCode = failures ? 1 : 0;
  });

  runner.on('end', function () {
    server.kill('SIGINT');
  });
}, 2000);
