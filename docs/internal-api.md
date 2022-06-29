# Internal documentation of LWNode.js

These flags and variables are for LWNode.js core development usage only. Do not use them in your own applications.

## CLI

### Flags

#### `--internal-log`

If the `--internal-log` flag is specified or the `LWNODE_INTERNAL_LOG` environment variable is set to 1, LWNode will print internal logs in release build.

#### `--trace-call[=[-]trace id[,<trace id>]]`

If the `--trace-call` flag is specified or the `LWNODE_TRACE_CALL` environment variable is set to trace ids, LWNode will print trace call logs in debug build. Setting trace id with `-` will negate printing trace logs for the specific id.

```shell
--trace-call=COMMON,ISOLATE

# or

export LWNODE_TRACE_CALL=COMMON,ISOLATE
```

### Environment variables

#### `LWNODE_INTERNAL_LOG`

Refer to the description above.

#### `LWNODE_TRACE_CALL`

Refer to the description above.

#### `LWNODE_RUNNING_ON_TESTS`

If the `LWNODE_RUNNING_ON_TESTS` environment variable is set to 1, LWNode will ignore comparing error messages in detail while using `assert.throw` and similars. This is used as default when using `tools/test.py`. Please refer to https://github.sec.samsung.net/lws/node-escargot/issues/1002 for more information.
