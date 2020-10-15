'use strict';

if (!process.features.inspector) return;

const common = require('../common');
const assert = require('assert');
const { dirname } = require('path');
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const tmpdir = require('../common/tmpdir');
tmpdir.refresh();

let dirc = 0;
function nextdir() {
  return process.env.NODE_V8_COVERAGE ||
    path.join(tmpdir.path, `source_map_${++dirc}`);
}

// Outputs source maps when event loop is drained, with no async logic.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/basic')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  if (output.status !== 0) {
    console.log(output.stderr.toString());
  }
  assert.strictEqual(output.status, 0);
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache('basic.js', coverageDirectory);
  assert.strictEqual(sourceMap.url, 'https://ci.nodejs.org/418');
}

// Outputs source maps when process.kill(process.pid, "SIGINT"); exits process.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/sigint')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  if (!common.isWindows) {
    if (output.signal !== 'SIGINT') {
      console.log(output.stderr.toString());
    }
    assert.strictEqual(output.signal, 'SIGINT');
  }
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache('sigint.js', coverageDirectory);
  assert.strictEqual(sourceMap.url, 'https://ci.nodejs.org/402');
}

// Outputs source maps when source-file calls process.exit(1).
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/exit-1')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache('exit-1.js', coverageDirectory);
  assert.strictEqual(sourceMap.url, 'https://ci.nodejs.org/404');
}

// Outputs source-maps for esm module.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    '--no-warnings',
    require.resolve('../fixtures/source-map/esm-basic.mjs')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache('esm-basic.mjs', coverageDirectory);
  assert.strictEqual(sourceMap.url, 'https://ci.nodejs.org/405');
}

// Loads source-maps with relative path from .map file on disk.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/disk-relative-path')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.status, 0);
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache(
    'disk-relative-path.js',
    coverageDirectory
  );
  // Source-map should have been loaded from disk and sources should have been
  // rewritten, such that they're absolute paths.
  assert.strictEqual(
    dirname(
      `file://${require.resolve('../fixtures/source-map/disk-relative-path')}`),
    dirname(sourceMap.data.sources[0])
  );
}

// Loads source-maps from inline data URL.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/inline-base64.js')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.status, 0);
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache(
    'inline-base64.js',
    coverageDirectory
  );
  // base64 JSON should have been decoded, and paths to sources should have
  // been rewritten such that they're absolute:
  assert.strictEqual(
    dirname(
      `file://${require.resolve('../fixtures/source-map/inline-base64')}`),
    dirname(sourceMap.data.sources[0])
  );
}

// base64 encoding error does not crash application.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/inline-base64-type-error.js')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.status, 0);
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache(
    'inline-base64-type-error.js',
    coverageDirectory
  );

  assert.strictEqual(sourceMap.data, null);
}

// JSON error does not crash application.
{
  const coverageDirectory = nextdir();
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/inline-base64-json-error.js')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  assert.strictEqual(output.status, 0);
  assert.strictEqual(output.stderr.toString(), '');
  const sourceMap = getSourceMapFromCache(
    'inline-base64-json-error.js',
    coverageDirectory
  );

  assert.strictEqual(sourceMap.data, null);
}

// Does not apply source-map to stack trace if --experimental-modules
// is not set.
{
  const output = spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/uglify-throw.js')
  ]);
  assert.strictEqual(
    output.stderr.toString().match(/->.*uglify-throw-original\.js:5:9/),
    null
  );
  assert.strictEqual(
    output.stderr.toString().match(/->.*uglify-throw-original\.js:9:3/),
    null
  );
}

// Applies source-maps generated by uglifyjs to stack trace.
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/uglify-throw.js')
  ]);
  assert.ok(
    output.stderr.toString().match(/->.*uglify-throw-original\.js:5:9/)
  );
  assert.ok(
    output.stderr.toString().match(/->.*uglify-throw-original\.js:9:3/)
  );
}

// Applies source-maps generated by tsc to stack trace.
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/typescript-throw.js')
  ]);
  assert.ok(output.stderr.toString().match(/->.*typescript-throw\.ts:18:11/));
  assert.ok(output.stderr.toString().match(/->.*typescript-throw\.ts:24:1/));
}

// Applies source-maps generated by babel to stack trace.
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/babel-throw.js')
  ]);
  assert.ok(
    output.stderr.toString().match(/->.*babel-throw-original\.js:18:31/)
  );
}

// Applies source-maps generated by nyc to stack trace.
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/istanbul-throw.js')
  ]);
  assert.ok(
    output.stderr.toString().match(/->.*istanbul-throw-original\.js:5:9/)
  );
  assert.ok(
    output.stderr.toString().match(/->.*istanbul-throw-original\.js:9:3/)
  );
}

// Applies source-maps in esm modules to stack trace.
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/babel-esm.mjs')
  ]);
  assert.ok(
    output.stderr.toString().match(/->.*babel-esm-original\.mjs:9:29/)
  );
}

// Does not persist url parameter if source-map has been parsed.
{
  const coverageDirectory = nextdir();
  spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/inline-base64.js')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  const sourceMap = getSourceMapFromCache(
    'inline-base64.js',
    coverageDirectory
  );
  assert.strictEqual(sourceMap.url, null);
}

// Persists line lengths for in-memory representation of source file.
{
  const coverageDirectory = nextdir();
  spawnSync(process.execPath, [
    require.resolve('../fixtures/source-map/istanbul-throw.js')
  ], { env: { ...process.env, NODE_V8_COVERAGE: coverageDirectory } });
  const sourceMap = getSourceMapFromCache(
    'istanbul-throw.js',
    coverageDirectory
  );
  if (common.isWindows) {
    assert.deepStrictEqual(sourceMap.lineLengths, [1086, 31, 185, 649, 0]);
  } else {
    assert.deepStrictEqual(sourceMap.lineLengths, [1085, 30, 184, 648, 0]);
  }
}

// trace.length === 0 .
{
  const output = spawnSync(process.execPath, [
    '--enable-source-maps',
    require.resolve('../fixtures/source-map/emptyStackError.js')
  ]);

  assert.ok(
    output.stderr.toString().match('emptyStackError')
  );
}

function getSourceMapFromCache(fixtureFile, coverageDirectory) {
  const jsonFiles = fs.readdirSync(coverageDirectory);
  for (const jsonFile of jsonFiles) {
    let maybeSourceMapCache;
    try {
      maybeSourceMapCache = require(
        path.join(coverageDirectory, jsonFile)
      )['source-map-cache'] || {};
    } catch (err) {
      console.warn(err);
      maybeSourceMapCache = {};
    }
    const keys = Object.keys(maybeSourceMapCache);
    for (const key of keys) {
      if (key.includes(fixtureFile)) {
        return maybeSourceMapCache[key];
      }
    }
  }
}
