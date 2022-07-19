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

const EventEmitter = require('events');
const { validateCallback, validateInteger } = require('internal/validators');
const rawMethods = internalBinding('process_methods');

/**
 * @typedef {object} FileSizeOption
 * @property {number} [fractionalDigits=2]
 * @property {boolean} [signed=false]
 */

/**
 * @typedef {object} LeakRecord
 * @property {Date} time
 * @property {MemoryStats} stats
 */

/** @type {number} msec time for idle GC interval time */
const kGcInterval = 5000;

/**
 * Converts a number into a human-readable string
 * @param {number} size number
 * @param {FileSizeOption?} options
 * @return {string} human-readable string
 */
function getHumanSize(size, { fractionalDigits = 2, signed = false } = {}) {
  let isNegative = size < 0;
  let abs = Math.abs(size);
  let i = abs == 0 ? 0 : Math.floor(Math.log(abs) / Math.log(1024));
  return (
    `${isNegative ? '-' : signed ? '+' : ''}` +
    `${(abs / Math.pow(1024, i)).toFixed(fractionalDigits)} ${
      ['B', 'kB', 'MB', 'GB', 'TB'][i]
    }`
  );
}

/**
 * @param {number} size number
 * @param {FileSizeOption?} options
 * @return {[string,number]} [human-readable string, the given size]
 */
function formatMemStats(size, option) {
  return [getHumanSize(size, option), size];
}

/**
 * @typedef {object} MemoryStats
 * @property {[string,number]} heap
 * @property {[string,number]} unmapped
 * @property {[string,number]} sinceLastGc
 */

/**
 * @param {{
 *  heapSize: number,
 *  bytesSinceGC: number,
 *  unmappedBytes: number,
 *  }} [stats=] if undefined, gc stats from native bindings is used.
 * @return {MemoryStats}
 */
function createMemStats(stats = rawMethods.getGCMemoryStats()) {
  return {
    heap: formatMemStats(stats.heapSize),
    sinceLastGc: formatMemStats(stats.bytesSinceGC),
    unmapped: formatMemStats(stats.unmappedBytes),
  };
}

class MemDiff {
  #last;
  #cur;
  #max;

  /** @type {MemoryStats} */
  static zero = createMemStats({
    heapSize: 0,
    unmappedBytes: 0,
    bytesSinceGC: 0,
  });

  constructor() {
    /** @type {MemoryStats} */
    this.#last = MemDiff.zero;
    /** @type {MemoryStats} */
    this.#cur = MemDiff.zero;
    /** @type {MemoryStats} */
    this.#max = MemDiff.zero;
  }

  /**
   * check the current stats and return the result
   * @return {CheckedResult}
   */
  check() {
    this.#last = this.#cur;
    this.#cur = createMemStats();
    this.#max = createMemStats({
      heapSize: Math.max(this.#cur.heap[1], this.#max.heap[1]),
      unmappedBytes: Math.max(this.#cur.unmapped[1], this.#max.unmapped[1]),
      bytesSinceGC: Math.max(
        this.#cur.sinceLastGc[1],
        this.#max.sinceLastGc[1],
      ),
    });

    /** @type {FileSizeOption} */
    const heap = this.#cur.heap[1] - this.#last.heap[1];
    const unmapped = this.#cur.unmapped[1] - this.#last.unmapped[1];
    const sinceLastGc = this.#cur.sinceLastGc[1] - this.#last.sinceLastGc[1];
    const option = { signed: true, fractionalDigits: 2 };

    /**
     * @typedef {object} CheckedResult
     * @property {MemoryStats} max
     * @property {MemoryStats} last
     * @property {MemoryStats} current
     * @property {MemoryStats} change
     */
    return {
      max: Object.assign({}, this.#max),
      last: Object.assign({}, this.#last),
      current: Object.assign({}, this.#cur),
      change: {
        heap: formatMemStats(heap, option),
        unmapped: formatMemStats(unmapped, option),
        sinceLastGc: formatMemStats(sinceLastGc, option),
      },
    };
  }
}

/**
 * @typedef {object} TrackRecord
 * @property {number} value
 * @property {Date} time
 */

/**
 * @callback OnMaxUpdated
 * @param {{
 *  growth: number,
 *  current: number,
 *  count: number,
 *  elapsed: number,
 * }}
 */

class MaxValueTracker {
  #onMaxUpdated;
  #maxIgnoreCount;
  #records;
  #count;
  /**
   * @param {number} maxIgnoreCount
   * @param {{ onMaxUpdated: OnMaxUpdated }}
   */
  constructor(maxIgnoreCount = 2, { onMaxUpdated }) {
    if (typeof onMaxUpdated != 'function') {
      throw new TypeError('emitter');
    }
    /** @type {OnMaxUpdated} */
    this.#onMaxUpdated = onMaxUpdated;
    /** @type {number} */
    this.#maxIgnoreCount = Math.max(maxIgnoreCount, 2);
    /** @type {TrackRecord[]} */
    this.#records = [];
    /** @type {number} */
    this.#count = 0;
  }

  /**
   * @param {number} current a value to update
   */
  update(current) {
    if (this.#records.length == 0) {
      this.#addRecord(current);
      return;
    }

    const { value: last } = this.#records.slice(-1)[0];

    if (current > last) {
      if (++this.#count >= this.#maxIgnoreCount) {
        const now = new Date();
        const { value: base, time: basetime } = this.#records[0];
        const growth = current - base;
        const elapsed = Math.round((now - basetime) / 1000);

        this.#onMaxUpdated({ growth, current, count: this.#count, elapsed });

        this.reset();
        this.#addRecord(current, now);
      } else {
        this.#addRecord(current);
      }
    }
  }

  reset() {
    this.#records = [];
    this.#count = 0;
  }

  #addRecord(value, time = new Date()) {
    this.#records.push({ value, time });
  }
}

/**
 * @callback OnLimitUpdated
 * @param {{
 *  current: number,
 *  limit: number,
 * }}
 */

class LimitValueTracker {
  #onLimitUpdated;
  #limit;
  /**
   * @param {number} limit
   * @param {{ onLimitUpdated: OnLimitUpdated }}
   */
  constructor(limit, { onLimitUpdated }) {
    validateInteger(limit);
    validateCallback(onLimitUpdated);

    /** @type {number} */
    this.#limit = limit;

    /** @type {OnLimitUpdated} */
    this.#onLimitUpdated = onLimitUpdated;
  }

  /**
   * @param {number} current a value to update
   */
  update(current) {
    if (current > this.#limit) {
      this.#onLimitUpdated({ current, limit: this.#limit });
    }
  }
}

class MemWatcher extends EventEmitter {
  #statsTimerId;
  #delay;
  #diff;
  #maxTracker;
  #limitTracker;

  /**
   * @param {number} delay heap size (bytes)
   * @param {EventEmitter} emitter
   */
  constructor({ delay = kGcInterval, maxIgnoreCount = 2, limit } = {}) {
    super([arguments]);

    this.#delay = delay;
    this.#statsTimerId = null;
    this.#diff = new MemDiff();

    this.#maxTracker = new MaxValueTracker(maxIgnoreCount, {
      onMaxUpdated: ({ growth, current, count, elapsed }) => {
        this.emit('max', {
          reason: `Max value growth occurred ${count} times over ${elapsed}s`,
          change: [getHumanSize(growth, { signed: true }), growth],
          currentMax: [getHumanSize(current), current],
        });
      },
    });

    if (limit !== undefined) {
      this.#limitTracker = new LimitValueTracker(limit, {
        onLimitUpdated: ({ current, limit }) => {
          const gap = current - limit;
          const data = {
            // reason: `Current memory usage exceeds the threshold value.`,
            gap: [getHumanSize(gap, { signed: true }), gap],
            current: [getHumanSize(current), current],
            limit: [getHumanSize(limit), limit],
          };
          data.reason =
            `Current memory usage (${data.current[0]}) exceeds ` +
            `(${data.gap[0]}) over the threshold value (${data.limit[0]}).`;
          this.emit('limit', data);
        },
      });
    }

    this.setMaxListeners(5);
    this.on('newListener', this.#onnewListener);
    this.on('removeListener', this.#onremoveListener);
  }

  end() {
    if (this.#statsTimerId) {
      clearInterval(this.#statsTimerId);
      this.#statsTimerId = null;
    }
  }

  #hasListener() {
    return ['stats', 'max', 'limit'].some(
      (event) => this.listeners(event).length > 0,
    );
  }

  #onnewListener(event) {
    switch (event) {
      case 'max':
      case 'stats':
      case 'limit':
        if (this.#hasListener() == false) {
          this.#statsTimerId = setInterval(() => {
            let result = this.#diff.check();
            this.emit('stats', result);

            if (this.listeners('limit').length) {
              this.#limitTracker?.update(result.current.heap[1]);
            }

            if (this.listeners('max').length) {
              this.#maxTracker?.update(result.max.heap[1]);
            }
          }, this.#delay);
        }
        break;
      default:
        break;
    }
  }

  #onremoveListener(event) {
    switch (event) {
      case 'max':
        this.#maxTracker?.reset();
      // fall-through
      case 'limit':
        this.#limitTracker?.reset();
      // fall-through
      case 'stats':
        if (this.#hasListener() == false) {
          this.end();
        }
        break;
      default:
        break;
    }
  }
}

module.exports = {
  MemWatcher,
  MemDiff,
};
