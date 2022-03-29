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

import { paths } from './lib/variables';
import { deepFreeze } from './lib/utils';

const filename = paths.store + '/db.lwnode';

const config = {
  db: {
    knex: {
      client: 'sqlite3',
      connection: {
        filename,
      },
      useNullAsDefault: true,
    },
    privateKey: process.env?.LWNODE_DB_USERS_PRIVATE_KEY || 'lwnode',
  },
  auth: {
    privateKey: process.env?.LWNODE_JWT_PRIVATE_KEY || 'qwer!234',
    signOptions: {
      issuer: 'lwnode',
      expiresIn: '30m',
    },
    cookieOptions: {
      httpOnly: true,
      maxAge: 1000 * 60 * 30, // 30 minutes
      secure: process.env.NODE_ENV === 'production',
    },
  },
};

// todo: check this file ownership.
// we need to consider how to securly store private key
// process.env.NODE_ENV === "production"

if (typeof config.db.privateKey != 'string') {
  throw new Error("private key doesn't exist");
}

export default deepFreeze(config);
