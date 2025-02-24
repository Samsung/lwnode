cmake_minimum_required(VERSION 2.8.12)
project(nd-message-port)

if(USE_MEM_TRACE)
  set(MSG_PORTS_DEFS ${MSG_PORTS_DEFS} USE_MEM_TRACE)
endif()

add_library(nd-message-port async-uv.cc message-port.cc debug-mem-trace.cc)
target_link_libraries(nd-message-port PUBLIC libuv::libuv nd-utils)
target_include_directories(nd-message-port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(nd-message-port PUBLIC MSG_PORTS_DEFS)
if(BUILD_SHARED_LIBS)
  target_compile_options(nd-message-port PRIVATE -fvisibility=hidden)
endif()
