'use strict';

const common = require('../common');
const assert = require('assert');
const path = require('path');
const fs = require('fs');

const tmpdir = require('../common/tmpdir');
tmpdir.refresh();
const tmpDir = tmpdir.path;
const longPath = path.join(...[tmpDir].concat(Array(30).fill('1234567890')));
fs.mkdirSync(longPath, { recursive: true });

// Test if we can have symlinks to files and folders with long filenames
const targetDirtectory = path.join(longPath, 'target-directory');
fs.mkdirSync(targetDirtectory);
const pathDirectory = path.join(tmpDir, 'new-directory');
fs.symlink(targetDirtectory, pathDirectory, 'dir', common.mustCall((err) => {
  assert.ifError(err);
  assert(fs.existsSync(pathDirectory));
}));

const targetFile = path.join(longPath, 'target-file');
fs.writeFileSync(targetFile, 'data');
const pathFile = path.join(tmpDir, 'new-file');
fs.symlink(targetFile, pathFile, common.mustCall((err) => {
  assert.ifError(err);
  assert(fs.existsSync(pathFile));
}));
