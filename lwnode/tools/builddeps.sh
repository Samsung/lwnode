#!/bin/bash

# Copyright (c) 2020-present Samsung Electronics Co., Ltd
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#  USA

set -e

MODULE_ROOT_RPATH=$1
# SCRIPT_PATH="$( cd "$(dirname "$0")" >/dev/null 2>&1; pwd )"
NOUSE_NPM=true; # command -v npm > /dev/null 2>&1 || NOUSE_NPM=true

function read_json {
  # [VAR]=read_json [filename] [key] || exit [code]
  local FNAME=$1
  local KEY=$2
  local UNAMESTR=`uname`
  local SED_EXTENDED=''

  if [[ -z $FNAME || -z $KEY ]]; then
    echo "Usage: [VAR]=${FUNCNAME[0]} <json filename> <key>"
    exit 1;
  fi

  if [[ "$UNAMESTR" == 'Linux' ]]; then
    SED_EXTENDED='-r'
  elif [[ "$UNAMESTR" == 'Darwin' ]]; then
    SED_EXTENDED='-E'
  fi

  local VALUE=`grep -m 1 "\"${KEY}\"" ${FNAME} | sed ${SED_EXTENDED} 's/^ *//;s/.*: *"//;s/",?//'`

  if [ ! "$VALUE" ]; then
    echo "Error: Cannot find \"${KEY}\" in ${FNAME}" >&2;
    exit 1;
  else
    echo $VALUE ;
  fi
}

find_and_build_modules() {
  local MODULE_ROOT_RPATH=$1

  for entry_path in "$MODULE_ROOT_RPATH"/*; do
    echo "build '$entry_path'"

    if [ -z $NOUSE_NPM ]; then
      # use npm
      cd $entry_path; git init;
      cd - > /dev/null;
      npm install --prefix $MODULE_RPATH
      npm run --prefix $MODULE_RPATH build

    else
      # no npm
      cd $entry_path
      git init

      local PKG_PATH=./package.json
      if ! [ -f $PKG_PATH ]; then
        echo "Error: '$PKG_PATH' doesn't exist." >&2;
        exit 1
      fi

      npm_package_name=`read_json ${PKG_PATH} name`
      npm_package_gbs_config=`read_json ${PKG_PATH} config`
      npm_package_gbs_profile=`read_json ${PKG_PATH} profile`
      npm_package_gbs_option=`read_json ${PKG_PATH} option`

      rm -rf build

      bash -c "gbs -c $npm_package_gbs_config build -A armv7l \
                  --buildroot ~/GBS-ROOT/$npm_package_name \
                  -P $npm_package_gbs_profile $npm_package_gbs_option"

      rm -rf .git
      cd - > /dev/null
    fi
  done
}

if ! [ -d $MODULE_ROOT_RPATH ]; then
  echo "Error: '$PWD/$MODULE_ROOT_RPATH' doesn't exist." >&2;
  exit 1
fi

find_and_build_modules $MODULE_ROOT_RPATH
