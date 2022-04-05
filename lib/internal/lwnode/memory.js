/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

const EventEmitter = require("events");
const rawMethods = internalBinding("process_methods");

function fileSize(size, { fractionalDigits = 2, signed = false } = {}) {
  let isNegative = size < 0;
  let abs = Math.abs(size);
  let i = abs == 0 ? 0 : Math.floor(Math.log(abs) / Math.log(1024));
  return (
    `${isNegative ? "-" : signed ? "+" : ""}` +
    `${(abs / Math.pow(1024, i)).toFixed(fractionalDigits)} ${
      ["B", "kB", "MB", "GB", "TB"][i]
    }`
  );
}

function formatMemoryStats(size) {
  return [fileSize(size), size];
}

// FIXME(escargot): calling isFunctionObject with a Function_instance.bind(this) is false.

function createMemoryStats(stats = rawMethods.getGCMemoryStats()) {
  return {
    heap: formatMemoryStats(stats.heapSize),
    unmapped: formatMemoryStats(stats.unmappedBytes),
    sinceLastGc: formatMemoryStats(stats.bytesSinceGC),
  };
}

class MemDiff {
  static zero = createMemoryStats({
    heapSize: 0,
    unmappedBytes: 0,
    bytesSinceGC: 0,
  });

  constructor() {
    this.last = MemDiff.zero;
    this.current = MemDiff.zero;
    this.max = MemDiff.zero;
    this.min = MemDiff.zero;
  }

  check() {
    this.last = this.current;
    this.current = createMemoryStats();
    this.max = createMemoryStats({
      heapSize: Math.max(this.current.heap[1], this.last.heap[1]),
      unmappedBytes: Math.max(this.current.unmapped[1], this.last.unmapped[1]),
      bytesSinceGC: Math.max(
        this.current.sinceLastGc[1],
        this.last.sinceLastGc[1]
      ),
    });

    const option = {
      signed: true,
      fractionalDigits: 2,
    };

    const change = {
      heap: formatMemoryStats(this.current.heap[1] - this.last.heap[1], option),
      unmapped: formatMemoryStats(
        this.current.unmapped[1] - this.last.unmapped[1],
        option
      ),
      sinceLastGc: formatMemoryStats(
        this.current.sinceLastGc[1] - this.last.sinceLastGc[1],
        option
      ),
    };

    // return user data
    return {
      last: Object.assign({}, this.last),
      current: Object.assign({}, this.current),
      change,
      max: Object.assign({}, this.max),
    };
  }
}

const kStatsTimerId = Symbol("StatsTimerId");
const kLeakTimerId = Symbol("leakTimerId");

class MemWatcher extends EventEmitter {
  constructor({ statsInternal = 5000, leakInternal = 10000 } = {}) {
    super([arguments]);

    // todo: declare private members using Symbol
    // todo: declare const members
    this.statsInternal = statsInternal;
    this.leakInternal = leakInternal;
    this.setMaxListeners(5);

    this.diff = new MemDiff();
    this.on("newListener", this.onnewListener);
    this.on("removeListener", this.onremoveListener);
  }

  onnewListener(event) {
    let count = this.listeners(event).length;

    switch (event) {
      // todo: use predefined event name
      case "stats":
        if (count == 0) {
          this.diff.check();
          this[kStatsTimerId] = setInterval(() => {
            this.emit("stats", this.diff.check());
          }, this.statsInternal);
        }
        break;
      case "leak":
        if (count == 0) {
          this[kLeakTimerId] = setInterval(() => {
            // todo: check heap allocation trend
            // todo: emit leak if leak is suspected
            this.emit("leak", null);
          }, this.leakInternal);
        }
        break;
      default:
        break;
    }
  }

  onremoveListener(event) {
    let count = this.listeners(event).length;
    switch (event) {
      case "stats":
        if (count == 0 && this[kStatsTimerId]) {
          clearInterval(this[kStatsTimerId]);
          this[kStatsTimerId] = null;
        }
        break;
      case "leak":
        if (count == 0 && this[kLeakTimerId]) {
          clearInterval(this[kLeakTimerId]);
          this[kLeakTimerId] = null;
        }
        break;
      default:
        break;
    }
  }

  end() {
    if (this[kStatsTimerId]) {
      clearInterval(this[kStatsTimerId]);
      this[kStatsTimerId] = null;
    }
    if (this[kLeakTimerId]) {
      clearInterval(this[kLeakTimerId]);
      this[kLeakTimerId] = null;
    }
  }
}

module.exports = {
  MemWatcher,
  MemDiff,
};
