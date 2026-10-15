# NuoSTL

**Explore STL internals. Go further with mathematical extensions.**

NuoSTL is a C++20 library that implements familiar STL components and adds tools for arbitrary-precision integers, fractions, complex numbers, matrices, and polynomials. It is built for learning, experimentation, and exploring how C++ library components work.

## What You Can Explore

| Area | Components |
| --- | --- |
| Containers | Vectors, arrays, deques, linked lists, ordered and unordered maps/sets, container adapters, fixed-size bitsets, and non-owning spans |
| Data types | Strings, string views, pairs, tuples, optional values, variants, type-erased values, and type-erased callables |
| Memory | Unique and shared ownership, weak references, custom deleters, allocator support, and factory helpers |
| Algorithms & utilities | Sorting, searching, heaps, sequence/set/numeric operations, bit utilities, iterators, function objects, type traits, allocators, alignment helpers, and uninitialized memory algorithms |
| Mathematical extensions | Arbitrary-precision integer arithmetic, rational numbers, complex functions, matrix operations, and polynomial arithmetic |

The implementations and unit tests let you follow each component from its public interface to its underlying behavior. NuoSTL is under development; API names and semantics can differ from the standard library. Check the API reference and tests for component-specific details.

## A First Look

```cpp
#include <iostream>

#include "core/algorithms/nuo_sort.hpp"
#include "core/seq_cont/nuo_vector.hpp"
#include "extra/nuo_big_integer.hpp"

int main()
{
  nuostl::nuo_vector<int> values
  {
    3, 1, 2
  };
  nuostl::NuoSort(values.begin(), values.end());
  std::cout << values.front() << '\n';  /* 1 */

  const auto power = nuostl::NuoBigInteger::Pow(2, 100);
  std::cout << power << '\n';  /* 1267650600228229401496703205376 */
}
```

Include individual component headers or use `nuostl.hpp` for the umbrella header. All public components live in the `nuostl` namespace.

## Getting Started

You need **CMake 3.16+** and a **C++20 compiler**. Fraction arithmetic uses `__int128_t`, so building the full library requires GCC or Clang with 128-bit integer support.

To use a checkout named `NuoSTL` alongside your application's sources, add this to your application's `CMakeLists.txt` after defining its `app` target:

```cmake
add_subdirectory(NuoSTL)
target_compile_features(app PRIVATE cxx_std_20)
target_include_directories(app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/NuoSTL/include")
target_link_libraries(app PRIVATE NuoSTLLib)
```

Template components are implemented in headers; linking `NuoSTLLib` also supplies the compiled implementations, including `NuoBigInteger` and `Fraction`.

To build the library and run the test suite from the repository root:

```sh
cmake -S . -B build
cmake --build build --parallel
cmake -S test -B test/build
cmake --build test/build --parallel
./test/build/test
```

With a multi-configuration generator, build with `--config Debug` and run `./test/build/Debug/test` (`test/build/Debug/test.exe` on Windows).

## Explore Further

The [API reference](docs/api/README.md) covers mathematical extensions with examples, behavior notes, and constraints. The [component roadmap](docs/RoadMap.md) lists implementation progress across the library. Browse [tests](test/) for usage examples and edge cases.

## License

[MIT LICENSE](LICENSE) — free to use, modify, and distribute under the license terms.
