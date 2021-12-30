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

[[ -z $TEST_ROOT ]] && TEST_ROOT=$(pwd)/test
[[ -z $SKIP_TESTS_PATH ]] && SKIP_TESTS_PATH=$TEST_ROOT/skip_tests.txt
# [[ -z $UNSUPPORTED_TESTS_PATH ]] && UNSUPPORTED_TESTS_PATH=$TEST_ROOT/skip_features.txt
# SKIP_TESTS_PATH=./skip_list.gen.txt

echo root: $TEST_ROOT
echo skip: $SKIP_TESTS_PATH
echo drop: $UNSUPPORTED_TESTS_PATH

function cleanup()
{
  local user=$(whoami)

  rm -rf $TEST_ROOT/.tmp.*

  if [[ -z $VM_PATH ]]; then
    ps -fu $user | grep 'Release\/lwnode' | grep -v grep | awk '{print $2}' | xargs -r kill -9
  else
    ps -fu $user | grep $VM_PATH | grep -v grep | awk '{print $2}' | xargs -r kill -9
  fi
  # ps -fu $user | grep '.tmp.' | grep -v grep | awk '{print $2}' | xargs -r kill -9
  ps -fu $user | grep 'defunct' | grep -v grep | awk '{print $2}' | xargs -r kill -9

  echo -e "cleanup remaining processes"
}

trap cleanup SIGINT EXIT

cleanup

ARGS=$*
SKIP_TEST_OPTION=
UNSUPPORTED_TEST_OPTION=

[[ -f $SKIP_TESTS_PATH ]] && SKIP_TEST_OPTION="--skip-tests=$(sed 's/\s#.*//g' $SKIP_TESTS_PATH | paste -sd,)"
[[ -f $UNSUPPORTED_TESTS_PATH ]] && UNSUPPORTED_TEST_OPTION="--unsupported-tests=$(paste -sd, $UNSUPPORTED_TESTS_PATH)"

if [[ -z ${ARGS[0]} ]]; then
  ARGS="test/parallel test/regression"
else
  SKIP_TEST_OPTION=""
fi

tools/test.py -J -p color --report --time \
  --timeout 60 --repeat 1 \
  --test-root=$TEST_ROOT \
  $SKIP_TEST_OPTION \
  $UNSUPPORTED_TEST_OPTION \
  $ARGS
