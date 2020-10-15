'use strict';

const common = require('../common');
const assert = require('assert');
const { AsyncResource, executionAsyncId } = require('async_hooks');

const fn = common.mustCall(AsyncResource.bind(() => {
  return executionAsyncId();
}));

setImmediate(() => {
  const asyncId = executionAsyncId();
  assert.notStrictEqual(asyncId, fn());
});

const asyncResource = new AsyncResource('test');

[1, false, '', {}, []].forEach((i) => {
  assert.throws(() => asyncResource.bind(i), {
    code: 'ERR_INVALID_ARG_TYPE'
  });
});

const fn2 = asyncResource.bind((a, b) => {
  return executionAsyncId();
});

assert.strictEqual(fn2.asyncResource, asyncResource);
assert.strictEqual(fn2.length, 2);

setImmediate(() => {
  const asyncId = executionAsyncId();
  assert.strictEqual(asyncResource.asyncId(), fn2());
  assert.notStrictEqual(asyncId, fn2());
});
