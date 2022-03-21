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
project(node_sqlite3)

# set (HOST "linux" CACHE STRING "HOST")
set (MODE "release" CACHE STRING "MODE")
# set (ARCH "x86_64" CACHE STRING "ARCH")
# set (COMPILER "gcc" CACHE STRING "COMPILER")
set (LTO "0" CACHE STRING "LTO")
set (ASAN "0" CACHE STRING "ASAN")
# set (OUTPUT_DIR ${CMAKE_SOURCE_DIR} CACHE STRING "OUTPUT DIR")
set (TIZEN_MAJOR_VERSION "1" CACHE STRING "TIZEN_MAJOR_VERSION")



# set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
set(SQLITE3_ROOT node_modules/sqlite3)
set(BUILD_MODE "release" CACHE STRING "MODE")

# Source Files
file(GLOB_RECURSE SQLITE3_SRC_FILES ${SQLITE3_ROOT}/src/*.cc)

# Include Paths
set(SQLITE3_INC_PATHS
    src
    deps/include
)

# Options
set(SQLITE3_FLAGS
    -std=gnu++11
    -g
    -fstack-protector
    -fdata-sections
    -ffunction-sections
    -Wno-format
)
if(${BUILD_MODE} STREQUAL debug)
    list(APPEND SQLITE3_FLAGS
        -O0
    )
elseif(${BUILD_MODE} STREQUAL release)
    list(APPEND SQLITE3_FLAGS
        -O3
    )
endif()


# Defines
set(SQLITE3_DEFINES
    BUILDING_NODE_EXTENSION=1
)
if(${BUILD_MODE} STREQUAL debug)
    list(APPEND SQLITE3_DEFINES
        _GLIBCXX_DEBUG
        GC_DEBUG
        DEBUG
    )
elseif(${BUILD_MODE} STREQUAL release)
    list(APPEND SQLITE3_DEFINES
        NDEBUG)
endif()

# Library
find_library(LIB_SQLITE3 NAMES sqlite3)

# Target
add_library(${PROJECT_NAME} SHARED ${SQLITE3_SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SQLITE3_INC_PATHS})
target_compile_options(${PROJECT_NAME} PUBLIC ${SQLITE3_FLAGS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SQLITE3_DEFINES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB_SQLITE3})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
