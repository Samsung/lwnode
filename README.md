# Lightweight Node.js

Lightweight Node.js is a memory efficient Node.js implementation,
which runs on top of [Escargot](https://github.com/Samsung/escargot),
a memory optimized JavaScript Engine developed by Samsung Research,
instead of the default V8 JS engine.

The following provides useful information about lwnode:
* [Build Guide](docs/BUILD.md)
* Lightweight node.js (lwnode)
  - [Supported Features](docs/spec.md)
  - [Debugging Guide](docs/DEBUG.md)
  - [Memory Usage](https://pages.github.sec.samsung.net/lws/lwnode-test-results): Memory usage and binary footprint are reported.
* [JS Backend Service Framework](docs/Framework.md)
  - [Resource Management](docs/api/lwnode.md): This page introduces about our resource management with a simple example.
  - [Express App Guide](docs/App-express.md)
    - [App Template](lwnode/apps/template/)
  - [Sqlite3 App Guide](docs/App-sqlite3.md)
* Others
  - [MAINTAINERS.md](MAINTAINERS.md).
  - [GOVERNANCE.md](GOVERNANCE.md)
