'use strict';

// @lwnode
process._rawDebug('js:run_main_module');

const {
  prepareMainThreadExecution
} = require('internal/bootstrap/pre_execution');

process._rawDebug('js:prepareMainThreadExecution');

prepareMainThreadExecution(true);

process._rawDebug('js:markBootstrapComplete');

markBootstrapComplete();

process._rawDebug('+js:runMain');

// Note: this loads the module through the ESM loader if the module is
// determined to be an ES module. This hangs from the CJS module loader
// because we currently allow monkey-patching of the module loaders
// in the preloaded scripts through require('module').
// runMain here might be monkey-patched by users in --require.
// XXX: the monkey-patchability here should probably be deprecated.
require('internal/modules/cjs/loader').Module.runMain(process.argv[1]);

process._rawDebug('-js:runMain');

let list = process.lwnode.getModuleList();
process._rawDebug(list, list.length);
