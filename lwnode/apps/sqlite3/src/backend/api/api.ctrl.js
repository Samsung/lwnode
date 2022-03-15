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

const { paths } = require('../lib/variables');
const { deleteFile, existFile } = require('../lib/utils');

const debug = require('debug')('api');

exports.downloadFile = async (req, res) => {
  const { file } = req.params;

  debug(`${req.method} ${req.url} ${req.ip}`);
  res.download(`${paths.download}/${file}`);
};

exports.deleteFile = async (req, res) => {
  try {
    const { file } = req.params;

    debug(`${req.method} ${req.url} ${req.ip}`);

    await deleteFile(`${paths.store}/${file}`);
    debug(`remove file : ${paths.store}/${file}`);

    res.status(200).end('OK');
  } catch (e) {
    res.status(409).end(`Error: ${e.message}`);
  }
};

exports.existFile = async (req, res) => {
  try {
    const { file } = req.params;

    debug(`${req.method} ${req.url} ${req.ip}`);

    let result = await existFile(`${paths.store}/${file}`);
    debug(`exist file : ${paths.store}/${file} - ${result}`);

    res.status(200).end(result.toString());
  } catch (e) {
    res.status(409).end(`Error: ${e.message}`);
  }
};
