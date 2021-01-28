#!/bin/bash

# Copyright (c) 2020-present Samsung Electronics Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

CONFIG="--without-npm --without-bundled-v8 --without-v8-platform \
    --without-inspector --without-node-code-cache --without-node-snapshot \
    --with-intl none --shared-openssl --shared-zlib \
    --dest-os linux --dest-cpu x64 \
    --engine escargot \
    --ninja"

if [[ $1 =~ ^"-d" ]]; then
  ! [[ $1 =~ .*"b" ]] && ./configure $CONFIG --debug --debug-node
  ninja -v -C out/Debug node |& lwnode/tools/colorize.sh

else
  ./configure $CONFIG
  ninja -v -C out/Release node |& lwnode/tools/colorize.sh
fi
