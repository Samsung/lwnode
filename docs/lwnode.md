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

#### `new lwnode.MemWatcher([interval][, maxIgnoreCount])`

* `interval` {number} The time, in milliseconds (thousandths of a second), that the
`stats` event should delay between measurements. Defaults to 5000 if not
specified.
* `maxIgnoreCount` {number} The count of how many times are skipped to emit the
`max` event. For instance, saying it's set to 2, the first detection the `max`
heap exceeded is ignored.
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

##### `memwatcher.end`

Calling the `memwatcher.end()` method stop receiving the `stats` event or the
`max` event.

#### MemoryStats Object

This object is an ordinary JavaScript object containing the following
properties.

* `max` {GC Stats Object} Indicates the maximum heap memory measured so far.
* `last` {GC Stats Object} Indicates the heap memory previously measured.
* `current` {GC Stats Object} Indicates the heap memory currently measured.
* `change` {GC Stats Object} Indicates the difference between the `last` and
`current` objects.

#### GC Stats Object

This object is an ordinary JavaScript object containing the following
properties.

The each property has a description of the measured memory which are in `Array`
format. The index, `0`, of the array has a string in human-readable format. The
index, `1`, has a number representing the change in bytes. The string in the `0`
index translates this number into KB, MB, and so on.

* `heap` {Array} Indicates the number of memory used in the heap. It excludes
the unmapped memory (returned to the OS).
* `unmapped` {Array} Indicates the size of the unmapped memory (which is
returned to the OS but could be remapped back by the collector later unless
the OS runs out of system/virtual memory).
* `sinceLastGc` {Array} Indicates the number of bytes changed since the last
GC collection.

#### MemoryMaxStats Object

This object is an ordinary JavaScript object containing the following
properties.

Expect the `reason` propery, each property has a description of the measured
memory which are in `Array` format. The index, `0`, of the array has a string
in human-readable format. The index, `1`, has a number representing the change
in bytes. The string in the `0` index translates this number into KB, MB, and
so on.

* `reason` {string} Indicates what's the reason why the `max` event is emitted.
* `change` {Array} Indicates the difference compared between the number last
measured and the current one.
* `currentMax` {Array} Indicates the max heap memory currently measured.
