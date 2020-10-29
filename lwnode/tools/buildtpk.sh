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

NODE_EXEC_PATH=$1
TPK_APP_RPATH=$2

PROJ_ROOT=$PWD
BUILD_ROOT=$PROJ_ROOT/out
TIZEN_MAJOR_VERSION=5

if [[ -z $NODE_EXEC_PATH || -z $TPK_APP_RPATH ]]; then
    echo "Usage: buildtpk.sh <node output path> <tpk project path> [node module name]"
    echo "  e.g) buildtpk.sh deps/node-escargot/dist apps/tizen.spotify.node audio-player-node"
    exit 1
fi


if ! [ -f $NODE_EXEC_PATH/node ]; then
    echo "Error: '$NODE_EXEC_PATH/node' doesn't exist."
    exit 1
fi

if ! [ -f $TPK_APP_RPATH/tizen-manifest.xml ]; then
    echo "Error: '$TPK_APP_RPATH/tizen-manifest.xml' doesn't exist."
    exit 1
fi

read_xml_value() {
  local FILE_PATH=$1
  local KEY=$2
  local VALUE=''

  if [[ -z $FILE_PATH || -z $KEY ]]; then
    echo "Usage: ${FUNCNAME[0]} <FILE_PATH> <KEY|ATTRIBUTE_KEY>"
    echo "  example"
    echo "    ${FUNCNAME[0]} manifest.xml 'package='"
    echo "    ${FUNCNAME[0]} manifest.xml '<label>'"
    exit 1
  fi

  if ! [ -f $FILE_PATH ]; then
    echo "Error: '$FILE_PATH' doesn't exist." >&2;
    exit 1
  fi

  if [[ $KEY == *= ]]; then
    VALUE=$(grep -oP "${KEY}\"\K[^\"]*" $FILE_PATH)
  elif [[ $KEY == *\> ]]; then
    VALUE=$(grep -oPm1 "(?<=${KEY})[^<]+" $FILE_PATH)
  else
    echo "Error: No matched key format." >&2;
    exit 1
  fi

  # echo fpath: $FILE_PATH, key: $KEY, result: $VALUE

  echo $VALUE
}

fancy_echo() {
  local BOLD="\033[1m"
  local CLEAR="\033[0m"
  echo -e ${BOLD}$1$2${CLEAR}
}

find_and_install_node_modules() {
  local NODE_MODULE_ROOT_PATH=$1
  local TPK_OUTPUT_ROOT_PATH=$2
  local TARGET_TIZEN_VERSION=$3

  local TARGET_LIB_DIR="tizen${TARGET_TIZEN_VERSION}"
  local PRUNE_LIST=(
    "include" "packaging" "lib" "src" "test" "etc"
    ".git" ".gitignore"
    "*.manifest" "CMakeLists.txt" "package.json"
  )

  # 1. enter 'root/res' folder (assumming entry js is there.)
  cd $TPK_OUTPUT_ROOT_PATH/res

  rm -rf node_modules
  mkdir -p node_modules

  # 2-1. search a full path of each node module inside NODE_MODULE_ROOT_PATH
  for entry_path in "$NODE_MODULE_ROOT_PATH"/*; do
    fancy_echo "install '$entry_path'"

    if [ ! -z $USE_NPM ]; then
      echo "Not required yet to install node modules via npm"
    else
      local module_name=$(basename $entry_path)

      # 2-2. copy prj_root/deps (NODE_MODULE_ROOT_PATH) to ./node_modules/. (tpktmp_root/res/node_modules/.)
      cp -a $entry_path ./node_modules/.

      # 2-3. copy ./node_modules/ into `tpktmp_root/lib` where libs are normally located.
      if [ -d ./node_modules/$module_name/lib ]; then
        if [ -d ./node_modules/$module_name/lib/$TARGET_LIB_DIR ]; then
          cp -va ./node_modules/$module_name/lib/$TARGET_LIB_DIR/. $TPK_OUTPUT_ROOT_PATH/lib/
        else
          cp -va ./node_modules/$module_name/lib/. $TPK_OUTPUT_ROOT_PATH/lib/
        fi
      fi

      # 2.4 prune files in ./node_modules/$module_name
      PRUNE_LIST+=("build/$module_name")
      for i in ${PRUNE_LIST[@]}
      do
        echo "pruning.. node_modules/$module_name/${i}"
        rm -rf ./node_modules/$module_name/${i}
      done
    fi
  done

  cd - > /dev/null
}

# read tpk package name from the manifest file
TPK_NAME=$(read_xml_value $TPK_APP_RPATH/tizen-manifest.xml 'package=')
TPK_FNAME=$TPK_NAME.tpk
TPK_TMP_ROOT_PATH=$BUILD_ROOT/tmp-${TPK_NAME}

# copy package scaffolding
rm -rf $TPK_TMP_ROOT_PATH
mkdir -p $TPK_TMP_ROOT_PATH && cp -a $TPK_APP_RPATH/. $TPK_TMP_ROOT_PATH/

# delete files from the scaffolding
PRUNE_LIST=("package-lock.json" "package.json")
for i in ${PRUNE_LIST[@]}
do
  echo "pruning.. $(basename $TPK_TMP_ROOT_PATH)/res/${i}"
  rm -rf $TPK_TMP_ROOT_PATH/res/${i}
done

# install node modules
if [ -d $PROJ_ROOT/deps ]; then
  find_and_install_node_modules $PROJ_ROOT/deps $TPK_TMP_ROOT_PATH $TIZEN_MAJOR_VERSION
fi

# copy node outputs
mkdir -p $TPK_TMP_ROOT_PATH/bin && cp -uv $NODE_EXEC_PATH/node $TPK_TMP_ROOT_PATH/bin
mkdir -p $TPK_TMP_ROOT_PATH/lib && find $NODE_EXEC_PATH -name "*.so" -exec cp -uv {} $TPK_TMP_ROOT_PATH/lib \;

# pack tpk
cd $BUILD_ROOT
rm -f $TPK_FNAME

fancy_echo "signing files"
curl -o ./kuep_net_signer.sh http://10.40.68.214/kuep_net_signer.sh && chmod +x ./kuep_net_signer.sh
if [ -z ./kuep_net_signer.sh ]; then
  echo "Downloading a signing script failed."
  exit 1
fi
find $TPK_TMP_ROOT_PATH -name 'node' -type f -printf '\n%kK %P\n' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
find $TPK_TMP_ROOT_PATH -name '*.so' -type f -printf '\n%kK %P\n' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
find $TPK_TMP_ROOT_PATH -name '*.node' -type f -printf '\n%kK %P\n' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
rm kuep_net_signer.sh

fancy_echo "zip packaging"
cd $TPK_TMP_ROOT_PATH
zip $BUILD_ROOT/$TPK_FNAME * -r
cd - > /dev/null

fancy_echo "tizen packaging"
tizen package -t tpk -- ./$TPK_FNAME
