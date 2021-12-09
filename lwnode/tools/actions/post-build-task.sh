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

set -e

SCRIPT_PATH="$( cd "$(dirname "$0")" >/dev/null 2>&1; pwd )"

show_output_info() {
  ! [[ -z $2 ]] && echo && find $1 -name "lwnode" -o -name "*.so*" | grep -v "/obj" | xargs strip -v $2
  echo; find $1 -name "lwnode" -o -name "*.so*" | grep -v "/obj\|TOC\|/licenses" | xargs size -t
  echo; find $1 -name "lwnode" -o -name "*.so*" -o -name "*.dat" | grep -v "/obj\|TOC\|/licenses" | xargs du -s
}

unpack_rpm() {
  local gbs_build_root_name=$1
  local dest_path=$2
  local rpm_output_path=~/GBS-ROOT/$gbs_build_root_name/local/repos/t65std/armv7l/RPMS

  rm -rf $dest_path;
  mkdir -p $dest_path;

  echo -e "\nunpack devel rpm\n"

  cd $dest_path
  find $rpm_output_path -name '*.rpm' | grep -e '/[a-z]*-[0-9]\|-devel' | xargs -I {} sh -c "rpm2cpio {} | cpio -idm"
  find usr/bin usr/lib -name "lwnode*" -o -name "*.so*" | xargs tar --transform 's/.*\///g' -cf lwnode.tar
  tar -xf lwnode.tar
  cd - > /dev/null

  rm -rf $dest_path/usr $dest_path/lwnode.tar
}

create_build_info_sh() {
  local filename="build-info.txt"
  local dest_path=$1
  local target_path=$dest_path/$filename

  rm -f $target_path

  # hash, timestamp, branch name
  echo $(git log -1 --format=%h) >> $target_path
  echo $(git log -1 --format=%ct) >> $target_path
  echo $(git rev-parse --abbrev-ref HEAD) >> $target_path
}

create_build_info() {
  local dest_path=$1
  bash -c "node $SCRIPT_PATH/build-info.mjs $dest_path"
}

post() {
  unpack_rpm lwnode rpm
  show_output_info rpm
}

if [ $? -eq 0 ]; then
  post
fi
