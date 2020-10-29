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

build() {
  CXXFLAGS+=--coverage LDFLAGS+=-fprofile-arcs ./build.sh -c -ts
  if [ ! $? -eq 0 ]; then
    exit 1
  fi
}

coverage() {
  # generate raw data
  cd ./out/Debug
  ./escargotshim_cctest
  gcov ./obj/deps/escargotshim/src/*.gcno
  lcov -c -d ./obj/deps/escargotshim/src/ -o gumb_before.info
  lcov --remove ./gumb_before.info '/usr/include/*' '*/escargot/*' '*/src/base/*' -o gumb.info
  cd - > /dev/null

  # generate outputs
  mkdir -p coverage
  cp -f ./out/Debug/gumb.info coverage/lcov.info
  genhtml coverage/lcov.info --output-directory coverage/html
  echo -e "\033[0;32mResult: ./coverage/html/index.html\033[0m"
}

if [ -z $1 ]; then
  build
  coverage
else
  OPT_QUEUED_TASKS=()
  queue() { OPT_QUEUED_TASKS+=("$1"); }
  queue_f() { OPT_QUEUED_TASKS=("$1" "${OPT_QUEUED_TASKS[@]}"); }

  opt=$(getopt -o cbrtvl -l build,coverage -- "$@")
  eval set -- "$opt"

  while [[ $# -gt 0 ]]; do
    case "$1" in
      -c | --coverage)
        queue "coverage"
        ;;
      -b | --build)
        queue_f "build"
        ;;
    esac
    shift
  done

  # execute queued tasks
  for args in "${OPT_QUEUED_TASKS[@]}"; do
    echo -e "\033[1;32m${args}\033[0m"
    $args
  done
fi

echo "done."
