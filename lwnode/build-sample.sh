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

ROOT_PATH=./out/sample

gyp ./lwnode/code/escargotshim/sample/sample.gyp --depth=. -f ninja \
  --generator-output=$ROOT_PATH \
  -Descargot_lib_type=static_lib -Dtarget_arch=x64 -Dtarget_os=linux \
  -Dbuild_mode=debug \
  -Descargot_threading=1

ninja -v -C $ROOT_PATH/out/Debug sample
