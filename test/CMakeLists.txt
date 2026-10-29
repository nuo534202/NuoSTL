cmake_minimum_required(VERSION 3.16)

project(TestNuoSTL CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_compile_options(-Wall)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include
)

# test source
file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/../src/*.cpp)

add_executable(test ${TEST_SRC} ${SRC})

add_executable(test_process_failure_probe
    harness/test_process_failure_probe.cpp
    src/test_framework.cpp)
set(PROBE_SOURCE_FILE
    "${PROJECT_SOURCE_DIR}/harness/test_process_failure_probe.cpp")
set(PROBE_DETACHED_EXPRESSION "detached.check+value")
set(PROBE_DETACHED_FAILURE_COUNT 3)
set(PROBE_ENCLOSING_SUITE_NAME "outer suite")
set(PROBE_TIMEOUT 30 CACHE STRING
    "Timeout in seconds for the process failure probe")
target_compile_definitions(test_process_failure_probe PRIVATE
    "NUO_DETACHED_CHECK_EXPRESSION=\"${PROBE_DETACHED_EXPRESSION}\""
    NUO_DETACHED_FAILURE_COUNT=${PROBE_DETACHED_FAILURE_COUNT}
    "NUO_PROBE_ENCLOSING_SUITE_NAME=\"${PROBE_ENCLOSING_SUITE_NAME}\"")
add_custom_command(
    TARGET test_process_failure_probe
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DPROBE_EXECUTABLE=$<TARGET_FILE:test_process_failure_probe>
        -DPROBE_SOURCE_FILE=${PROBE_SOURCE_FILE}
        "-DPROBE_DETACHED_EXPRESSION=${PROBE_DETACHED_EXPRESSION}"
        -DPROBE_DETACHED_FAILURE_COUNT=${PROBE_DETACHED_FAILURE_COUNT}
        "-DPROBE_ENCLOSING_SUITE_NAME=${PROBE_ENCLOSING_SUITE_NAME}"
        -DPROBE_TIMEOUT=${PROBE_TIMEOUT}
        -P ${PROJECT_SOURCE_DIR}/harness/check_process_failure.cmake)

# Keep legacy assert-based suites active without disabling NDEBUG elsewhere.
foreach(TEST_SOURCE ${TEST_SRC})
    get_filename_component(TEST_SOURCE_NAME "${TEST_SOURCE}" NAME)
    if(NOT TEST_SOURCE_NAME STREQUAL "test.cpp"
       AND NOT TEST_SOURCE_NAME STREQUAL "test_framework.cpp"
       AND NOT TEST_SOURCE_NAME STREQUAL "test_harness.cpp")
        if(MSVC)
            set_source_files_properties(
                ${TEST_SOURCE} PROPERTIES COMPILE_OPTIONS "/UNDEBUG")
        else()
            set_source_files_properties(
                ${TEST_SOURCE} PROPERTIES COMPILE_OPTIONS "-UNDEBUG")
        endif()
    endif()
endforeach()
