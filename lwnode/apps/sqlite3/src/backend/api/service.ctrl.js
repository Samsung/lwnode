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

import { ServiceData, ServicesDB } from '../db/Services';
import DAO from '../db/dao';

exports.registerService = async (req, res) => {
  debug(`${req.method} ${req.url} ${req.ip}`);

  const { body: data } = req;

  const schema = Joi.object({
    name: Joi.string().min(2).max(30).required(),
    path: Joi.string().min(2).max(100).required(),
  });

  const result = schema.validate(data);
  if (result.error) {
    return res.status(400).end(result.error.details[0].message);
  }

  try {
    let services = new ServicesDB(DAO.knex());
    debug(`register service: ${data.name}(${data.path})`);

    if (await services.getDataByName(data.name)) {
      return res.status(400).end(`${data.name} already exists`);
    }

    await services.create(data);

    return res.status(200).end('OK');
  } catch (error) {
    return res.status(500).end('something wrong');
  }
};
