/* Copyright 2020-present Samsung Electronics Co., Ltd. and other contributors
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

const express = require('express');
const apiCtrl = require('./api.ctrl');
const authCtrl = require('./auth.ctrl');
const serviceCtrl = require('./service.ctrl');

const api = express.Router();
const auth = express.Router();
const service = express.Router();

api.use('/auth', auth);

api.get('/download/:file', apiCtrl.downloadFile);
api.delete('/delete/:file', apiCtrl.deleteFile);
api.get('/exist/:file', apiCtrl.existFile);

auth.post('/register/local', authCtrl.localRegister);
auth.post('/login/local', authCtrl.localLogin);
auth.get('/logout', authCtrl.logout);

api.use('/service', service);

service.post('/register', serviceCtrl.registerService);
service.delete('/delete/:name', serviceCtrl.deleteService);

module.exports = api;
