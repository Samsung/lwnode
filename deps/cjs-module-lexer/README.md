# CJS Module Lexer

[![Build Status][travis-image]][travis-url]

A [very fast](#benchmarks) JS CommonJS module syntax lexer used to detect the most likely list of named exports of a CommonJS module.

Outputs the list of named exports (`exports.name = ...`) and possible module reexports (`module.exports = require('...')`), including the common transpiler variations of these cases.

Forked from https://github.com/guybedford/es-module-lexer.

_Comprehensively handles the JS language grammar while remaining small and fast. - ~90ms per MB of JS cold and ~15ms per MB of JS warm, [see benchmarks](#benchmarks) for more info._

### Usage

```
npm install cjs-module-lexer
```

For use in CommonJS:

```js
const parse = require('cjs-module-lexer');

const { exports, reexports } = parse(`
  // named exports detection
  module.exports.a = 'a';
  (function () {
    exports.b = 'b';
  })();
  Object.defineProperty(exports, 'c', { value: 'c' });
  /* exports.d = 'not detected'; */

  // reexports detection
  if (maybe) module.exports = require('./dep1.js');
  if (another) module.exports = require('./dep2.js');

  // literal exports assignments
  module.exports = { a, b: c, d, 'e': f }

  // __esModule detection
  Object.defineProperty(module.exports, '__esModule', { value: true })
`);

// exports === ['a', 'b', 'c', '__esModule']
// reexports === ['./dep1.js', './dep2.js']
```

When using the ESM version, Wasm is supported instead:

```js
import { parse, init } from 'cjs-module-lexer';
// init needs to be called and waited upon
await init();
const { exports, reexports } = parse(source);
```

The Wasm build is around 1.5x faster and without a cold start.

### Grammar

CommonJS exports matches are run against the source token stream.

The token grammar is:

```
IDENTIFIER: As defined by ECMA-262, without support for identifier `\` escapes, filtered to remove strict reserved words:
            "implements", "interface", "let", "package", "private", "protected", "public", "static", "yield", "enum"

STRING_LITERAL: A `"` or `'` bounded ECMA-262 string literal.

IDENTIFIER_STRING: ( `"` IDENTIFIER `"` | `'` IDENTIFIER `'` )

COMMENT_SPACE: Any ECMA-262 whitespace, ECMA-262 block comment or ECMA-262 line comment

MODULE_EXPORTS: `module` COMMENT_SPACE `.` COMMENT_SPACE `exports`

EXPORTS_IDENTIFIER: MODULE_EXPORTS_IDENTIFIER | `exports`

EXPORTS_DOT_ASSIGN: EXPORTS_IDENTIFIER COMMENT_SPACE `.` COMMENT_SPACE IDENTIFIER COMMENT_SPACE `=`

EXPORTS_LITERAL_COMPUTED_ASSIGN: EXPORTS_IDENTIFIER COMMENT_SPACE `[` COMMENT_SPACE IDENTIFIER_STRING COMMENT_SPACE `]` COMMENT_SPACE `=`

EXPORTS_LITERAL_PROP: (IDENTIFIER (COMMENT_SPACE `:` COMMENT_SPACE IDENTIFIER)?) | (IDENTIFIER_STRING COMMENT_SPACE `:` COMMENT_SPACE IDENTIFIER)

EXPORTS_MEMBER: EXPORTS_DOT_ASSIGN | EXPORTS_LITERAL_COMPUTED_ASSIGN

EXPORTS_DEFINE: `Object` COMMENT_SPACE `.` COMMENT_SPACE `defineProperty COMMENT_SPACE `(` EXPORTS_IDENTIFIER COMMENT_SPACE `,` COMMENT_SPACE IDENTIFIER_STRING

EXPORTS_LITERAL: MODULE_EXPORTS COMMENT_SPACE `=` COMMENT_SPACE `{` COMMENT_SPACE (EXPORTS_LITERAL_PROP COMMENT_SPACE `,` COMMENT_SPACE)+ `}`

REQUIRE: `require` COMMENT_SPACE `(` COMMENT_SPACE STRING_LITERAL COMMENT_SPACE `)`

EXPORTS_ASSIGN: (`var` | `const` | `let`) IDENTIFIER `=` REQUIRE

MODULE_EXPORTS_ASSIGN: MODULE_EXPORTS COMMENT_SPACE `=` COMMENT_SPACE REQUIRE

EXPORT_STAR: (`__export` | `__exportStar`) `(` REQUIRE

EXPORT_STAR_LIB: `Object.keys(` IDENTIFIER$1 `).forEach(function (` IDENTIFIER$2 `) {`
  (
    `if (` IDENTIFIER$2 `===` ( `'default'` | `"default"` ) `||` IDENTIFIER$2 `===` ( '__esModule' | `"__esModule"` ) `) return` `;`? |
    `if (` IDENTIFIER$2 `!==` ( `'default'` | `"default"` ) `)`
  )
  (
    EXPORTS_IDENTIFIER `[` IDENTIFIER$2 `] =` IDENTIFIER$1 `[` IDENTIFIER$2 `]` `;`? |
    `Object.defineProperty(` EXPORTS_IDENTIFIER `, ` IDENTIFIER$2 `, { enumerable: true, get: function () { return ` IDENTIFIER$1 `[` IDENTIFIER$2 `]` `;`? } })` `;`?
  )
  `})`
```

* The returned export names are the matched `IDENTIFIER` and `IDENTIFIER_STRING` slots for all `EXPORTS_MEMBER`, `EXPORTS_DEFINE` and `EXPORTS_LITERAL` matches.
* The reexport specifiers are taken to be the `STRING_LITERAL` slots of all `MODULE_EXPORTS_ASSIGN` as well as all _top-level_ `EXPORT_STAR` `REQUIRE` matches and `EXPORTS_ASSIGN` matches whose `IDENTIFIER` also matches the first `IDENTIFIER` in `EXPORT_STAR_LIB`.

### Parsing Examples

#### Named Exports Parsing

The basic matching rules for named exports are `exports.name`, `exports['name']` or `Object.defineProperty(exports, 'name', ...)`. This matching is done without scope analysis and regardless of the expression position:

```js
// DETECTS EXPORTS: a, b, c
(function (exports) {
  exports.a = 'a'; 
  exports['b'] = 'b';
  Object.defineProperty(exports, 'c', { value: 'c' });
})(exports);
```

Because there is no scope analysis, the above detection may overclassify:

```js
// DETECTS EXPORTS: a, b, c
(function (exports, Object) {
  exports.a = 'a';
  exports['b'] = 'b';
  if (false)
    Object.defineProperty(exports, 'c', { value: 'c' });
})(NOT_EXPORTS, NOT_OBJECT);
```

It will in turn underclassify in cases where the identifiers are renamed:

```js
// DETECTS: NO EXPORTS
(function (e, defineProperty) {
  e.a = 'a';
  e['b'] = 'b';
  defineProperty(e, 'c', { value: 'c' });
})(exports, defineProperty);
```

#### Exports Object Assignment

A best-effort is made to detect `module.exports` object assignments, but because this is not a full parser, arbitrary expressions are not handled in the
object parsing process.

Simple object definitions are supported:

```js
// DETECTS EXPORTS: a, b, c
module.exports = {
  a,
  b: 'c',
  c: c
};
```

Object properties that are not identifiers or string expressions will bail out of the object detection:

```js
// DETECTS EXPORTS: a, b
module.exports = {
  a,
  b: require('c'),
  c: "not detected since require('c') above bails the object detection"
}
```

`Object.defineProperties` is not currently supported either.

#### module.exports reexport assignment

Any `module.exports = require('mod')` assignment is detected as a reexport:

```js
// DETECTS REEXPORTS: a, b, c
module.exports = require('a');
(module => module.exports = require('b'))(NOT_MODULE);
if (false) module.exports = require('c');
```

As a result, the total list of exports would be inferred as the union of all of these reexported modules, which can lead to possible over-classification.

#### Transpiler Re-exports

For named exports, transpiler output works well with the rules described above.

But for star re-exports, special care is taken to support common patterns of transpiler outputs from Babel and TypeScript as well as bundlers like RollupJS.
These reexport and star reexport patterns are restricted to only be detected at the top-level as provided by the direct output of these tools.

For example, `export * from 'external'` is output by Babel as:

```js
"use strict";

exports.__esModule = true;

var _external = require("external");

Object.keys(_external).forEach(function (key) {
  if (key === "default" || key === "__esModule") return;
  exports[key] = _external[key];
});
```

Where the `var _external = require("external")` is specifically detected as well as the `Object.keys(_external)` statement, down to the exact
for of that entire expression including minor variations of the output. The `_external` and `key` identifiers are carefully matched in this
detection.

Similarly for TypeScript, `export * from 'external'` is output as:

```js
"use strict";
function __export(m) {
    for (var p in m) if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}
Object.defineProperty(exports, "__esModule", { value: true });
__export(require("external"));
```

Where the `__export(require("external"))` statement is explicitly detected as a reexport, including variations `tslib.__export` and `__exportStar`.

### Environment Support

Node.js 10+, and [all browsers with Web Assembly support](https://caniuse.com/#feat=wasm).

### JS Grammar Support

* Token state parses all line comments, block comments, strings, template strings, blocks, parens and punctuators.
* Division operator / regex token ambiguity is handled via backtracking checks against punctuator prefixes, including closing brace or paren backtracking.
* Always correctly parses valid JS source, but may parse invalid JS source without errors.

### Benchmarks

Benchmarks can be run with `npm run bench`.

Current results:

JS Build:

```
Module load time
> 2ms
Cold Run, All Samples
test/samples/*.js (3635 KiB)
> 333ms

Warm Runs (average of 25 runs)
test/samples/angular.js (1410 KiB)
> 16.48ms
test/samples/angular.min.js (303 KiB)
> 5.36ms
test/samples/d3.js (553 KiB)
> 8.32ms
test/samples/d3.min.js (250 KiB)
> 4.28ms
test/samples/magic-string.js (34 KiB)
> 1ms
test/samples/magic-string.min.js (20 KiB)
> 0.36ms
test/samples/rollup.js (698 KiB)
> 10.48ms
test/samples/rollup.min.js (367 KiB)
> 6.64ms

Warm Runs, All Samples (average of 25 runs)
test/samples/*.js (3635 KiB)
> 49.28ms
```

Wasm Build:
```
Module load time
> 11ms
Cold Run, All Samples
test/samples/*.js (3635 KiB)
> 48ms

Warm Runs (average of 25 runs)
test/samples/angular.js (1410 KiB)
> 12.32ms
test/samples/angular.min.js (303 KiB)
> 3.76ms
test/samples/d3.js (553 KiB)
> 6.08ms
test/samples/d3.min.js (250 KiB)
> 3ms
test/samples/magic-string.js (34 KiB)
> 0.24ms
test/samples/magic-string.min.js (20 KiB)
> 0ms
test/samples/rollup.js (698 KiB)
> 7.2ms
test/samples/rollup.min.js (367 KiB)
> 4.2ms

Warm Runs, All Samples (average of 25 runs)
test/samples/*.js (3635 KiB)
> 33.6ms
```

### Wasm Build Steps

To build download the WASI SDK from https://github.com/CraneStation/wasi-sdk/releases.

The Makefile assumes the existence of "wasi-sdk-10.0", "binaryen" and "wabt" (both optional) as sibling folders to this project.

The build through the Makefile is then run via `make lib/lexer.wasm`, which can also be triggered via `npm run build-wasm` to create `dist/lexer.js`.

On Windows it may be preferable to use the Linux subsystem.

After the Web Assembly build, the CJS build can be triggered via `npm run build`.

Optimization passes are run with [Binaryen](https://github.com/WebAssembly/binaryen) prior to publish to reduce the Web Assembly footprint.

### License

MIT

[travis-url]: https://travis-ci.org/guybedford/es-module-lexer
[travis-image]: https://travis-ci.org/guybedford/es-module-lexer.svg?branch=master
