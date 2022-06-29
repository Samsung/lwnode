# Lightweight Node.js

Lightweight Node.js is a memory efficient Node.js implementation,
which runs on top of [Escargot](https://github.com/Samsung/escargot),
a memory optimized JavaScript Engine developed by Samsung Research,
instead of the default V8 JS engine.

The following provides useful information about lwnode:
* [Build Guide](docs/Build.md)
* Lightweight node.js (lwnode)
  - [Supported Features](docs/Spec.md)
  - [Debugging Guide](docs/Debugger.md)
  - [Memory Usage](https://pages.github.sec.samsung.net/lws/lwnode-test-results): Memory usage and binary footprint are reported.
* [JS Backend Service Framework](docs/Framework.md)
  - [Resource Management](docs/api/lwnode.md): This page introduces about our resource management with a simple example.
  - [Service App Guide](docs/App-service.md)
    - [App Template](lwnode/apps/template/)
  - [DB Service and Client App Guide](docs/App-db-service.md)
* Others
  - [MAINTAINERS.md](MAINTAINERS.md).
  - [GOVERNANCE.md](GOVERNANCE.md)
