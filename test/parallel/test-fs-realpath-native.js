'use strict';
const common = require('../common');
const assert = require('assert');
const fs = require('fs');

// @lwnode (daeyeon)
// fixed: this assumes the current working directory is always the root.
const filename = __filename.toLowerCase();
const relativePath = filename.replace(process.cwd(), '.');

assert.strictEqual(
  fs.realpathSync.native(relativePath).toLowerCase(),
  filename,
);

fs.realpath.native(
  relativePath,
  common.mustCall(function (err, res) {
    assert.ifError(err);
    assert.strictEqual(res.toLowerCase(), filename);
    assert.strictEqual(this, undefined);
  }),
);
