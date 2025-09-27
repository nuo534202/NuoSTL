cmake_minimum_required(VERSION 3.16)

project(TestNuoSTL CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include
)

# source
# file(GLOB DATA_TYPES ${PROJECT_SOURCE_DIR}/../src/core/data_types/*.cpp)

# set(SRC
#     # C++ Core
#     ${DATA_TYPES}
# )

# test source
file(GLOB TEST_DATA_TYPES ${PROJECT_SOURCE_DIR}/src/core/data_types/*.cpp)

set(TEST_SRC
    ${PROJECT_SOURCE_DIR}/src/test.cpp

    # C++ Core
    ${TEST_DATA_TYPES}
)

set(TEST ${TEST_SRC} ${SRC})

add_executable(test ${TEST})