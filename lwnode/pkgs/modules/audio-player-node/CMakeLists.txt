cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project (audio-player)

add_definitions(-DBUILDING_NODE_EXTENSION)

include_directories(include src)
link_directories(${CMAKE_SOURCE_DIR}/lib)

file (GLOB_RECURSE SOURCE_FILES src/*.cc)

add_definitions(-DTARGET_OS_TIZEN)

find_package (PkgConfig REQUIRED)
pkg_check_modules(PACKAGES REQUIRED capi-media-audio-io capi-media-player capi-media-sound-manager)
include_directories(${PACKAGES_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} capi-media-audio-io capi-media-player capi-media-sound-manager tvbridge pthread)
add_definitions(-DSPOTIFY_USERNAME="" 
                -DSPOTIFY_PASSWORD=""
                )
