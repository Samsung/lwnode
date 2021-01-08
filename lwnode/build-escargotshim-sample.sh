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

configure() {
  local build_type=$1
  local gyp_file=$2

  tools/gyp/gyp \
    -f ninja --depth=. \
    -Dbuild_mode=$build_type \
    $gyp_file
}

build() {
  local build_out_dir=$1
  local build_target=$2

  ninja -v -C $build_out_dir $build_target
}

BUILD_MODE=debug

opt=$(getopt \
      -o cbr \
      -l config,build,release,clean \
      -- "$@")

eval set -- "$opt"

while [[ $# -gt 0 ]]; do
  case "$1" in
    -c | --config)
      configure $BUILD_MODE lwnode/codes/escargotshim/sample/sample.gyp
      ;;
    -b | --build)
      BUILD_MODE_FIRST_LETTER_IN_UPPERCASE="$(tr '[:lower:]' '[:upper:]' <<< ${BUILD_MODE:0:1})${BUILD_MODE:1}"
      build out/$BUILD_MODE_FIRST_LETTER_IN_UPPERCASE escargotshim_sample |& lwnode/tools2/colorize.sh
      ;;
    -r | --release)
      BUILD_MODE=release
      ;;
    --clean)
      rm -rf out
      ;;
  esac
  shift
done
