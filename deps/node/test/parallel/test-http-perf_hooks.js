'use strict';

const common = require('../common');
const assert = require('assert');
const http = require('http');

const { PerformanceObserver } = require('perf_hooks');

const obs = new PerformanceObserver(common.mustCall((items) => {
  const entry = items.getEntries()[0];
  assert.strictEqual(entry.entryType, 'http');
  assert.strictEqual(typeof entry.startTime, 'number');
  assert.strictEqual(typeof entry.duration, 'number');
}, 2));

obs.observe({ entryTypes: ['http'] });

const expected = 'Post Body For Test';
const makeRequest = (options) => {
  return new Promise((resolve, reject) => {
    http.request(options, common.mustCall((res) => {
      resolve();
    })).on('error', reject).end(options.data);
  });
};

const server = http.Server(common.mustCall((req, res) => {
  let result = '';

  req.setEncoding('utf8');
  req.on('data', function(chunk) {
    result += chunk;
  });

  req.on('end', common.mustCall(function() {
    assert.strictEqual(result, expected);
    res.writeHead(200);
    res.end('hello world\n');
  }));
}, 2));

server.listen(0, common.mustCall(async () => {
  await Promise.all([
    makeRequest({
      port: server.address().port,
      path: '/',
      method: 'POST',
      data: expected
    }),
    makeRequest({
      port: server.address().port,
      path: '/',
      method: 'POST',
      data: expected
    })
  ]);
  server.close();
}));
