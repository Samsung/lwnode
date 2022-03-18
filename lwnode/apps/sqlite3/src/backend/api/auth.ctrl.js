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

const { paths } = require('../lib/variables');
const Joi = require('joi');

const debug = require('debug')('api');

exports.localRegister = async (req, res) => {
  debug(`${req.method} ${req.url} ${req.ip}`);
  debug(`${JSON.stringify(req.body)}`);

  const { body } = req;

  const schema = Joi.object({
    email: Joi.string().email().required(),
    password: Joi.string().min(4).max(30).required(),
    displayName: Joi.string().regex(/^[a-zA-Z0-9ㄱ-힣]{3,12}$/),
  });

  const result = schema.validate(body);
  if (result.error) {
    res.status(400).end(result.error.details[0].message);
    return;
  }

  // todo: save user

  res.status(200).end('OK');
};

exports.localLogin = async (req, res) => {
  const { body } = req;

  const schema = Joi.object({
    email: Joi.string().email().required(),
    password: Joi.string().min(6).max(30),
  });

  const result = schema.validate(body);

  if (result.error) {
    res.status(400).end(result.error.details[0].message);
    return;
  }

  const { email, password } = body;

  try {
    // todo: find user
    const user = null;

    if (!user) {
      res.status(403).end('user does not exist');
      return;
    }

    // todo: validate password
    const validated = false;
    if (!validated) {
      res.status(403).end('wrong password');
      return;
    }

    // todo: generate token
    const accessToken = await user.generateToken();

    // todo: write response body
    // todo: set cookie
  } catch (e) {
    res.status(500).end('something wrong password');
  }
};
