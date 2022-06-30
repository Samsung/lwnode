# Service App Template
This directory contains a service app template that can help you to create a service app based on `lwnode` for Tizen devices such as Tizen TV.

## 1. Template Structure
At minimum, a service app consists of the following files.

```sh
// Typical directory structure
root
  bin
    lwnode
  lib
    index.js
  packaging
    helloworld.service
    helloworld.spec
  package.json
```

* lwnode: A `lwnode` executable to use to run a service app.
* [index.js](lib/index.js): It defines the main entry point to a service app.
* [helloworld.service](packaging/helloworld.service): It makes the app to register as a service on a Tizen TV.
* [helloworld.spec](packaging/helloworld.spec): It defines GBS build rules.
* [helloworld.manifest](packaging/helloworld.manifest): Minimum `.manifest` to create a Tizen app.
