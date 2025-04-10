cmake_minimum_required(VERSION 3.21.3)

project(embedtest)

set(TARGET_OS "linux")
if (CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
  set(TARGET_OS "tizen/arm")
endif()

set(BUILD_TYPE "Release")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O2 -g)
  set(BUILD_TYPE "Debug")
endif()

include_directories("../../include/lwnode")
find_library(LWNODE_LIB lwnode HINT "../../out/${TARGET_OS}/${BUILD_TYPE}/lib")
link_libraries(pthread ${LWNODE_LIB} -Wl,-rpath='./')

add_compile_options(-std=c++17)

add_executable(embedtest.x embedtest.cc)
add_executable(example.x example.cc)
add_executable(message-port-sync.x message-port-sync.cc)
add_executable(send-message-sync.x send-message-sync.cc)
add_executable(runtime-stop.x runtime-stop.cc)
