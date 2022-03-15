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

global.require = require;

const assert = require('assert');
const path = require('path');
const utils = require('./support/utils');
const helper = require('./support/helper');

// TODO: The variables is not found in the taget .

// const variables = require('./support/variables');
// const url = `ws://${variables.DB_HOSTNAME}:${variables.DB_PORT}${variables.DB_PATH}`;
const DB_HOSTNAME = '127.0.0.1';
const DB_PORT = process.env.DB_PORT || 8140;
const DB_PATH = '/';
const url = `ws://${DB_HOSTNAME}:${DB_PORT}${DB_PATH}`;

let scriptPath;

if (helper.isTizen) {
  document.addEventListener('tizenhwkey', function (e) {
    if (e.keyName == 'back')
      try {
        tizen.application.getCurrentApplication().exit();
      } catch (ignore) {}
  });

  scriptPath = `http://${DB_HOSTNAME}:${DB_PORT}/api/download/libsqlite.js`;

} else {
  scriptPath = 'script/dist/libsqlite.js';
}

window.onload = function () {

  let s = document.createElement('script');
  s.type = 'text/javascript';
  s.src = scriptPath;
  s.onload = async function () {

    if (mocha === undefined) {
      throw new Error("Cannot load mocha!");
    }

    if (helper.isTizen) {
      await initTizen();
    }

    sqlite3.configure(url);
    utils.log('Start Test');
    mocha.run();
  }

  document.querySelector('head').appendChild(s);
};
