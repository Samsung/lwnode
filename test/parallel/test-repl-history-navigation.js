'use strict';

// Flags: --expose-internals

const common = require('../common');
const stream = require('stream');
const REPL = require('internal/repl');
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const { inspect } = require('util');

common.skipIfDumbTerminal();

const tmpdir = require('../common/tmpdir');
tmpdir.refresh();

process.throwDeprecation = true;

const defaultHistoryPath = path.join(tmpdir.path, '.node_repl_history');

// Create an input stream specialized for testing an array of actions
class ActionStream extends stream.Stream {
  run(data) {
    const _iter = data[Symbol.iterator]();
    const doAction = () => {
      const next = _iter.next();
      if (next.done) {
        // Close the repl. Note that it must have a clean prompt to do so.
        this.emit('keypress', '', { ctrl: true, name: 'd' });
        return;
      }
      const action = next.value;

      if (typeof action === 'object') {
        this.emit('keypress', '', action);
      } else {
        this.emit('data', `${action}`);
      }
      setImmediate(doAction);
    };
    doAction();
  }
  resume() {}
  pause() {}
}
ActionStream.prototype.readable = true;

// Mock keys
const ENTER = { name: 'enter' };
const UP = { name: 'up' };
const DOWN = { name: 'down' };
const LEFT = { name: 'left' };
const RIGHT = { name: 'right' };
const DELETE = { name: 'delete' };
const BACKSPACE = { name: 'backspace' };
const WORD_LEFT = { name: 'left', ctrl: true };
const WORD_RIGHT = { name: 'right', ctrl: true };
const GO_TO_END = { name: 'end' };
const DELETE_WORD_LEFT = { name: 'backspace', ctrl: true };
const SIGINT = { name: 'c', ctrl: true };
const ESCAPE = { name: 'escape', meta: true };

const prompt = '> ';
const WAIT = '€';

const prev = process.features.inspector;

let completions = 0;

const tests = [
  { // Creates few history to navigate for
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    test: [ 'let ab = 45', ENTER,
            '555 + 909', ENTER,
            '{key : {key2 :[] }}', ENTER,
            'Array(100).fill(1).map((e, i) => i ** i)', LEFT, LEFT, DELETE,
            '2', ENTER],
    expected: [],
    clean: false
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    test: [UP, UP, UP, UP, UP, DOWN, DOWN, DOWN, DOWN, DOWN],
    expected: [prompt,
               `${prompt}Array(100).fill(1).map((e, i) => i ** 2)`,
               prev && '\n// [ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, ' +
                 '144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529,' +
                 ' 576, 625, 676, 729, 784, 841, 900, 961, 1024, 1089, 1156, ' +
                 '1225, 1296, 1369, 1444, 1521, 1600, 1681, 1764, 1849, 1936,' +
                 ' 2025, 2116, 2209,...',
               `${prompt}{key : {key2 :[] }}`,
               prev && '\n// { key: { key2: [] } }',
               `${prompt}555 + 909`,
               prev && '\n// 1464',
               `${prompt}let ab = 45`,
               prompt,
               `${prompt}let ab = 45`,
               `${prompt}555 + 909`,
               prev && '\n// 1464',
               `${prompt}{key : {key2 :[] }}`,
               prev && '\n// { key: { key2: [] } }',
               `${prompt}Array(100).fill(1).map((e, i) => i ** 2)`,
               prev && '\n// [ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, ' +
                 '144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529,' +
                 ' 576, 625, 676, 729, 784, 841, 900, 961, 1024, 1089, 1156, ' +
                 '1225, 1296, 1369, 1444, 1521, 1600, 1681, 1764, 1849, 1936,' +
                 ' 2025, 2116, 2209,...',
               prompt].filter((e) => typeof e === 'string'),
    clean: false
  },
  { // Creates more history entries to navigate through.
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    test: [
      '555 + 909', ENTER, // Add a duplicate to the history set.
      'const foo = true', ENTER,
      '555n + 111n', ENTER,
      '5 + 5', ENTER,
      '55 - 13 === 42', ENTER
    ],
    expected: [],
    clean: false
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    checkTotal: true,
    preview: false,
    showEscapeCodes: true,
    test: [
      '55', UP, UP, UP, UP, UP, UP, ENTER
    ],
    expected: [
      '\x1B[1G', '\x1B[0J', prompt, '\x1B[3G',
      // '55'
      '5', '5',
      // UP
      '\x1B[1G', '\x1B[0J',
      '> 55 - 13 === 42', '\x1B[17G',
      // UP - skipping 5 + 5
      '\x1B[1G', '\x1B[0J',
      '> 555n + 111n', '\x1B[14G',
      // UP - skipping const foo = true
      '\x1B[1G', '\x1B[0J',
      '> 555 + 909', '\x1B[12G',
      // UP, UP
      // UPs at the end of the history reset the line to the original input.
      '\x1B[1G', '\x1B[0J',
      '> 55', '\x1B[5G',
      // ENTER
      '\r\n', '55\n',
      '\x1B[1G', '\x1B[0J',
      '> ', '\x1B[3G',
      '\r\n'
    ],
    clean: true
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    skip: !process.features.inspector,
    test: [
      // あ is a full width character with a length of one.
      // 🐕 is a full width character with a length of two.
      // 𐐷 is a half width character with the length of two.
      // '\u0301', '0x200D', '\u200E' are zero width characters.
      `const x1 = '${'あ'.repeat(124)}'`, ENTER, // Fully visible
      ENTER,
      `const y1 = '${'あ'.repeat(125)}'`, ENTER, // Cut off
      ENTER,
      `const x2 = '${'🐕'.repeat(124)}'`, ENTER, // Fully visible
      ENTER,
      `const y2 = '${'🐕'.repeat(125)}'`, ENTER, // Cut off
      ENTER,
      `const x3 = '${'𐐷'.repeat(248)}'`, ENTER, // Fully visible
      ENTER,
      `const y3 = '${'𐐷'.repeat(249)}'`, ENTER, // Cut off
      ENTER,
      `const x4 = 'a${'\u0301'.repeat(1000)}'`, ENTER, // á
      ENTER,
      `const ${'veryLongName'.repeat(30)} = 'I should be previewed'`,
      ENTER,
      'const e = new RangeError("visible\\ninvisible")',
      ENTER,
      'e',
      ENTER,
      'veryLongName'.repeat(30),
      ENTER,
      `${'\x1B[90m \x1B[39m'.repeat(235)} fun`,
      ESCAPE,
      ENTER,
      `${' '.repeat(236)} fun`,
      ESCAPE,
      ENTER
    ],
    expected: [],
    clean: false
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    columns: 250,
    checkTotal: true,
    showEscapeCodes: true,
    skip: !process.features.inspector,
    test: [
      UP,
      UP,
      UP,
      WORD_LEFT,
      UP,
      BACKSPACE,
      'x1',
      BACKSPACE,
      '2',
      BACKSPACE,
      '3',
      BACKSPACE,
      '4',
      DELETE_WORD_LEFT,
      'y1',
      BACKSPACE,
      '2',
      BACKSPACE,
      '3',
      SIGINT
    ],
    // A = Cursor n up
    // B = Cursor n down
    // C = Cursor n forward
    // D = Cursor n back
    // G = Cursor to column n
    // J = Erase in screen; 0 = right; 1 = left; 2 = total
    // K = Erase in line; 0 = right; 1 = left; 2 = total
    expected: [
      // 0. Start
      '\x1B[1G', '\x1B[0J',
      prompt, '\x1B[3G',
      // 1. UP
      // This exceeds the maximum columns (250):
      // Whitespace + prompt + ' // '.length + 'function'.length
      // 236 + 2 + 4 + 8
      '\x1B[1G', '\x1B[0J',
      `${prompt}${' '.repeat(236)} fun`, '\x1B[243G',
      ' // ction', '\x1B[243G',
      ' // ction', '\x1B[243G',
      '\x1B[0K',
      // 2. UP
      '\x1B[1G', '\x1B[0J',
      `${prompt}${' '.repeat(235)} fun`, '\x1B[242G',
      // TODO(BridgeAR): Investigate why the preview is generated twice.
      ' // ction', '\x1B[242G',
      ' // ction', '\x1B[242G',
      // Preview cleanup
      '\x1B[0K',
      // 3. UP
      '\x1B[1G', '\x1B[0J',
      // 'veryLongName'.repeat(30).length === 360
      // prompt.length === 2
      // 360 % 250 + 2 === 112 (+1)
      `${prompt}${'veryLongName'.repeat(30)}`, '\x1B[113G',
      // "// 'I should be previewed'".length + 86 === 112 (+1)
      "\n// 'I should be previewed'", '\x1B[113G', '\x1B[1A',
      // Preview cleanup
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      // 4. WORD LEFT
      // Almost identical as above. Just one extra line.
      // Math.floor(360 / 250) === 1
      '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      `${prompt}${'veryLongName'.repeat(30)}`, '\x1B[3G', '\x1B[1A',
      '\x1B[1B', "\n// 'I should be previewed'", '\x1B[3G', '\x1B[2A',
      // Preview cleanup
      '\x1B[2B', '\x1B[2K', '\x1B[2A',
      // 5. UP
      '\x1B[1G', '\x1B[0J',
      `${prompt}e`, '\x1B[4G',
      // '// RangeError: visible'.length - 19 === 3 (+1)
      '\n// RangeError: visible', '\x1B[4G', '\x1B[1A',
      // Preview cleanup
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      // 6. Backspace
      '\x1B[1G', '\x1B[0J',
      '> ', '\x1B[3G', 'x', '1',
      `\n// '${'あ'.repeat(124)}'`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> x', '\x1B[4G', '2',
      `\n// '${'🐕'.repeat(124)}'`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> x', '\x1B[4G', '3',
      `\n// '${'𐐷'.repeat(248)}'`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> x', '\x1B[4G', '4',
      `\n// 'a${'\u0301'.repeat(1000)}'`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> ', '\x1B[3G', 'y', '1',
      `\n// '${'あ'.repeat(121)}...`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> y', '\x1B[4G', '2',
      `\n// '${'🐕'.repeat(121)}...`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\x1B[1G', '\x1B[0J',
      '> y', '\x1B[4G', '3',
      `\n// '${'𐐷'.repeat(242)}...`,
      '\x1B[5G', '\x1B[1A',
      '\x1B[1B', '\x1B[2K', '\x1B[1A',
      '\r\n',
      '\x1B[1G', '\x1B[0J',
      '> ', '\x1B[3G',
      '\r\n'
    ],
    clean: true
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    showEscapeCodes: true,
    skip: !process.features.inspector,
    checkTotal: true,
    test: [
      'fu',
      'n',
      RIGHT,
      BACKSPACE,
      LEFT,
      LEFT,
      'A',
      BACKSPACE,
      GO_TO_END,
      BACKSPACE,
      WORD_LEFT,
      WORD_RIGHT,
      ESCAPE,
      ENTER,
      UP,
      LEFT,
      ENTER,
      UP,
      ENTER
    ],
    // C = Cursor n forward
    // D = Cursor n back
    // G = Cursor to column n
    // J = Erase in screen; 0 = right; 1 = left; 2 = total
    // K = Erase in line; 0 = right; 1 = left; 2 = total
    expected: [
      // 0.
      // 'f'
      '\x1B[1G', '\x1B[0J', prompt, '\x1B[3G', 'f',
      // 'u'
      'u', ' // nction', '\x1B[5G',
      // 'n' - Cleanup
      '\x1B[0K',
      'n', ' // ction', '\x1B[6G',
      // 1. Right. Cleanup
      '\x1B[0K',
      'ction',
      // 2. Backspace. Refresh
      '\x1B[1G', '\x1B[0J', `${prompt}functio`, '\x1B[10G',
      // Autocomplete and refresh?
      ' // n', '\x1B[10G', ' // n', '\x1B[10G',
      // 3. Left. Cleanup
      '\x1B[0K',
      '\x1B[1D', '\x1B[10G', ' // n', '\x1B[9G',
      // 4. Left. Cleanup
      '\x1B[10G', '\x1B[0K', '\x1B[9G',
      '\x1B[1D', '\x1B[10G', ' // n', '\x1B[8G',
      // 5. 'A' - Cleanup
      '\x1B[10G', '\x1B[0K', '\x1B[8G',
      // Refresh
      '\x1B[1G', '\x1B[0J', `${prompt}functAio`, '\x1B[9G',
      // 6. Backspace. Refresh
      '\x1B[1G', '\x1B[0J', `${prompt}functio`, '\x1B[8G', '\x1B[10G', ' // n',
      '\x1B[8G', '\x1B[10G', ' // n',
      '\x1B[8G', '\x1B[10G',
      // 7. Go to end. Cleanup
      '\x1B[0K', '\x1B[8G', '\x1B[2C',
      'n',
      // 8. Backspace. Refresh
      '\x1B[1G', '\x1B[0J', `${prompt}functio`, '\x1B[10G',
      // Autocomplete
      ' // n', '\x1B[10G', ' // n', '\x1B[10G',
      // 9. Word left. Cleanup
      '\x1B[0K', '\x1B[7D', '\x1B[10G', ' // n', '\x1B[3G', '\x1B[10G',
      // 10. Word right. Cleanup
      '\x1B[0K', '\x1B[3G', '\x1B[7C', ' // n', '\x1B[10G',
      // 11. ESCAPE
      '\x1B[0K', ' // n', '\x1B[10G', '\x1B[0K',
      // 12. ENTER
      '\r\n',
      'Uncaught ReferenceError: functio is not defined\n',
      '\x1B[1G', '\x1B[0J',
      // 13. UP
      prompt, '\x1B[3G', '\x1B[1G', '\x1B[0J',
      `${prompt}functio`, '\x1B[10G',
      ' // n', '\x1B[10G',
      ' // n', '\x1B[10G',
      // 14. LEFT
      '\x1B[0K', '\x1B[1D',
      '\x1B[10G', ' // n', '\x1B[9G', '\x1B[10G',
      // 15. ENTER
      '\x1B[0K', '\x1B[9G', '\x1B[1C',
      '\r\n',
      'Uncaught ReferenceError: functio is not defined\n',
      '\x1B[1G', '\x1B[0J',
      '> ', '\x1B[3G',
      // 16. UP
      '\x1B[1G', '\x1B[0J',
      '> functio', '\x1B[10G',
      ' // n', '\x1B[10G',
      ' // n', '\x1B[10G', '\x1B[0K',
      // 17. ENTER
      'n', '\r\n',
      '\x1B[1G', '\x1B[0J',
      '... ', '\x1B[5G',
      '\r\n'
    ],
    clean: true
  },
  {
    // Check changed inspection defaults.
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    skip: !process.features.inspector,
    test: [
      'util.inspect.replDefaults.showHidden',
      ENTER
    ],
    expected: [],
    clean: false
  },
  {
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    skip: !process.features.inspector,
    checkTotal: true,
    test: [
      '[ ]',
      WORD_LEFT,
      WORD_LEFT,
      UP,
      ' = true',
      ENTER,
      '[ ]',
      ENTER
    ],
    expected: [
      prompt,
      '[', ' ', ']',
      '\n// []', '\n// []', '\n// []',
      '> util.inspect.replDefaults.showHidden',
      '\n// false',
      ' ', '=', ' ', 't', 'r', 'u', 'e',
      'true\n',
      '> ', '[', ' ', ']',
      '\n// [ [length]: 0 ]',
      '[ [length]: 0 ]\n',
      '> ',
    ],
    clean: true
  },
  {
    // Check that the completer ignores completions that are outdated.
    env: { NODE_REPL_HISTORY: defaultHistoryPath },
    completer(line, callback) {
      if (line.endsWith(WAIT)) {
        if (completions++ === 0) {
          callback(null, [[`${WAIT}WOW`], line]);
        } else {
          setTimeout(callback, 1000, null, [[`${WAIT}WOW`], line]).unref();
        }
      } else {
        callback(null, [[' Always visible'], line]);
      }
    },
    skip: !process.features.inspector,
    test: [
      WAIT, // The first call is awaited before new input is triggered!
      BACKSPACE,
      's',
      BACKSPACE,
      WAIT, // The second call is not awaited. It won't trigger the preview.
      BACKSPACE,
      's',
      BACKSPACE
    ],
    expected: [
      prompt,
      WAIT,
      ' // WOW',
      prompt,
      's',
      ' // Always visible',
      prompt,
      WAIT,
      prompt,
      's',
      ' // Always visible',
      prompt,
    ],
    clean: true
  }
];
const numtests = tests.length;

const runTestWrap = common.mustCall(runTest, numtests);

function cleanupTmpFile() {
  try {
    // Write over the file, clearing any history
    fs.writeFileSync(defaultHistoryPath, '');
  } catch (err) {
    if (err.code === 'ENOENT') return true;
    throw err;
  }
  return true;
}

function runTest() {
  const opts = tests.shift();
  if (!opts) return; // All done

  const { expected, skip } = opts;

  // Test unsupported on platform.
  if (skip) {
    setImmediate(runTestWrap, true);
    return;
  }
  const lastChunks = [];
  let i = 0;

  REPL.createInternalRepl(opts.env, {
    input: new ActionStream(),
    output: new stream.Writable({
      write(chunk, _, next) {
        const output = chunk.toString();

        if (!opts.showEscapeCodes &&
            (output[0] === '\x1B' || /^[\r\n]+$/.test(output))) {
          return next();
        }

        lastChunks.push(output);

        if (expected.length && !opts.checkTotal) {
          try {
            assert.strictEqual(output, expected[i]);
          } catch (e) {
            console.error(`Failed test # ${numtests - tests.length}`);
            console.error('Last outputs: ' + inspect(lastChunks, {
              breakLength: 5, colors: true
            }));
            throw e;
          }
          // TODO(BridgeAR): Auto close on last chunk!
          i++;
        }

        next();
      }
    }),
    completer: opts.completer,
    prompt,
    useColors: false,
    preview: opts.preview,
    terminal: true
  }, function(err, repl) {
    if (err) {
      console.error(`Failed test # ${numtests - tests.length}`);
      throw err;
    }

    repl.once('close', () => {
      if (opts.clean)
        cleanupTmpFile();

      if (opts.checkTotal) {
        assert.deepStrictEqual(lastChunks, expected);
      } else if (expected.length !== i) {
        console.error(tests[numtests - tests.length - 1]);
        throw new Error(`Failed test # ${numtests - tests.length}`);
      }

      setImmediate(runTestWrap, true);
    });

    if (opts.columns) {
      Object.defineProperty(repl, 'columns', {
        value: opts.columns,
        enumerable: true
      });
    }
    repl.input.run(opts.test);
  });
}

// run the tests
runTest();
