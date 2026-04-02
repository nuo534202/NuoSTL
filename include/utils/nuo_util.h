#pragma once

#include <stddef.h>

#include <type_traits>

namespace nuostl
{

/* move */
template <typename T>
typename std::remove_reference<T>::type&& NuoMove(T&& arg) noexcept
{
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

/* forward */
template <typename T>
T&& forward(typename std::remove_reference<T>::type& args) noexcept
{
  return static_cast<T&&>(args);
}

template <typename T>
T&& forward(typename std::remove_reference<T>::type& args) noexcept
{
  static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
  return static_cast<T&&>(args);
}

} /* namespace nuostl */