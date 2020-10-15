'use strict';

// This list must be computed before we require any modules to
// to eliminate the noise.
const actualModules = new Set(process.moduleLoadList.slice());

const common = require('../common');
const assert = require('assert');

const expectedModules = new Set([
  'Internal Binding errors',
  'Internal Binding async_wrap',
  'Internal Binding buffer',
  'Internal Binding config',
  'Internal Binding constants',
  'Internal Binding contextify',
  'Internal Binding credentials',
  'Internal Binding fs',
  'Internal Binding fs_dir',
  'Internal Binding messaging',
  'Internal Binding module_wrap',
  'Internal Binding native_module',
  'Internal Binding options',
  'Internal Binding process_methods',
  'Internal Binding report',
  'Internal Binding string_decoder',
  'Internal Binding symbols',
  'Internal Binding task_queue',
  'Internal Binding timers',
  'Internal Binding trace_events',
  'Internal Binding types',
  'Internal Binding url',
  'Internal Binding util',
  'NativeModule buffer',
  'NativeModule events',
  'NativeModule fs',
  'NativeModule internal/assert',
  'NativeModule internal/async_hooks',
  'NativeModule internal/bootstrap/pre_execution',
  'NativeModule internal/buffer',
  'NativeModule internal/console/constructor',
  'NativeModule internal/console/global',
  'NativeModule internal/constants',
  'NativeModule internal/encoding',
  'NativeModule internal/errors',
  'NativeModule internal/fixed_queue',
  'NativeModule internal/fs/dir',
  'NativeModule internal/fs/utils',
  'NativeModule internal/idna',
  'NativeModule internal/linkedlist',
  'NativeModule internal/modules/run_main',
  'NativeModule internal/modules/package_json_reader',
  'NativeModule internal/modules/cjs/helpers',
  'NativeModule internal/modules/cjs/loader',
  'NativeModule internal/modules/esm/create_dynamic_module',
  'NativeModule internal/modules/esm/get_format',
  'NativeModule internal/modules/esm/get_source',
  'NativeModule internal/modules/esm/loader',
  'NativeModule internal/modules/esm/module_job',
  'NativeModule internal/modules/esm/module_map',
  'NativeModule internal/modules/esm/resolve',
  'NativeModule internal/modules/esm/transform_source',
  'NativeModule internal/modules/esm/translators',
  'NativeModule internal/process/esm_loader',
  'NativeModule internal/options',
  'NativeModule internal/priority_queue',
  'NativeModule internal/process/execution',
  'NativeModule internal/process/per_thread',
  'NativeModule internal/process/promises',
  'NativeModule internal/process/report',
  'NativeModule internal/process/signal',
  'NativeModule internal/process/task_queues',
  'NativeModule internal/process/warning',
  'NativeModule internal/querystring',
  'NativeModule internal/source_map/source_map_cache',
  'NativeModule internal/timers',
  'NativeModule internal/url',
  'NativeModule internal/util',
  'NativeModule internal/util/debuglog',
  'NativeModule internal/util/inspect',
  'NativeModule internal/util/types',
  'NativeModule internal/validators',
  'NativeModule internal/vm/module',
  'NativeModule internal/worker/js_transferable',
  'NativeModule path',
  'NativeModule timers',
  'NativeModule url',
  'NativeModule vm',
]);

if (!common.isMainThread) {
  [
    'Internal Binding messaging',
    'Internal Binding symbols',
    'Internal Binding worker',
    'NativeModule _stream_duplex',
    'NativeModule _stream_passthrough',
    'NativeModule _stream_readable',
    'NativeModule _stream_transform',
    'NativeModule _stream_writable',
    'NativeModule internal/error_serdes',
    'NativeModule internal/event_target',
    'NativeModule internal/process/worker_thread_only',
    'NativeModule internal/streams/buffer_list',
    'NativeModule internal/streams/destroy',
    'NativeModule internal/streams/end-of-stream',
    'NativeModule internal/streams/legacy',
    'NativeModule internal/streams/pipeline',
    'NativeModule internal/streams/state',
    'NativeModule internal/worker',
    'NativeModule internal/worker/io',
    'NativeModule stream',
    'NativeModule util',
    'NativeModule worker_threads',
  ].forEach(expectedModules.add.bind(expectedModules));
}

if (common.hasIntl) {
  expectedModules.add('Internal Binding icu');
} else {
  expectedModules.add('NativeModule punycode');
}

if (process.features.inspector) {
  expectedModules.add('Internal Binding inspector');
  expectedModules.add('NativeModule internal/inspector_async_hook');
  expectedModules.add('NativeModule internal/util/inspector');
}

if (process.env.NODE_V8_COVERAGE) {
  expectedModules.add('Internal Binding profiler');
}

const difference = (setA, setB) => {
  return new Set([...setA].filter((x) => !setB.has(x)));
};
const missingModules = difference(expectedModules, actualModules);
const extraModules = difference(actualModules, expectedModules);
const printSet = (s) => { return `${[...s].sort().join(',\n  ')}\n`; };

assert.deepStrictEqual(actualModules, expectedModules,
                       (missingModules.size > 0 ?
                         'These modules were not loaded:\n  ' +
                         printSet(missingModules) : '') +
                       (extraModules.size > 0 ?
                         'These modules were unexpectedly loaded:\n  ' +
                         printSet(extraModules) : ''));
