# Unit Test

This directory contains unit tests and example programs for the **NuoSTL** project.  
The goal of these tests is to verify correctness, stability, and compatibility of the custom STL-like components.

## Test Environment

- **Compiler:** gcc 13.2.0
- **C++ Standard:** C++20
- **Build System:** CMake 3.16
- **Operating System:** Windows 11

## Running Tests

Linux and macOS users can run the tests using the following commands:

```
mkdir build
cd build
cmake ..
make
./test
```

Windows users can use the following commands in PowerShell:

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
./test
```