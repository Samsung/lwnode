#!/bin/bash

ROOT=`pwd`

repo=$1

if [ "$repo" == "lwnode-tizen" ]; then
  echo "Syncing with: lwnode-tizen"
else
  echo "Usage: $0 [ lwnode-tizen ]"
  exit 0
fi

if [ "$repo" == "lwnode-tizen" ] && [ ! -d ../lwnode-tizen ]; then
  echo "Error: $repo not exist. Clone the repo first"
  exit 0
fi

echo "Found: $repo"

git submodule update --init

pushd deps/escargot
git submodule update --init third_party

# Patch update code for wasm
pushd third_party/wasm/wabt
patch -p0 --forward -r /dev/null -i ../../../tools/test/wasm-js/wabt_patch
popd

popd

rsync -armL --delete --delete-excluded --filter="merge tools/release_filter.txt" . ../$repo
if [ $? != 0 ]; then
  echo "Error: rsync failed"
  exit 1
fi

hash=`git log | head -1 | cut -f2 -d' ' | cut -c 1-7`
today=`date +%y%m%d`

pushd ../$repo

git add -A
echo "======================================="
echo git commit -m "Node_Escargot_Release_$today""_$hash"
echo "======================================="
msg="LWNode_Release_$today""_$hash"
git commit -m "$msg"

popd
