# device-api

Support tizen device api.

* This module require `gmain-loop` module.

## API

### init()
 * Bind the device api to global. After initialization, you can use the tizen api.

 ```js
 require('gmain-loop').init(); // This module is required.
 require('device-api').init();

 let adapter = tizen.bluetooth.getDefaultAdapter();
 ```

## Requirements

### tvinputdevice
 * When using tvinputdevice api in tpk package, you should add the following privilege to `tizen-manifest.xml`.
   * `http://tizen.org/privilege/tv.inputdevice`
   * `http://developer.samsung.com/privilege/inputdevice`
