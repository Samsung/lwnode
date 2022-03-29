/*
 * Copyright 2022-present Samsung Electronics Co., Ltd. and other contributors
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

import config from '../config';
const debug = require('debug')('api');

class DAO {
  private static instance: any = null;

  public static knex() {
    if (!DAO.instance) {
      debug('create a DAO');
      DAO.instance = require('knex')(config.db?.knex);
    }
    return DAO.instance;
  }
}

export default DAO;
