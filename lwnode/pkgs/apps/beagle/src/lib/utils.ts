import os from 'os';

const units = ['h', 'm', 's', 'ms', 'μs', 'ns'];
const divisors = [60 * 60 * 1000, 60 * 1000, 1000, 1, 1e-3, 1e-6];

const utils = {
  isFunction(fn) {
    return typeof fn === 'function';
  },

  isString(s) {
    return typeof s === 'string' || s instanceof String;
  },

  isObject(o) {
    return o !== null && typeof o === 'object' && !(o instanceof String);
  },

  isPlainObject(o) {
    return o != null
      ? Object.getPrototypeOf(o) === Object.prototype ||
          Object.getPrototypeOf(o) === null
      : false;
  },

  isDate(d) {
    return d instanceof Date && !Number.isNaN(d.getTime());
  },

  flatten(arr) {
    return Array.prototype.reduce.call(arr, (a, b) => a.concat(b), []);
  },

  humanize(milli) {
    if (milli == null) return '?';

    for (let i = 0; i < divisors.length; i++) {
      const val = milli / divisors[i];
      if (val >= 1.0) return '' + Math.floor(val) + units[i];
    }

    return 'now';
  },

  getNodeID(): string {
    return os.hostname().toLowerCase() + '-' + process.pid;
  },
};

export default utils;
