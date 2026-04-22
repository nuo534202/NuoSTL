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
T&& NuoForward(typename std::remove_reference<T>::type& args) noexcept
{
  return static_cast<T&&>(args);
}

template <typename T>
T&& NuoForward(typename std::remove_reference<T>::type&& args) noexcept
{
  static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
  return static_cast<T&&>(args);
}

/* swap */
template <typename T>
void NuoSwap(T& lhs, T& rhs)
{
  T tmp = NuoMove(lhs);
  lhs = NuoMove(rhs);
  rhs = NuoMove(tmp);
}

} /* namespace nuostl */