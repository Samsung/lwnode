#!/bin/bash

export CC="$(which ccache) gcc"
export CXX="$(which ccache) g++"

OPTS="--nopt --node-builtin-modules-path=deps/node"
BUILD_TYPE=Release

[[ $1 =~ ^-.*d ]] && BUILD_TYPE=Debug && OPTS="${OPTS} --debug"

OUTPUT_DIR=out/linux/${BUILD_TYPE}

! [[ $1 =~ ^-.*n ]] && ./configure.py --nopt --shared ${OPTS} && ninja -C ${OUTPUT_DIR} liblwnode

cmake -S test/embedding -B${OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

make -C ${OUTPUT_DIR} -j$(nproc)
