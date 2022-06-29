#!/bin/bash

pushd deps/node
./tools/test.py \
    -J -p dots --report --time --timeout 240 --repeat 1 \
    --shell=../../out/linux/Release/lwnode \
    --skip-tests=$(sed 's/\s#.*//g' test/skip_tests.txt | paste -sd,) \
    --unsupported-tests=$(sed '/#\|^$/d' test/skip_features.txt | paste -sd,) \
    test/parallel test/regression
popd

