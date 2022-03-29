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

const Joi = require('joi');
const debug = require('debug')('api');

import Users, { UserData } from '../db/Users';
import DAO from '../db/dao';

exports.localRegister = async (req, res) => {
  debug(`${req.method} ${req.url} ${req.ip}`);

  const { body: data } = req;

  // todo: use joi typescript validator with UserData
  const schema = Joi.object({
    email: Joi.string().email().required(),
    password: Joi.string().min(4).max(30).required(),
    displayName: Joi.string().regex(/^[a-zA-Z0-9ㄱ-힣]{3,12}$/),
  });

  const result = schema.validate(data);
  if (result.error) {
    return res.status(400).end(result.error.details[0].message);
  }

  try {
    let users = new Users(DAO.knex());

    if (await users.getByEmail(data.email)) {
      return res.status(400).end(`${data.email} already exists`);
    }

    await users.create(data);

    return res.status(200).end('OK');
  } catch (error) {
    debug(error);
    return res.status(500).end('something wrong');
  }
};

exports.localLogin = async (req, res) => {
  debug(`${req.method} ${req.url} ${req.ip}`);

  const { body } = req;

  const schema = Joi.object({
    email: Joi.string().email().required(),
    password: Joi.string().min(6).max(30),
  });

  const result = schema.validate(body);

  if (result.error) {
    return res.status(400).end(result.error.details[0].message);
  }

  const { email, password } = body;

  try {
    let users = new Users(DAO.knex());
    const user = await users.getByEmail(email);

    if (!user) {
      return res.status(403).end('user does not exist');
    }

    if (user.validatePassword(password) == false) {
      return res.status(403).end('wrong password');
    }

    const accessToken = await user.generateToken();

    return res
      .cookie('access_token', accessToken, {
        httpOnly: true,
        maxAge: 1000 * 60 * 60 * 24 * 3, // 3 days
        secure: process.env.NODE_ENV === 'production',
      })
      .status(200)
      .end('OK');
  } catch (e) {
    res.status(500).end('something wrong');
  }
};
