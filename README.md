# Lightweight Node.js

Lightweight Node.js is a memory efficient Node.js implementation,
which runs on top of [Escargot](https://github.com/Samsung/escargot),
a memory optimized JavaScript Engine developed by Samsung Research,
instead of the default V8 JS engine.


### How to build
```
$ ./configure.py
$ ninja -C out/linux/Release lwnode
```
