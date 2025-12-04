cmake_minimum_required(VERSION 3.16)

project(TestNuoSTL CXX)

set(CMAKE_CXX_STANDARD 23)
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
file(GLOB TEST_DATA_TYPES ${PROJECT_SOURCE_DIR}/src/core/data_types/*.cpp)
file(GLOB TEST_ALGORITHMS ${PROJECT_SOURCE_DIR}/src/core/algorithms/*.cpp)
file(GLOB TEST_SEQ_CONT ${PROJECT_SOURCE_DIR}/src/core/seq_cont/*.cpp)

set(TEST_SRC
    ${PROJECT_SOURCE_DIR}/src/test.cpp

    # C++ Core
    ${TEST_DATA_TYPES}
    ${TEST_ALGORITHMS}
    ${TEST_SEQ_CONT}
)

set(TEST ${TEST_SRC})

add_executable(test ${TEST})