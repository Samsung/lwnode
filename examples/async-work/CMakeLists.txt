cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project (async-work)

add_definitions(-DBUILDING_NODE_EXTENSION)

include_directories(include)

file(GLOB SOURCE_FILES "async-work.cc")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
