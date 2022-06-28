// Flags: --expose-gc
'use strict';
const common = require('../common');
const onGC = require('../common/ongc');
const assert = require('assert');
const async_hooks = require('async_hooks');
const domain = require('domain');
const EventEmitter = require('events');
const isEnumerable = Function.call.bind(Object.prototype.propertyIsEnumerable);

// This test makes sure that the (async id → domain) map which is part of the
// domain module does not get in the way of garbage collection.
// See: https://github.com/nodejs/node/issues/23862

let d = domain.create();
d.run(() => {
  const resource = new async_hooks.AsyncResource('TestResource');
  const emitter = new EventEmitter();

  d.remove(emitter);
  d.add(emitter);

  emitter.linkToResource = resource;
  assert.strictEqual(emitter.domain, d);
  assert.strictEqual(isEnumerable(emitter, 'domain'), false);
  assert.strictEqual(resource.domain, d);
  assert.strictEqual(isEnumerable(resource, 'domain'), false);

  // This would otherwise be a circular chain now:
  // emitter → resource → async id ⇒ domain → emitter.
  // Make sure that all of these objects are released:

  onGC(resource, { ongc: common.mustCall() });
  onGC(d, { ongc: common.mustCall() });
  onGC(emitter, { ongc: common.mustCall() });
});

d = null;
global.gc();
