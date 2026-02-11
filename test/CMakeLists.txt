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
file(GLOB_RECURSE TEST_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/../src/*.cpp)

add_executable(test ${TEST_SRC} ${SRC})