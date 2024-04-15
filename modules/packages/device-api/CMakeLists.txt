cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)
project (device-api)

find_package (PkgConfig REQUIRED)
pkg_check_modules(PACKAGES REQUIRED dlog glib-2.0 pkgmgr-info tpk-manifest-handlers libevdev)

set(CMAKE_C_FLAGS "-std=gnu++14 -fPIE -Wno-invalid-offsetof -Wno-error=format=")

add_definitions(${LWNODE_DEFINITIONS} -DNDEBUG)
include_directories(${LWNODE_INCLUDES})

file (GLOB_RECURSE SOURCE_FILES src/*.cpp)
include_directories(
  ${PROJECT_ROOT_PATH}/deps/escargot/src/api
  ${PROJECT_ROOT_PATH}/deps/escargot/third_party/GCutil
  ${PROJECT_ROOT_PATH}/deps/escargot/third_party/GCutil/bdwgc/include
  ${PROJECT_ROOT_PATH}/deps/node/deps/uv/include
  ${PROJECT_ROOT_PATH}/src
  include
  ${PACKAGES_INCLUDE_DIRS}
)


add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${PACKAGES_LIBRARIES} udev)
