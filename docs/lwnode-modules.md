# LWNode modules

## Build modules
LWNode supports special modules for Tizen. To build them, use the command below.

```sh
$ gbs -c .github/gbs.conf build -A arm7l --packaging-dir modules/packages/packaging --include-all --incremental
```

You can find build result file in `<lwnode_project_path>/out/modules/tizen`.


## Install modules

In order to use these modules, need to install a `lwnode-modules-x.x.x-x.armv7l.rpm` on your device.

```console
rpm -Uvh --nodeps --force lwnode-modules-1.0.0-1.armv7l.rpm
```

You can load modules in your app.
For example,
```js
const gmainLoop = require('gmain-loop');
```

To include a module in your app without installing the module globally, copy the built `<module_name>.node` file from `<lwnode_project_path>/out/modules/tizen`.


## Supported modules
* [gmain-loop](modules/packages/gmain-loop/README.md): change node event loop to gmain loop
* [device-api](modules/packages/device-api/README.md): support tizen device-api
