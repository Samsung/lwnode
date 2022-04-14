#!/bin/bash
# Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

ROOT=$PWD
OUT_PATH=$ROOT/out/cctest
ARCH="x64"
ASAN=1

while [[ $# -gt 0 ]]; do
  case "$1" in
  --arch=*)
    ARCH="${1#*=}"
    ;;
  --asan=*)
    ASAN="${1#*=}"
    ;;
  *)
    echo "Unknown option $1"
    exit -1
    ;;
  esac

  shift
done

./tools/gyp/gyp ./lwnode/code/escargotshim/test/cctest.gyp --depth=. -f ninja \
  --generator-output=$OUT_PATH -Dasan=$ASAN -Dbuild_mode=debug \
  -Descargot_lib_type=static_lib -Dtarget_arch=$ARCH -Dtarget_os=linux \
  -Denable_experimental=true -Descargot_threading=1 -Dinclude_node_bindings=false \
  -Descargot_debugger=0

ninja -C $OUT_PATH/out/Debug cctest

if [ $ARCH == "x64" ]; then
ln -sf $OUT_PATH/out/Debug/cctest ./cctest
fi
