#
# Copyright 2021-present Samsung Electronics Co., Ltd.
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
#

cmake_minimum_required (VERSION 2.8)
project(sqlite3)

set (HOST "linux" CACHE STRING "HOST")
set (MODE "release" CACHE STRING "MODE")
set (ARCH "x86_64" CACHE STRING "ARCH")
set (COMPILER "gcc" CACHE STRING "COMPILER")
set (LTO "0" CACHE STRING "LTO")
set (ASAN "0" CACHE STRING "ASAN")
set (OUTPUT_DIR ${CMAKE_SOURCE_DIR} CACHE STRING "OUTPUT DIR")
set (TIZEN_MAJOR_VERSION "1" CACHE STRING "TIZEN_MAJOR_VERSION")

# INCLUDE CMAKE FILES
include (build/node_sqlite3.cmake)
