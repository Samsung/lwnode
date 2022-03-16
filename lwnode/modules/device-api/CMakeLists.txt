cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project (device-api)

find_package (PkgConfig REQUIRED)
pkg_check_modules(PACKAGES REQUIRED dlog)

set(CMAKE_C_FLAGS "-std=gnu++14 -fPIE -Wno-invalid-offsetof -Wno-error=format=")

add_definitions(-DBUILDING_NODE_EXTENSION -DNDEBUG)
include_directories(${LWNODE_INCLUDES})

file (GLOB_RECURSE SOURCE_FILES src/*.cpp)
include_directories(
  ../../../src
  ../../code/escargotshim/deps/escargot/src/api
  ../../code/escargotshim/deps/escargot/third_party/GCutil
  ../../code/escargotshim/deps/escargot/third_party/GCutil/bdwgc/include
  include
  ${PACKAGES_INCLUDE_DIRS}
)


add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} dlog)
