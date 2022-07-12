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
