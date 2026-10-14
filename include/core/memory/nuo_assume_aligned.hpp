#pragma once

#include <cstddef>
#include <type_traits>

namespace nuostl
{

/* The standard library declaration is
   template <std::size_t Alignment, typename T>
   constexpr T* assume_aligned(T* ptr);
   cppreference.com/w/cpp/memory/assume_aligned.html */
template <std::size_t Alignment, typename T>
constexpr T* NuoAssumeAligned(T* pointer) noexcept
{
  static_assert(!std::is_function_v<T>,
                "NuoAssumeAligned requires an object type");
  static_assert(Alignment != 0 && (Alignment & (Alignment - 1)) == 0,
                "Alignment must be a power of two");
#if defined(__GNUC__) || defined(__clang__)
  if (std::is_constant_evaluated())
  {
    return pointer;
  }
  return static_cast<T*>(__builtin_assume_aligned(pointer, Alignment));
#else
  return pointer;
#endif
}

} /* namespace nuostl */
