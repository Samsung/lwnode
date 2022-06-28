/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// related: https://github.sec.samsung.net/lws/node-escargot/issues/889
// 이 한글 comment로 인해 escargot에서 이 소스는 utf16으로 encoding해 사용된다.
const assert = require('assert');

const latinString = 'tést';

const encBuffLatin = Buffer.from(latinString, 'latin1');
const decBuffLatin = encBuffLatin.toString('latin1');

console.log(encBuffLatin); // <Buffer 74 e9 73 74>
console.log(decBuffLatin);

for (let i = 0; i < latinString.length; ++i) {
  assert.ok(encBuffLatin[i] == latinString.charCodeAt(i));
}
