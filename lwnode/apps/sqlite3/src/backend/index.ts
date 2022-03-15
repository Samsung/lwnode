/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
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

if (process.cwd() != __dirname) {
  process.chdir(__dirname);
}

(function reloadNodePath(nodePath) {
  process.env.NODE_PATH = process.env.NODE_PATH
    ? `${process.env.NODE_PATH}:${nodePath}`
    : nodePath;

  require('module').Module._initPaths(); // eslint-disable-line no-underscore-dangle
})('./');

const { port } = require('./lib/variables');
const compression = require('compression');
const express = require('express');
import { log } from './lib/log';
const debug = require('debug');

const asyncify = require('express-asyncify');

const app = asyncify(express());

startServer(app);

function startServer(app) {
  const http = require('http');
  const WebSocket = require('ws');
  const { isLinux } = require('./lib/utils');
  const api = require('./api');
  const router = express.Router();

  app.all('/*', function (req, res, next) {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Headers', 'X-Requested-With');
    res.header(
      'Access-Control-Allow-Methods',
      'GET, POST, DELETE, PATCH, OPTIONS',
    );
    next();
  });

  const rpc = require('./lib/rpc').create();
  const { database, statement } = require('./lib/sqlite3');

  rpc.use('Database#', database.ws);
  rpc.use('Statement#', statement.ws);
  router.use('/api', api);

  app.use(compression());
  app.use(express.json());
  app.use(express.urlencoded({ extended: false }));
  app.use(router);

  const server = http.createServer(app);
  const wss = new WebSocket.Server({ server });

  wss.on('connection', (ws) => rpc.handle(ws));

  server.listen(port, (err) => {
    if (err) {
      log.error(err);
      process.exit(1);
    }

    log.info(`listening port: ${port}`);

    let onCloseCallback = setServerCloseHandler(server);

    if (isLinux()) {
      enablePrompt(onCloseCallback);
    }
  });

  return server;
}

function setServerCloseHandler(server) {
  ['SIGINT', 'SIGTERM', 'SIGQUIT', 'exit'].forEach((signal) => {
    // @ts-ignore
    process.once(signal, shutdown);
  });

  let connections = [];

  server.on('connection', (socket) => {
    connections.push(socket);
    socket.on('close', () => {
      connections = connections.filter((element) => element !== socket);
    });
  });

  server.on('error', (error) => {
    log.error(`error: ${error}`);
  });

  let isShutdowning = false;

  function shutdown() {
    if (isShutdowning) {
      return;
    }
    isShutdowning = true;

    server.close(() => {
      log.info('shutdown');
      process.exit(0);
    });

    connections.forEach((socket) => socket.end());

    const socketCloseTimeout = 300;
    const serverCloseTimeout = 10000;

    setTimeout(() => {
      log.info('serverCloseTimeout');
      process.exit(1);
    }, serverCloseTimeout);

    setTimeout(() => {
      log.info('socketCloseTimeout');
      connections.forEach((socket) => socket.destroy());
    }, socketCloseTimeout);
  }

  return shutdown;
}

function enablePrompt(closeCallback) {
  const readline = require('readline');
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    completer: (line) => {
      const completions = 'c clear d debug e q x exit'.split(' ');
      const hits = completions.filter((c) => c.startsWith(line));
      return [hits.length ? hits : completions, line];
    },
  });

  if (process.stdin.isTTY) {
    process.stdin.setRawMode(true);
  }

  rl.on('line', function (line) {
    let input = line.trim().split(/[ =]+/);
    switch (input[0]) {
      case 'q':
      case 'e':
      case 'x':
      case 'exit':
        rl.close();
        break;
      case 'c':
      case 'clear':
        process.stdout.write('\033[2J');
        process.stdout.write('\u001b[H\u001b[2J\u001b[3J');
        break;
      case 'd':
      case 'debug':
        let cns = debug.disable();
        let nns = input[1] ? input[1] : '';
        debug.enable(nns);
        console.log(`[${cns}]  -->  [${nns}]\n`);
        break;
      default:
        break;
    }
    rl.prompt();
  });

  rl.on('close', function () {
    closeCallback && closeCallback();
  });

  rl.on('SIGINT', function () {
    rl.close();
  });

  console.log('\ndebug console is open.');
  rl.prompt();
}
