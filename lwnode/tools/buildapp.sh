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

# option defaults
OPT_BUILD_NODE_MODULE=0
OPT_PACKAGE_TPK=0
OPT_PACKAGE_WEB=0
OPT_PACKAGE_HYBRID=0

PASS='\033[32m\u2714\033[0m '
FAIL='\033[31m\u2718\033[0m '

fancy_echo() {
  local BOLD="\033[1m"
  local GREEN="\033[1;32m"
  local CLEAR="\033[0m"
  echo -e ""
  echo -e ${GREEN}$1$2${CLEAR}
}

exec() {
  declare -f -F $1 > /dev/null
  [[ $? -eq 0 ]] && $@;
}

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

  echo $VALUE
}

get_absolute_dir()
{
  local SOURCE=$1
  local DIR=""

  while [ -h "$SOURCE" ]; do
    DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
  done

  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  echo $DIR
}

ensure_absolute_dir()
{
  local DIR=$1
  local BASEDIR=$2

  if [[ "$DIR" = /* ]]; then
    echo $DIR
  else
    echo $BASEDIR/$DIR
  fi
}

ARGS=$@

# print helps
if [[ -z $1 || -z $2 ]]; then
    echo "Usage: buildapp.sh <config file> [options]"
    echo "
    build
    -t | -tpk)       PACKAGE_TPK
    -tc| -tpk-clean) PACKAGE_TPK + BUILD_NODE_MODULE
    -w | -wgt)       PACKAGE_WEB
    -h | -hwgt)      PACKAGE_HYBRID

    test
    -i | -inst)      INSTALL_PKG
    -l | -log)       SHOW_PKG_LOG
    -s | -setup)     SETUP_CONNECTION
    -r | -run)       RUN_PKG
    -tm| -trace-mem) TRACE_MEMORY
    "
    exit 1
fi

# cache the directory the given config exists
CFG_DIR=$(get_absolute_dir $1)

# read user configuration
if [ -f $1 ]; then
  source $1; shift;
else
  fancy_echo "$FAIL '$1' doesn't exist."
  exit 1
fi

# verify definitions
if [[ -z $TPK_PRJ_PATH || -z $TPK_APP_PATH ||
      -z $WGT_PATH || -z $WGT_NAME ||
      -z $SECURITY_PROFILE || -z $PKG_NAME ]]; then
  echo -e $FAIL "Error: One or more configrations are undefined. Check the configuration file."
  exit 1
fi

# ensure an absolute path for tpk root using the path of the config file
TPK_DIR=$(ensure_absolute_dir $TPK_PRJ_PATH $CFG_DIR)

# split tpk build option, the first param is the app path
TPK_APP_RPATH=$(echo "$TPK_APP_PATH" | awk -F' ' '{print $1}')
TPK_NAME=$(read_xml_value $TPK_DIR/$TPK_APP_RPATH/tizen-manifest.xml 'package=')

# more configration
HPREFIX_="hybrid."
OUTPUT_PATH="./out"
TPK=$TPK_NAME.tpk
WGT=$WGT_NAME.wgt
HWGT=$HPREFIX_$WGT

[[ -z $NODE_PATH ]] && { NODE_PATH=$PWD/deps/node-escargot/dist; }
[[ -z $TOOL_PATH ]] && { TOOL_PATH=$(get_absolute_dir $0); }

BUILD_TPK_TOOL=$TOOL_PATH/buildtpk.sh
BUILD_DEPS_TOOL=$TOOL_PATH/builddeps.sh
SHELL_LIB=$TOOL_PATH/libtest.sh

[[ -f $SHELL_LIB ]] && { source $SHELL_LIB; };

# prepare command queue
OPT_QUEUED_TASKS=()
queue()
{
  OPT_QUEUED_TASKS+=("$1")
}

copy_file() {
  NEW_FILE=$1
  OLD_FILE=$2
  # copy it after checking if the new and the old are the same file
  cmp -s $NEW_FILE $OLD_FILE || cp -v $NEW_FILE $OLD_FILE
}

# parsing arguments
while [[ $# -gt 0 ]]; do
    case $1 in
    # build
    -t | -tpk) OPT_PACKAGE_TPK=1 ;;
    -tc| -tpk-clean) OPT_PACKAGE_TPK=1; OPT_BUILD_NODE_MODULE=1 ;;
    -w | -wgt) OPT_PACKAGE_WEB=1 ;;
    -h | -hwgt) OPT_PACKAGE_HYBRID=1 ;;
    # test
    -i | -inst-hwgt)
      queue "install_pkg $OUTPUT_PATH $HWGT $PKG_NAME.$WGT_NAME" ;;
    -iw | -inst-wgt)
      queue "install_pkg $OUTPUT_PATH $WGT" ;;
    -it | -inst-tpk)
      queue "install_pkg $OUTPUT_PATH $TPK" ;;
    -l* | -log*)
      case $1 in (*=*) OPTARG=${1#*=}; esac
      queue "show_pkg_log $OPTARG"
      ;;
    -s | -setup)
      queue "setup_connection $TARGET $HOST_IP" ;;
    -r | -run)
      queue "run_pkg $PKG_NAME" ;;
    -tm| -trace-mem)
      queue "trace_memory 'node'" ;;
    *)
      echo -e $FAIL "Unknown option $1"
      exit -1
      ;;
    esac
    shift
done

# create output path
mkdir -p $OUTPUT_PATH

if [ $OPT_PACKAGE_TPK -eq 1 ]; then
  cd $TPK_DIR

  fancy_echo "build deps"
  if [ $OPT_BUILD_NODE_MODULE -eq 1 ]; then
    if [ -f $BUILD_DEPS_TOOL  ]; then
      $BUILD_DEPS_TOOL deps
    else
      echo -e $FAIL "Error: module build script doesn't exist. '$PWD/tools/build.sh'"
      exit 1
    fi
  fi

  fancy_echo "package tpk"
  if ! [ -f $BUILD_TPK_TOOL ]; then
    echo -e $FAIL "Error: '$BUILD_TPK_TOOL' doesn't exist."
    exit 1
  fi

  # The following should run inside TPK_DIR since
  # TPK_APP_PATH works with relative paths as of now.
  $BUILD_TPK_TOOL $NODE_PATH $TPK_APP_PATH

  cd - > /dev/null

  fancy_echo "copy tpk"
  copy_file $TPK_DIR/out/$TPK $OUTPUT_PATH/$TPK
fi


if [ $OPT_PACKAGE_WEB -eq 1 ]; then
  rm -rf $WGT_PATH/.buildResult
  fancy_echo "build web app"
  tizen build-web -- $WGT_PATH
  fancy_echo "package web app"
  tizen package -t wgt -s $SECURITY_PROFILE -- $WGT_PATH/.buildResult
  fancy_echo "copy web app"
  rm -f $OUTPUT_PATH/$WGT && cp -v $WGT_PATH/.buildResult/$WGT $OUTPUT_PATH;
fi


if [ $OPT_PACKAGE_HYBRID -eq 1 ]; then
  fancy_echo "package hybrid app"
  rm -f $OUTPUT_PATH/$HWGT && cp -v $OUTPUT_PATH/$WGT $OUTPUT_PATH/$HWGT;
  tizen package -t wgt -s $SECURITY_PROFILE -r $OUTPUT_PATH/$TPK -- $OUTPUT_PATH/$HWGT

  # display results
  fancy_echo "results"
  unzip -c $OUTPUT_PATH/$HWGT config.xml tizen-manifest.xml
  du -h $OUTPUT_PATH/$HWGT
  # rm -rf $OUTPUT_PATH/_$HWGT && unzip -q $OUTPUT_PATH/$HWGT -d $OUTPUT_PATH/_$HWGT; # unzip the package
  echo "profile: ${SECURITY_PROFILE}"
fi

# execute queue tasks
for args in "${OPT_QUEUED_TASKS[@]}"; do
  exec $args
done

fancy_echo "$PASS done. ($ARGS)"
