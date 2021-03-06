# Lightweight Node.js Specification

## Node.js
  * Forked version: 14.14
## Supported Features
  * [Assertion](https://nodejs.org/dist/latest-v14.x/docs/api/assert.html)
  * [Async hooks](https://nodejs.org/dist/latest-v14.x/docs/api/async_hooks.html)
  * [Buffer](https://nodejs.org/dist/latest-v14.x/docs/api/buffer.html)
  * [C/C++ addons with Node-API](https://nodejs.org/dist/latest-v14.x/docs/api/n-api.html)
  * [Child processes](https://nodejs.org/dist/latest-v14.x/docs/api/child_process.html)
  * [Cluster](https://nodejs.org/dist/latest-v14.x/docs/api/cluster.html)
  * [Command-line options](https://nodejs.org/dist/latest-v14.x/docs/api/cli.html)
  * [Console](https://nodejs.org/dist/latest-v14.x/docs/api/console.html)
  * [Crypto](https://nodejs.org/dist/latest-v14.x/docs/api/crypto.html)
  * [DNS](https://nodejs.org/dist/latest-v14.x/docs/api/dns.html)
  * [Domain](https://nodejs.org/dist/latest-v14.x/docs/api/domain.html)
  * [Errors](https://nodejs.org/dist/latest-v14.x/docs/api/errors.html)
  * [Events](https://nodejs.org/dist/latest-v14.x/docs/api/events.html)
  * [File system](https://nodejs.org/dist/latest-v14.x/docs/api/fs.html)
  * [Globals](https://nodejs.org/dist/latest-v14.x/docs/api/globals.html)
  * [HTTP](https://nodejs.org/dist/latest-v14.x/docs/api/http.html)
  * [HTTP/2](https://nodejs.org/dist/latest-v14.x/docs/api/http2.html)
  * [HTTPS](https://nodejs.org/dist/latest-v14.x/docs/api/https.html)
  * [Modules: CommonJS modules](https://nodejs.org/dist/latest-v14.x/docs/api/modules.html)
  * [Net](https://nodejs.org/dist/latest-v14.x/docs/api/net.html)
  * [OS](https://nodejs.org/dist/latest-v14.x/docs/api/os.html)
  * [Path](https://nodejs.org/dist/latest-v14.x/docs/api/path.html)
  * [Process](https://nodejs.org/dist/latest-v14.x/docs/api/process.html)
  * [Punycode](https://nodejs.org/dist/latest-v14.x/docs/api/punycode.html)
  * [Query strings](https://nodejs.org/dist/latest-v14.x/docs/api/querystring.html)
  * [Readline](https://nodejs.org/dist/latest-v14.x/docs/api/readline.html)
  * [Stream](https://nodejs.org/dist/latest-v14.x/docs/api/stream.html)
  * [String decoder](https://nodejs.org/dist/latest-v14.x/docs/api/string_decoder.html)
  * [Timers](https://nodejs.org/dist/latest-v14.x/docs/api/timers.html)
  * [TLS/SSL](https://nodejs.org/dist/latest-v14.x/docs/api/tls.html)
  * [TTY](https://nodejs.org/dist/latest-v14.x/docs/api/tty.html)
  * [UDP/datagram](https://nodejs.org/dist/latest-v14.x/docs/api/dgram.html)
  * [URL](https://nodejs.org/dist/latest-v14.x/docs/api/url.html)
  * [Utilities](https://nodejs.org/dist/latest-v14.x/docs/api/util.html)
  * [Zlib](https://nodejs.org/dist/latest-v14.x/docs/api/zlib.html)

## Experimental (with optional flags)
  * [Deprecated APIs](https://nodejs.org/dist/latest-v14.x/docs/api/deprecations.html)
  * [Worker threads](https://nodejs.org/dist/latest-v14.x/docs/api/worker_threads.html)
  * [Modules: ECMAScript modules](https://nodejs.org/dist/latest-v14.x/docs/api/esm.html)
  * [Modules: `module` API](https://nodejs.org/dist/latest-v14.x/docs/api/module.html)
  * [Modules: Packages](https://nodejs.org/dist/latest-v14.x/docs/api/packages.html)
  * [Internationalization](https://nodejs.org/dist/latest-v14.x/docs/api/intl.html)

## Unsupported Features
  * [C++ addons](https://nodejs.org/dist/latest-v14.x/docs/api/addons.html)
  * [C++ embedder API](https://nodejs.org/dist/latest-v14.x/docs/api/embedding.html)
  * [Debugger](https://nodejs.org/dist/latest-v14.x/docs/api/debugger.html): Specific to V8. Debugging is supported via VS Code.
  * [Inspector](https://nodejs.org/dist/latest-v14.x/docs/api/inspector.html): Specific to V8
  * [Performance hooks](https://nodejs.org/dist/latest-v14.x/docs/api/perf_hooks.html): Specific to V8
  * [Policies](https://nodejs.org/dist/latest-v14.x/docs/api/policy.html)
  * [REPL](https://nodejs.org/dist/latest-v14.x/docs/api/repl.html)
  * [Report](https://nodejs.org/dist/latest-v14.x/docs/api/report.html): Specific to V8
  * [Trace events](https://nodejs.org/dist/latest-v14.x/docs/api/tracing.html): Specific to V8
  * [V8](https://nodejs.org/dist/latest-v14.x/docs/api/v8.html): Specific to V8
  * [VM](https://nodejs.org/dist/latest-v14.x/docs/api/vm.html)
  * [WASI](https://nodejs.org/dist/latest-v14.x/docs/api/wasi.html)

## Design Decisions and Known Issues
  * V8's implementation-specific internal APIs are not supported, e.g., Modules, etc.
  * Due to different GC models, V8's GC-related operations are not supported. Memory management is achieved by lwnode's automatic GC.
  * Supported user flags are: `--exposed-gc`, `--disallow-code-generation-from-strings`. User flags specific to V8's internal APIs are not supported, e.g., `--max_old_space_size`, etc.
  * `vm` and `repl`  are not supported for security reasons.
  * All literal strings are encoded in UTF16, when JS source has been encoded in UTF16.
  * If a JS source code file is encoded in UTF16, all literal strings in the file will also be encoded in UTF16 even if UTF8 is sufficient. This decision is to reduce memory usage by reusing the same string literals internally.

  * Known Issues:
    - In some cases, an async hook ID is set to null.
    - In some cases, an error message format is slightly different from node.js's error message, although it contains the same information.
    - In some cases, an event listener cannot receive an event thrown by a child process.
    - In some cases, a child process cannot obtain values from `process.env`.
    - `Worker` is experimental. It should be used with caution.
    - `ValueSerializer` is experimental. It should be used with caution.

## ECMAScript
  * [node.green](https://node.green/) provides an overview over supported ECMAScript features in our target version of Node.js, `v14.14`.
