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

} /* namespace nuostl */