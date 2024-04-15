/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// https://developer.samsung.com/smarttv/develop/api-references/tizen-web-device-api-references/tvinputdevice-api.html

require('./device-api').init();

let i, keyCode = {};
let supportedKeys = tizen.tvinputdevice.getSupportedKeys();

for (i = 0; i < supportedKeys.length; i++) {
    keyCode[supportedKeys[i].name] = supportedKeys[i].code;
}

if (keyCode.hasOwnProperty("ChannelUp")) {
    console.log("register ChannelUp");
    tizen.tvinputdevice.registerKey("ChannelUp");
}

function errorCB(err) {
    console.log('The following error occurred: ' + err.name);
}

function successCB() {
    console.log('Registered successfully');
}

console.log("register VolumeUp, VolumeDown");
const keys = ["VolumeUp", "VolumeDown"];
tizen.tvinputdevice.registerKeyBatch(keys, successCB, errorCB);

process.on("keydown", function (keyEvent) {
    // identify the key by the numeric code from the keyEvent
    if (keyEvent.keyCode === keyCode.ChannelUp) {
        console.log("+ The CHANNEL UP was pressed");
    } else if (keyEvent.keyCode === keyCode.VolumeUp) {
        console.log("+  The VOLUME UP was pressed");
    } else if (keyEvent.keyCode === keyCode.VolumeDown) {
        console.log("+  The VOLUME DOWN was pressed");
    } else {
        console.log("fail test");
    }
});

process.on("keyup", function (keyEvent) {
    if (keyEvent.keyCode === keyCode.ChannelUp) {
        console.log("- The CHANNEL UP was released");
    } else if (keyEvent.keyCode === keyCode.VolumeUp) {
        console.log("- The VOLUME UP was released");
    } else if (keyEvent.keyCode === keyCode.VolumeDown) {
        console.log("- The VOLUME DOWN was released");
    } else {
        console.log("fail test");
    }
});

setTimeout(() => {
    console.log("unregister ChannelUp");

    tizen.tvinputdevice.unregisterKey("ChannelUp");
}, 10000);


setTimeout(() => {
    console.log("unregister VolumeUp, VolumeDown");

    function errorCB(err) {
        console.log('The following error occurred: ' + err.name);
    }
    
    function successCB() {
        console.log('Unregistered successfully');
    }
    tizen.tvinputdevice.unregisterKeyBatch(keys, successCB, errorCB);
}, 10000);


setTimeout(() => {
    console.log("test done!");
}, 50000);
