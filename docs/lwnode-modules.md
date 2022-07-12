# LWNode modules

## Install modules

LWNode supports special modules. In order to use these modules, need to install a `lwnode-modules-x.x.x-x.armv7l.rpm` on your device.

```console
rpm -Uvh --nodeps --force lwnode-modules-1.0.0-1.armv7l.rpm
```

You can load modules in your app.
For example,
```js
const gmainLoop = require('gmain-loop');
```

## Supported modules
* [gmain-loop](modules/packages/gmain-loop/README.md): change node event loop to gmain loop
* [device-api](modules/packages/device-api/README.md): support tizen device-api
