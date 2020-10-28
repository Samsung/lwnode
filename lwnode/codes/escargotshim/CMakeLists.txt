cmake_minimum_required (VERSION 2.8)
project (ESCARGOTSHIM)

set (ESCARGOT_ROOT ${CMAKE_SOURCE_DIR}/escargot)
set (ESCARGOT_MODE ${MODE})
set (ESCARGOT_ARCH ${ARCH})
set (ESCARGOT_OUTPUT static_lib)

if (${HOST} STREQUAL "linux")
    set (ESCARGOT_HOST ${HOST})
else()
    set (ESCARGOT_HOST "tizen_obs")
    set (ESCARGOT_LIBICU_SUPPORT OFF)
    set (ESCARGOT_SMALL_CONFIG ON)
    set (ESCARGOT_GC_SMALL_CONFIG ON)
endif()

set (OUTBUT_ROOT ${PROJECT_SOURCE_DIR}/out/${HOST}/${ARCH}/${MODE}/)

set(ENV{CXXFLAGS} "-Wno-error -Wno-uninitialized -Wno-strict-aliasing $ENV{CXXFLAGS}")

message ("***Build Escargot: ${ESCARGOT_MODE} ${ESCARGOT_ARCH} ${ESCARGOT_HOST}")

if (${VALGRIND} STREQUAL "ON")
    set(ESCARGOT_PROFILE_BDWGC ON)
    set(ESCARGOT_MEM_STATS ON)
    set(ESCARGOT_VALGRIND ON)
endif()

add_subdirectory (escargot ${OUTBUT_ROOT})

set (VERSION_OUTPUT_NAME escargotshim-version.h)
include (escargotversion.cmake)
