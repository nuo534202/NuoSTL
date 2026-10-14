#pragma once

namespace nuostl
{

/* The standard library declaration is
   template <typename T> constexpr T* addressof(T& value) noexcept;
   cppreference.com/w/cpp/memory/addressof.html
   The builtin is used so that a class type with an overloaded operator& still
   yields the address of the object itself. */
template <typename T>
constexpr T* NuoAddressof(T& value) noexcept
{
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_addressof(value);
#else
  return reinterpret_cast<T*>(
    &const_cast<char&>(reinterpret_cast<const volatile char&>(value)));
#endif
}

template <typename T>
const T* NuoAddressof(const T&&) = delete;

} /* namespace nuostl */
