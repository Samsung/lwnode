cmake_minimum_required(VERSION 3.9.4)

project(lwnode-example-messageport)

find_library(LWNODE_LIB lwnode HINT lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules (TIZEN_PKGS REQUIRED dlog)

set(SAMPLE_DEFINITIONS
    -DNODE_ARCH=arm,
    -DNODE_PLATFORM=linux,
    -DNODE_WANT_INTERNALS=1,
)

add_executable(${PROJECT_NAME} src/main.cc)
target_include_directories(${PROJECT_NAME} PRIVATE "include" ${TIZEN_PKGS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LWNODE_LIB} ${TIZEN_PKGS_LIBRARIES} -pthread -Wl,-rpath='\$\$ORIGIN/../lib')
target_link_directories(${PROJECT_NAME} PRIVATE ${TIZEN_PKGS_LIBRARY_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${SAMPLE_DEFINITIONS})

