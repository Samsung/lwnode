cmake_minimum_required(VERSION 3.21.3)

project(embedtest)

include_directories("../../include/lwnode")
link_libraries(lwnode pthread)
add_compile_options(-std=c++17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O2 -g)
  link_directories("../../out/linux/Debug/lib")
else()
  link_directories("../../out/linux/Release/lib")
endif()

add_executable(embedtest.x embedtest.cc)
add_executable(example.x example.cc)
