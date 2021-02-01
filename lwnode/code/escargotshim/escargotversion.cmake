# Setup output path
set(VERSION_OUTPUT_DIR ${OUTPUT_HEADER_DIR})
if ("${VERSION_OUTPUT_DIR}" STREQUAL "")
  set(VERSION_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/include)
endif()

if ("${VERSION_OUTPUT_NAME}" STREQUAL "")
  set(VERSION_OUTPUT_NAME version.h)
endif()

execute_process(COMMAND git log --pretty=format:'%h' -n 1
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")
    set(GIT_USER "N/A")    
else()
    execute_process(
        COMMAND bash -c "git diff --quiet --exit-code || echo +"
        OUTPUT_VARIABLE GIT_DIFF)
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)
    execute_process(
        COMMAND git config user.name
        OUTPUT_VARIABLE GIT_USER)

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_DIFF}" GIT_DIFF)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
    string(STRIP "${GIT_USER}" GIT_USER)
    string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M")
endif()

set(VERSION "#define GIT_REV \"${GIT_REV}${GIT_DIFF}\"
#define GIT_TAG \"${GIT_TAG}\"
#define GIT_BRANCH \"${GIT_BRANCH}\"
#define GIT_USER \"${GIT_USER}\"
#define BUILD_TIME \"${BUILD_TIME}\"
")

if(EXISTS ${VERSION_OUTPUT_DIR}/${VERSION_OUTPUT_NAME})
    file(READ ${VERSION_OUTPUT_DIR}/${VERSION_OUTPUT_NAME} VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${VERSION_OUTPUT_DIR}/${VERSION_OUTPUT_NAME} "${VERSION}")
endif()
