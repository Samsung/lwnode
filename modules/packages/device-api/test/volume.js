/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

require('./gmain-loop').init();
require('./device-api').init();

function onVolumeChanged(volume) {
  console.log('onVolumeChanged', volume);
}

console.log("start");
tizen.tvaudiocontrol.setVolumeChangeListener(onVolumeChanged);

setTimeout(()=> {
  console.log("setVolumeUp");
  tizen.tvaudiocontrol.setVolumeUp();
}, 500);

setTimeout(()=> {
  tizen.tvaudiocontrol.setVolume(0);
  console.log("end");
}, 5000);
