'use strict';

const {
  customInspectSymbol: kInspect,
} = require('internal/util');

const { format } = require('util');
const { Map, Symbol } = primordials;

const {
  ERR_INVALID_ARG_TYPE,
  ERR_INVALID_ARG_VALUE,
} = require('internal/errors').codes;

const kDestroy = Symbol('kDestroy');
const kHandle = Symbol('kHandle');

// Histograms are created internally by Node.js and used to
// record various metrics. This Histogram class provides a
// generally read-only view of the internal histogram.
class Histogram {
  // @lwnode
  // #handle = undefined;
  // #map = new Map();
  __handle = undefined;
  __map = new Map();
  // end @lwnode

  constructor(internal) {
    // @lwnode
    // this.#handle = internal;
    this.__handle = internal;
  }

  [kInspect]() {
    const obj = {
      min: this.min,
      max: this.max,
      mean: this.mean,
      exceeds: this.exceeds,
      stddev: this.stddev,
      percentiles: this.percentiles,
    };
    return `Histogram ${format(obj)}`;
  }

  get min() {
    // @lwnode
    return this.#handle ? this.#handle.min() : undefined;
    return this.__handle ? this.__handle.min() : undefined;
  }

  get max() {
    // @lwnode
    // return this.#handle ? this.#handle.max() : undefined;
    return this.__handle ? this.__handle.max() : undefined;
  }

  get mean() {
    // @lwnode
    // return this.#handle ? this.#handle.mean() : undefined;
    return this.__handle ? this.__handle.mean() : undefined;
  }

  get exceeds() {
    // @lwnode
    // return this.#handle ? this.#handle.exceeds() : undefined;
    return this.__handle ? this.__handle.exceeds() : undefined;
  }

  get stddev() {
    // @lwnode
    // return this.#handle ? this.#handle.stddev() : undefined;
    return this.__handle ? this.__handle.stddev() : undefined;
  }

  percentile(percentile) {
    if (typeof percentile !== 'number')
      throw new ERR_INVALID_ARG_TYPE('percentile', 'number', percentile);

    if (percentile <= 0 || percentile > 100)
      throw new ERR_INVALID_ARG_VALUE.RangeError('percentile', percentile);

    // @lwnode
    // return this.#handle ? this.#handle.percentile(percentile) : undefined;
    return this.__handle ? this.__handle.percentile(percentile) : undefined;
  }

  get percentiles() {
    // @lwnode
    // this.#map.clear();
    // if (this.#handle)
    //   this.#handle.percentiles(this.#map);
    // return this.#map;
    this.__map.clear();
    if (this.__handle)
      this.__handle.percentiles(this.__map);
    return this.__map;
    // end @lwnode
  }

  reset() {
    // @lwnode
    // if (this.#handle)
    //   this.#handle.reset();
    if (this.__handle)
      this.__handle.reset();
    // end @lwnode
  }

  [kDestroy]() {
    // @lwnode
    // this.#handle = undefined;
    this.__handle = undefined;
  }

  // @lwnode
  //get [kHandle]() { return this.#handle; }
  get [kHandle]() { return this.__handle; }
}

module.exports = {
  Histogram,
  kDestroy,
  kHandle,
};
