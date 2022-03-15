#!/bin/bash

set -e

# mandatory
PROJECT_DIR=`pwd`
WGT_PATH=${PROJECT_DIR}/sample
OUT_PATH=${PROJECT_DIR}/out
WGT_OUT_PATH=${OUT_PATH}/sample
SECURITY_PROFILE=tizen_author

# optional
TARGET=root@kiosk1

fancy_echo() {
  local BOLD="\033[1m"
  local GREEN="\033[1;32m"
  local CLEAR="\033[0m"
  echo -e ""
  echo -e ${GREEN}$1$2${CLEAR}
}

build_frontend() {
  cd $PROJECT_DIR
  npm run build:test_noenv
}

build_wgt() {
  local WGT_PATH=$1
  local OUT_PATH=$2
  local SECURITY_PROFILE=$3

  cd $PROJECT_DIR
  mkdir -p $OUT_PATH
  rm -rf $WGT_OUT_PATH
  mkdir -p $WGT_OUT_PATH

  
  cp -rfL $WGT_PATH $OUT_PATH
  
  # copy test and resource files
  mkdir -p $WGT_OUT_PATH/res
  cp -f $PROJECT_DIR/sample/script/test/support/*.db $WGT_OUT_PATH/res

  # fancy_echo "build web app"
  tizen build-web -- $WGT_OUT_PATH -e "node_modules/*"
  fancy_echo "package web app"
  tizen package -t wgt -s $SECURITY_PROFILE -- $WGT_OUT_PATH/.buildResult
  fancy_echo "copy web app"
  cp -fv $WGT_OUT_PATH/.buildResult/*.wgt $OUT_PATH
  rm -rf $WGT_OUT_PATH
}

install_wgt() {
  local outputdir=$1
  local filename=$(find $outputdir -maxdepth 1 -name "*.wgt" -printf "%f\n")

  # use tizen cli
  # tizen install -n $filename -- $outputdir/

  # use ssh
  local target=$2
  ssh -t $target 'rm /tmp/'$filename'' > /dev/null 2>&1
  scp $outputdir/$filename $target:/tmp/
  ssh -t $target 'pkgcmd -i -t wgt -p /tmp/'$filename''
}

# main

build_wgt $WGT_PATH $OUT_PATH $SECURITY_PROFILE

if [ "$1" = "-i" ]; then
  set +e
  fancy_echo "install web app"
  install_wgt $OUT_PATH $TARGET
  set -e
fi
