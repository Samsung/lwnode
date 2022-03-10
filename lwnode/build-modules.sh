#!/bin/bash
# Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

TARGET_OS="linux"

PROJECT_ROOT_PATH=$PWD
MODULES_ROOT_PATH=$PROJECT_ROOT_PATH/lwnode/code/modules
BUILD_OUT_ROOT_PATH=$PROJECT_ROOT_PATH/out/modules
LWNODE_INCLUDES_PATH="$PROJECT_ROOT_PATH/src;$PROJECT_ROOT_PATH/lwnode/code/escargotshim/include"

fancy_echo() {
  local BOLD="\033[1m"
  local GREEN="\033[1;32m"
  local CLEAR="\033[0m"
  echo -e ""
  echo -e ${GREEN}$1$2${CLEAR}
}

error_echo() {
  local BOLD="\033[1m"
  local RED="\033[1;31m"
  local CLEAR="\033[0m"
  echo -e ""
  echo -e ${RED}$1$2${CLEAR}
}

usage() {
  echo "Usage: build-modules.sh <modules-list> [options]"
  echo "
   options:
    --os)       target os (linux/tizen)
    "
  echo "example) build-modules.sh hello-world,tizen-device-api --os=tizen
       "
}

find_and_build_modules() {
  local module_list=($(echo $1 | tr "," "\n"))

  for module in "${module_list[@]}"; do
    if [ -d "$MODULES_ROOT_PATH/$module" ]; then
      build_module $module
    else
      error_echo "Cannot find module: $module"
    fi
  done
}

build_module() {
  fancy_echo "build [$1]"

  local out_path=$BUILD_OUT_ROOT_PATH/$TARGET_OS/$1
  local module_path=$MODULES_ROOT_PATH/$1
  mkdir -p $out_path

  cmake $module_path -B$out_path -H$module_path -DLWNODE_INCLUDES=$LWNODE_INCLUDES_PATH \
    -G Ninja
  ninja -C $out_path
}

if [[ -z $1 ]] || [[ $1 == -* ]]; then
  usage
  exit 1
fi

MODULES_LIST=$1
shift

while [[ $# -gt 0 ]]; do
  case "$1" in
  --os=*)
    TARGET_OS="${1#*=}"
    ;;
  *)
    echo "Unknown option $1"
    exit -1
    ;;
  esac

  shift
done

fancy_echo "target os: $TARGET_OS"
find_and_build_modules $MODULES_LIST
