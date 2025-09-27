# CMake Version
cmake_minimum_required(VERSION 3.16)

project(NuoSTL CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)

# Warnings
add_compile_options(-Wall)

# Include directorty
include_directories(${PROJECT_SOURCE_DIR}/include)

# Get the source code
# file(GLOB DATA_TYPES ${PROJECT_SOURCE_DIR}/src/core/data_types/*.cpp)

set(CORE
    ${DATA_TYPES}
)

set(NUOSTL
    ${CORE}
)

add_library(NuoSTLLib SHARED ${NUOSTL})