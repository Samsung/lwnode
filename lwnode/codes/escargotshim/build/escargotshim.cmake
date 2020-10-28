CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# Set


# JS2C
set(JS2C_TOOL "${NODE_ROOT}/tools/js2c.py")
set(JS2C_OUTPUT "${OUTPUT_ROOT}/escargot_natives.h")
set(JS2C_INPUT "lib/escargot_shim.js")
add_custom_target(JS2C
    COMMENT "***** Run js2c *****"
    WORKING_DIRECTORY ${ESCARGOTSHIM_ROOT}
    COMMAND ${JS2C_TOOL}
        --namespace=EscargotShim
         ${JS2C_OUTPUT} ${JS2C_INPUT}
)


# Source Files
file(GLOB_RECURSE ESCARGOTSHIM_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)


# Include Paths
set(ESCARGOTSHIM_INC_PATHS
    ${ESCARGOTSHIM_ROOT}/src 
    ${ESCARGOTSHIM_ROOT}/include
    ${ESCARGOTSHIM_ROOT}/escargot/src/api
    ${ESCARGOTSHIM_ROOT}/escargot/third_party/GCutil
    ${ESCARGOTSHIM_ROOT}/escargot/third_party/GCutil/bdwgc/include
    ${OUTPUT_ROOT}
)


# Defines
set(ESCARGOTSHIM_DEFINES
    ESCARGOT_ENABLE_TYPEDARRAY=1
    ESCARGOT_ENABLE_PROMISE=1
)

if(${ESCARGOT_MODE} STREQUAL debug)
list(APPEND ESCARGOTSHIM_DEFINES
        _GLIBCXX_DEBUG
        GC_DEBUG
    )
elseif(${ESCARGOT_MODE} STREQUAL release)
    list(APPEND ESCARGOTSHIM_DEFINES
        NDEBUG
    )
endif()


# Options
set(ESCARGOTSHIM_FLAGS
    -std=gnu++11
    -g3
    -w
    -fno-omit-frame-pointer
    -fstack-protector
    -fdata-sections
    -ffunction-sections
    -Wno-format
)

if(${ESCARGOT_MODE} STREQUAL "debug")
    list (APPEND ESCARGOTSHIM_FLAGS "-g" "-O0" "-Wall")
elseif(${ESCARGOT_MODE} STREQUAL "release")
    list (APPEND ESCARGOTSHIM_FLAGS 
        "-O3" "-Wall" "-Wextra"
        "-Wno-unused-parameter"
    )
endif()

set(ESCARGOTSHIM_LDFLAGS
)

if(${ESCARGOT_HOST} STREQUAL "tizen_obs")
    if(${TIZEN_MAJOR_VERSION} STREQUAL "6")
        list (APPEND ESCARGOTSHIM_LDFLAGS "-mthumb")
    endif()
endif()

# Libraries
set(ESCARGOTSHIM_STATIC_LIBS
    escargot
    ${GC_TARGET}
)


# Target
message("***** Build EscargotShim *****")
message("*Defines: ${ESCARGOTSHIM_DEFINES}")
message("*Flags: ${ESCARGOTSHIM_FLAGS}")
message("*Include Paths: ${ESCARGOTSHIM_INC_PATHS}")

add_compile_options("-fno-lto")

add_library(${ESCARGOTSHIM_OUTPUT} STATIC ${ESCARGOTSHIM_SRC_FILES})
target_include_directories(${ESCARGOTSHIM_OUTPUT} PUBLIC ${ESCARGOTSHIM_INC_PATHS})
target_compile_definitions(${ESCARGOTSHIM_OUTPUT} PUBLIC ${ESCARGOTSHIM_DEFINES})
target_compile_options(${ESCARGOTSHIM_OUTPUT} PUBLIC ${ESCARGOTSHIM_FLAGS})
target_link_libraries(${ESCARGOTSHIM_OUTPUT} PUBLIC ${ESCARGOTSHIM_STATIC_LIBS} ${ESCARGOTSHIM_LDFLAGS})
set_target_properties(${ESCARGOTSHIM_OUTPUT} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ROOT}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}"
)

add_dependencies(${ESCARGOTSHIM_OUTPUT} JS2C)
