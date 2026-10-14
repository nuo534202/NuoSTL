#pragma once

#include <type_traits>

namespace nuostl
{

/* The standard library declaration is
   template <typename T> constexpr T* launder(T* p) noexcept;
   cppreference.com/w/cpp/utility/launder.html */
template <typename T>
constexpr T* NuoLaunder(T* pointer) noexcept
{
  static_assert(!std::is_function_v<T>,
                "NuoLaunder requires an object type");
  static_assert(!std::is_void_v<T>,
                "NuoLaunder requires an object type");
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_launder(pointer);
#else
  return pointer;
#endif
}

} /* namespace nuostl */
