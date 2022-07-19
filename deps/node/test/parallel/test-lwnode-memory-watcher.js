'use strict';

const common = require('../common');
const assert = require('assert');

if (!process.lwnode) common.skip("`process.lwnode` doesn't exist");

const { MemWatcher, MemDiff } = process.lwnode;

// diff
const diff = new MemDiff();

checkDiffProps(diff.check());

function checkDiffProps(r) {
  for (const v of Object.values(r)) {
    for (const k of ['heap', 'sinceLastGc', 'unmapped']) {
      assert.equal(Array.isArray(v[k]), true);
      assert.equal(v[k].length, 2);
    }
  }
}

// watcher
const watcher = new MemWatcher();

watcher.on(
  'stats',
  common.mustCallAtLeast((stats) => {
    checkDiffProps(stats);
  }, 1),
);

watcher.on(
  'max',
  common.mustCall((stats) => {
    teardown();
  }),
);

const idi = setInterval(makesLeaks(), 100);
const idt = setTimeout(teardown, common.platformTimeout(1000 * 60));

function teardown() {
  watcher.end();
  clearInterval(idi);
  clearTimeout(idt);
}

function makesLeaks() {
  let theThing = null;
  let closureLeak = function () {
    let originalThing = theThing; // leak
    let unused = function () {
      if (originalThing) console.log('hi');
    };
    theThing = {
      longStr: new Array(1000000).join('*'), // leak
      someMethod: function () {},
    };
  };
  return closureLeak;
}
