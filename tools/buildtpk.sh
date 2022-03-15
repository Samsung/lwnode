#!/bin/bash

PROJECT_DIR=`pwd`
PACKAGE_NAME='node-sqlite3-service'
TOOLS_DIR=${PROJECT_DIR}/tools
TPK_WORKING_DIR=${PROJECT_DIR}/backend
OUTPUT_DIR=${PROJECT_DIR}/out
TPK_PROJECT_DIR=${OUTPUT_DIR}/tpk

echo_err() {
  echo -e "\e[1;31m${@:1}\e[0m"
}

signing() {
  ./kuep_net_signer.sh -s -tizen_major_ver 4  $1
  if [ $? != 0 ];then
    echo_err "Cannot sign $1"
    exit 1
  fi
}

build() {
  rm -rf ./out/tpk

  # copy node binary
  mkdir -p ${TPK_PROJECT_DIR}/bin
  cp -rf ${TPK_WORKING_DIR}/dist/bin/tizen/node ${TPK_PROJECT_DIR}/bin

  # copy manifest
  cp -rf ${TPK_WORKING_DIR}/tizen-manifest.xml ${TPK_PROJECT_DIR}

  # copy Javascript file with node_modules
  mkdir -p ${TPK_PROJECT_DIR}/res/node_modules
  # TODO: remove test and example file
  rsync -arvmL --delete-excluded --filter="merge ./backend/tools/rpm_package_filter.txt" \
      ${TPK_WORKING_DIR}/* ${TPK_PROJECT_DIR}/res

  # singing
  curl --noproxy '*' -o ./kuep_net_signer.sh http://10.40.68.214/kuep_net_signer.sh && chmod +x ./kuep_net_signer.sh
  if [ -z ./kuep_net_signer.sh ]; then
    echo_err "Downloading a signing script failed."
    exit 1
  fi
  signing ${TPK_PROJECT_DIR}/bin/node
  signing ${TPK_PROJECT_DIR}/res/node_modules/sqlite3/lib/binding/tizen/node_sqlite3.node
  rm -f kuep_net_signer.sh

  # create package file
  cd ${TPK_PROJECT_DIR}
  zip ${PACKAGE_NAME}.tpk * -r
  cd - > /dev/null
  tizen package -t tpk -o ./out -- ${TPK_PROJECT_DIR}/${PACKAGE_NAME}.tpk
  rm -rf ${TPK_PROJECT_DIR}
}

i() {
  local target=$1
  [[ -z "$target" ]] && target=${KIOSK_TARGET_IP}
  [[ -z "$target" ]] && echo "usage: ./tools/buildtpk.sh i <target-ip>" && exit 1

  local filename=$(find ${OUTPUT_DIR} -maxdepth 1 -name "*.tpk" -printf "%f\n")
  local appid="${filename%.*}"
  echo "install and launch ${appid}"

  scp ${OUTPUT_DIR}/${filename} root@${target}:/tmp && \
  ssh -t root@${target} "pkgcmd -i -t tpk -p /tmp/${filename} && app_launcher -s ${appid}"
}

if [ "$1" = "-h" ]; then
  echo "usage:"
  echo "  package tpk: ./tools/buildtpk.sh"
  echo "  install and start app on target: ./tools/buildtpk.sh i <target-ip>"
  exit 0
fi

if declare -f "$1" > /dev/null
then
  "$@"
else
  build
fi
