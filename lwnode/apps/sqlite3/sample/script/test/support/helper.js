var assert = require('assert');
var fs = require('fs');
var path = require('path');
var pathExists = fs.existsSync || path.existsSync;
var isBrowser = typeof window !== 'undefined' && window.document;
var isTizen = global.tizen !== undefined;
var utils = require('./utils');

function getStoreFilePath(name) {
  if (isTizen) {
    return path.resolve('/tmp', name);
  } else if (isBrowser) {
    let root = process.env.DB_STORE || process.cwd();
    return path.resolve(root, name);
  }
  return path.resolve(__dirname, name);
}

if (isBrowser) {
  exports.deleteFile = async function (name) {
    try {
      await utils.requestAPI('delete', name);
    } catch (err) {
    }
  };

  exports.ensureExists = function (name) {
    // There is no problem even if this function does not run.
  };

  assert.fileDoesNotExist = async function (name) {
    try {
      let result = await utils.requestAPI('exist', name);
      if (result === 'true') {
        throw new Error(`${name} exist.`)
      }
    } catch (err) {
      throw err;
    }
  };

  assert.fileExists = async function (name) {
    try {
      let result = await utils.requestAPI('exist', name);
      if (result === 'false') {
        throw new Error(`${name} does not exist.`)
      }
      return true;
    } catch (err) {
      throw err;
    }
  };

} else {
  exports.deleteFile = function (name) {
    try {
      fs.unlinkSync(getStoreFilePath(name));
    } catch (err) {
      if (
        err.errno !== process.ENOENT &&
        err.code !== 'ENOENT' &&
        err.syscall !== 'unlink'
      ) {
        throw err;
      }
    }
  };

  exports.ensureExists = function (name) {
    if (!pathExists(name)) {
      fs.mkdirSync(name);
    }
  };

  assert.fileDoesNotExist = function (name) {
    try {
      fs.statSync(getStoreFilePath(name));
    } catch (err) {
      if (
        err.errno !== process.ENOENT &&
        err.code !== 'ENOENT' &&
        err.syscall !== 'unlink'
      ) {
        throw err;
      }
    }
  };

  assert.fileExists = function (name) {
    try {
      fs.statSync(getStoreFilePath(name));
    } catch (err) {
      throw err;
    }
  };
}

function loadSqlite3() {
  if (!isBrowser && !global.sqlite3) {
    require("./setup-sqlite3.js");
  }
}

exports.isBrowser = isBrowser;
exports.isTizen = isTizen;
exports.getStoreFilePath = getStoreFilePath;
exports.loadSqlite3 = loadSqlite3;
