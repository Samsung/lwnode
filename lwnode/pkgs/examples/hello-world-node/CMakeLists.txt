cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project (hello-world)

add_definitions(-DBUILDING_NODE_EXTENSION
                # -DUSING_UV_SHARED=1
                # -DUSING_V8_SHARED=1
                # -DV8_DEPRECATION_WARNINGS=1
                # -D_LARGEFILE_SOURCE
                # -D_FILE_OFFSET_BITS=64
                )

include_directories(include)

file(GLOB SOURCE_FILES "hello.cc")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
