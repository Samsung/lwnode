# lwnode

The `lwnode` object provides features to detect the memory footprint consumed
in Javascript. It can be accessed through the `process` object.

## `process.lwnode`

* {Object}

### `class MemWatcher`

* Extends: {EventEmitter}

The class `MemWatcher` exposes an interface for tracking memory usage
asynchronously. It provides a way to evaluate the memory footprint and to catch
the trends of heap usage.

#### Example

The following illustrates a leak detection using `MemWatcher`. The example
intentionally makes memory leakage occur by wrongly using a `closure`.

```js
// test.js
const { MemWatcher } = process.lwnode;

console.warn('NOTE: This code intentionally makes leaks.');

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

const id = setInterval(closureLeak, 1000);

const watcher = new MemWatcher();

watcher.on('stats', (stats) => {
  console.table(stats);
});

watcher.on('max', (stats) => {
  console.log(stats);
  clearInterval(id);
  watcher.end();
});

// Run:
//
// $ lwnode ./test.js
//
// Print:
//
// NOTE: This code intentionally makes leaks.
// ┌─────────┬───────────────────────────┬───────────────────────────┬───────────────────────────┐
// │ (index) │           heap            │        sinceLastGc        │         unmapped          │
// ├─────────┼───────────────────────────┼───────────────────────────┼───────────────────────────┤
// │   max   │ [ '80.09 MB', 83976192 ]  │ [ '979.34 kB', 1002848 ]  │ [ '46.84 MB', 49115136 ]  │
// │  last   │      [ '0.00 B', 0 ]      │      [ '0.00 B', 0 ]      │      [ '0.00 B', 0 ]      │
// │ current │ [ '80.09 MB', 83976192 ]  │ [ '979.34 kB', 1002848 ]  │ [ '46.84 MB', 49115136 ]  │
// │ change  │ [ '+80.09 MB', 83976192 ] │ [ '+979.34 kB', 1002848 ] │ [ '+46.84 MB', 49115136 ] │
// └─────────┴───────────────────────────┴───────────────────────────┴───────────────────────────┘
//
// ...
//
// {
//   reason: 'Max value growth occurred 2 times over 10s',
//   change: [ '+10.19 MB', 10686464 ],
//   currentMax: [ '90.28 MB', 94662656 ]
// }

```

#### `new lwnode.MemWatcher([interval][, maxIgnoreCount][, limit][, memTarget])`

* `interval` {number} The time, in milliseconds (thousandths of a second), that the
`stats` event should delay between measurements. Defaults to 5000 if not
specified.
* `maxIgnoreCount` {number} The count of how many times are skipped to emit the
`max` event. For instance, saying it's set to 2, the first detection the `max`
heap exceeded is ignored.
* `limit` {number} A heap threshold size in bytes to monitor.
* `memType` {'String'} `pss` or `gc`. A memory type to monitor. Defaults to
`pss` if not specified.
* Returns: {lwnode.MemWatcher}

`lwnode.MemWatcher` is an [`EventEmitter`][] with the following events:

##### Event: `'stats'`

* `stats` {MemoryStats Object}

The `'stats'` event is emitted every delayed milliseconds that is given when a
instance of `MemWatcher` is created, giving you the data describing your heap
usage and trends over time.

```js
const { MemWatcher } = require('process').lwnode;

const watcher = new MemWatcher();
watcher.on('stats', (stats) => {
  console.log(stats);
});
```

##### Event: `'max'`

* `stats` {MemoryMaxStats Object}

The `'max'` event is emitted when the max heap usage exceeds the previous max
amount recorded. It can be skipped by setting the `maxIgnoreCount` when an
instance of `MemWatcher` is created.

```js
const { MemWatcher } = require('process').lwnode;

const watcher = new MemWatcher();
watcher.on('max', (stats) => {
  console.log(stats);
});
```

##### Event: `'limit'`

* `stats` {MemoryMaxStats Object}

The `'limit'` event is emitted when the current heap usage exceeds over a
threshold size passed at the instantiation of `MemWatcher`.

```js
const { MemWatcher } = require('process').lwnode;

const watcher = new MemWatcher({ limit: 1024 * 1024 * 4 });
watcher.on('limit', (stats) => {
  console.log(stats);
});
```

###### Example
```js
// test.js
const { MemWatcher } = process.lwnode;
const { clearLine, cursorTo } = require('readline');

let strStat = '', strLimit = '';

function startMemoryTest() {
  let array = [];
  let increment = true;
  let idx = 0,
    count = 1;
  const times = 3;

  console.log(`Limit: ${limit / 1024 / 1024} MB`);
  function display() {
    if (strStat === '') return;
    const strStatus = `${increment ? '[↑ increment]' : '[↓ decrement]'}`;
    clearLine(process.stdout, 0);
    cursorTo(process.stdout, 0);
    process.stdout.write(
      `#${++idx}/${times * 10} ${strStatus} ${strStat} ${strLimit}`,
    );
    strStat = strLimit = '';
  }

  const id1 = setInterval(() => {
    display();
    if (count > times) return teardown();
    increment ? array.push(new Array(80000)) : (array = []);
  }, 1000);

  const id2 = setInterval(() => {
    increment = !increment;
    ++count;
  }, 10000);

  function teardown() {
    clearInterval(id1);
    clearInterval(id2);
    watcher.end();
    console.log('\ndone.');
  }
}

// Set a watcher
const limit = 1024 * 1024 * 15;
const watcher = new MemWatcher({ delay: 1000, limit });

// Set event listeners
watcher.on('limit', (stats) => {
  strLimit = `(! ${stats.gap[0]} exceeds)`;
});

watcher.on('stats', (stats) => {
  strStat = `PSS: ${stats.current.pssSwap[0]}`;
});

startMemoryTest();

// Run:
//
// $ lwnode ./test.js
//
// Print:
//
// Limit: 15 MB
// #30/30 [↓ decrement] PSS: 16.17 MB (! 1.17 MB exceeds)
// done.
```

##### `memwatcher.end`

Calling the `memwatcher.end()` method stop receiving the `stats` event or the
`max` event.

#### MemoryStats Object

This object is an ordinary JavaScript object containing the following
properties.

* `max` {MemoryInfo Object} Indicates the maximum heap memory measured so far.
* `last` {MemoryInfo Object} Indicates the heap memory previously measured.
* `current` {MemoryInfo Object} Indicates the heap memory currently measured.
* `change` {MemoryInfo Object} Indicates the difference between the `last` and
`current` objects.

#### MemoryInfo Object

This object is an ordinary JavaScript object containing the following
properties.

The each property has a description of the measured memory which are in `Array`
format. The index, `0`, of the array has a string in human-readable format. The
index, `1`, has a number representing the change in bytes. The string in the `0`
index translates this number into KB, MB, and so on.

* `pssSwap` {Array} Indicates the `Proportional Set Size` of `lwnode` process. It includes the swap memory.
* `heap` {Array} Indicates the number of memory used in the heap. It excludes
the unmapped memory (returned to the OS). This is managed by the JavaScript VM except for using an external allocator. (e.g ArrayBuffer)
* `unmapped` {Array} Indicates the size of the unmapped memory (which is
returned to the OS but could be remapped back by the collector later unless
the OS runs out of system/virtual memory). This is managed by the JavaScript VM except for using an external allocator.
* `sinceLastGc` {Array} Indicates the number of bytes changed since the last
GC collection. This is managed by the JavaScript VM except for using an external allocator.

#### MemoryMaxStats Object

This object is an ordinary JavaScript object containing the following
properties.

Except the `reason` propery, each property has a description of the measured
memory which are in `Array` format. The index, `0`, of the array has a string
in human-readable format. The index, `1`, has a number representing the change
in bytes. The string in the `0` index translates this number into KB, MB, and
so on.

* `reason` {string} Indicates what's the reason why the `max` event is emitted.
* `change` {Array} Indicates the difference compared between the number last
measured and the current one.
* `currentMax` {Array} Indicates the max heap memory currently measured.

#### MemoryLimitStats Object

This object is an ordinary JavaScript object containing the following
properties.

Except the `reason` propery, each property has a description of the measured
memory which are in `Array` format. The index, `0`, of the array has a string
in human-readable format. The index, `1`, has a number representing the change
in bytes. The string in the `0` index translates this number into KB, MB, and
so on.

* `reason` {string} Indicates what's the reason why the `limit` event is emitted.
* `current` {string} Indicates the heap memory currently measured.
* `gap` {Array} Indicates the difference between the current value and the threshold value.
* `limit` {Array} Indicates the threshold value passed at the instantiation of
`MemWatcher`.
