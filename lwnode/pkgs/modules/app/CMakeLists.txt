cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project (app)

find_package (PkgConfig REQUIRED)
pkg_check_modules(PACKAGES REQUIRED capi-appfw-app-common)
include_directories(${PACKAGES_INCLUDE_DIRS})

file (GLOB_RECURSE SOURCE_FILES src/*.cc)
add_definitions(-DBUILDING_NODE_EXTENSION)
include_directories(include)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} capi-appfw-app-common)
