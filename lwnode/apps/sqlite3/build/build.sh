#!/bin/bash

# Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

echo -e "\033[0;33m"build sqlite3"\033[0m"

rm -rf $BUILD_OUT_PATH
mkdir -p $BUILD_OUT_PATH

# build native module
echo -e "\033[0;32m"build native module"\033[0m"
cmake $APP_PATH -B$BUILD_OUT_PATH -H$APP_PATH -G Ninja
ninja -C $BUILD_OUT_PATH

# copy native module
echo -e "\033[0;32m"copy native module"\033[0m"
mkdir -p $APP_PATH/node_modules/sqlite3/lib/binding/napi-v3-linux-arm
cp -f $BUILD_OUT_PATH/node_sqlite3.node $APP_PATH/node_modules/sqlite3/lib/binding/napi-v3-linux-arm/

# zip node_modules
echo -e "\033[0;32m"zip node_modules"\033[0m"
SQLITE3_SCRIPT_DIR=$BUILD_OUT_PATH/script
mkdir -p $SQLITE3_SCRIPT_DIR
cd $APP_PATH

find node_modules | rsync -avmL --delete --delete-excluded --filter="merge tools/sqlite3-filter.txt" --files-from=- ./ $SQLITE3_SCRIPT_DIR

cp -rf download $SQLITE3_SCRIPT_DIR
cp -rf out/backend/* $SQLITE3_SCRIPT_DIR # *.js files should have been generated
mkdir -p $BUILD_OUT_PATH/script

cd $BUILD_OUT_PATH
zip -r script.zip ./script/*
cd -

cd -

