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
PACKAGES_ROOT_PATH=$PROJECT_ROOT_PATH/modules/packages
BUILD_OUT_ROOT_PATH=$PROJECT_ROOT_PATH/out/modules
LWNODE_INCLUDES_PATH="$PROJECT_ROOT_PATH/deps/node/src;$PROJECT_ROOT_PATH/include"

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
    if [ -d "$PACKAGES_ROOT_PATH/$module" ]; then
      build_module $module
    else
      error_echo "Cannot find module: $module"
    fi
  done
}

check_build_result() {
  ret=$?
  if [ $ret -ne 0 ]; then
    error_echo "Failed to build $1"
    exit $ret
  fi
}

build_module() {
  fancy_echo "build [$1]"

  local out_path=$BUILD_OUT_ROOT_PATH/$TARGET_OS/$1
  local module_path=$PACKAGES_ROOT_PATH/$1
  local definitions="-DBUILDING_NODE_EXTENSION;-DLWNODE"
  mkdir -p $out_path

  cmake $module_path -B$out_path -H$module_path \
    -DPROJECT_ROOT_PATH=$PROJECT_ROOT_PATH      \
    -DLWNODE_INCLUDES=$LWNODE_INCLUDES_PATH     \
    -DLWNODE_DEFINITIONS=$definitions           \
    -G Ninja
  check_build_result $1
  ninja -C $out_path
  check_build_result $1
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
