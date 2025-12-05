# CMake Version
cmake_minimum_required(VERSION 3.16)

project(NuoSTL CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

# Warnings
add_compile_options(-Wall)

# Include directorty
include_directories(${PROJECT_SOURCE_DIR}/include)

# Get the source code
file(GLOB DATA_TYPES ${PROJECT_SOURCE_DIR}/src/core/data_types/*.cpp)
file(GLOB ALGORITHMS ${PROJECT_SOURCE_DIR}/src/core/algorithms/*.cpp)
file(GLOP SEQ_CONT ${PROJECT_SOURCE_DIR}/src/core/seq_cont/*.cpp)

set(CORE
    ${DATA_TYPES}
    ${ALGORITHMS}
    ${SEQ_CONT}
)

set(NUOSTL
    ${CORE}
)

add_library(NuoSTLLib SHARED ${NUOSTL})

# Place build outputs (DLL/import lib) under the project's bin directory
set_target_properties(NuoSTLLib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)