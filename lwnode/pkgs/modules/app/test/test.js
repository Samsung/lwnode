/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

var app = require('./app');

console.log(`app data path: ${app.common.getDataPath()}`);
console.log(`app cache path: ${app.common.getCachePath()}`);
console.log(`app resource path: ${app.common.getResourcePath()}`);
console.log(`app shared data path: ${app.common.getSharedDataPath()}`);
console.log(`app shared resource path: ${app.common.getSharedResourcePath()}`);
console.log(`app shared trusted path: ${app.common.getSharedTrustedPath()}`);
console.log(`app external data path: ${app.common.getExternalDataPath()}`);
console.log(`app external cache path: ${app.common.getExternalCachePath()}`);
console.log(`app external shared data path: ${app.common.getExternalSharedDataPath()}`);

console.log(`app name: ${app.common.getName()}`);
console.log(`app id: ${app.common.getID()}`);
console.log(`app version: ${app.common.getVersion()}`);
