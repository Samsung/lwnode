cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)
project(gmain-loop)

set(CMAKE_C_FLAGS "-std=gnu++1y -fPIE -Wno-invalid-offsetof -Wno-error=format=")

add_definitions(${LWNODE_DEFINITIONS} -DNDEBUG)
include_directories(
  ${LWNODE_INCLUDES}
  ../../code/escargotshim/deps/node-bindings/include
)

file(GLOB_RECURSE SOURCE_FILES src/*.cc)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
