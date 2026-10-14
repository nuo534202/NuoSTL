#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_addressof.hpp"
#include "utils/nuo_destroy.hpp"

namespace nuostl
{

/* The standard library declaration is
   template <typename T, typename... Args>
   constexpr T* construct_at(T* location, Args&&... args);
   cppreference.com/w/cpp/memory/construct_at.html */
template <typename T, typename... Args>
constexpr T* NuoConstructAt(T* location, Args&&... args)
{
  return ::new (static_cast<void*>(location)) T(std::forward<Args>(args)...);
}

/* The standard library declaration is
   template <typename T> constexpr void destroy_at(T* location);
   cppreference.com/w/cpp/memory/destroy_at.html
   Array types destroy every element of the array. */
template <typename T>
constexpr void NuoDestroyAt(T* location)
{
  if constexpr (std::is_array_v<T>)
  {
    for (auto& element : *location)
    {
      NuoDestroyAt(NuoAddressof(element));
    }
  }
  else
  {
    location->~T();
  }
}

/* The standard library declaration is
   template <typename ForwardIt, typename Size>
   constexpr ForwardIt destroy_n(ForwardIt first, Size count);
   cppreference.com/w/cpp/memory/destroy_n.html */
template <typename ForwardIt, typename Size>
constexpr ForwardIt NuoDestroyN(ForwardIt first, Size count)
{
  for (; count > 0; ++first, (void)--count)
  {
    NuoDestroyAt(NuoAddressof(*first));
  }
  return first;
}

namespace detail
{

/* Default initialization uses no parentheses so scalar elements keep an
   indeterminate value, while value initialization uses T(). */
template <typename T>
constexpr void NuoDefaultConstructAt(T* location)
{
  ::new (static_cast<void*>(location)) T;
}

template <typename T>
constexpr void NuoValueConstructAt(T* location)
{
  ::new (static_cast<void*>(location)) T();
}

} /* namespace detail */

/* The standard library declaration is
   template <typename InputIt, typename ForwardIt>
   ForwardIt uninitialized_copy(InputIt first, InputIt last,
                                ForwardIt destination);
   The already constructed prefix is destroyed when an initialization
   throws. */
template <typename InputIt, typename ForwardIt>
ForwardIt NuoUninitializedCopy(InputIt first, InputIt last,
                               ForwardIt destination)
{
  ForwardIt current = destination;
  try
  {
    for (; first != last; ++first, (void)++current)
    {
      NuoConstructAt(NuoAddressof(*current), *first);
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(destination, current);
    throw;
  }
}

template <typename InputIt, typename Size, typename ForwardIt>
ForwardIt NuoUninitializedCopyN(InputIt first, Size count,
                                ForwardIt destination)
{
  ForwardIt current = destination;
  try
  {
    for (; count > 0; ++first, (void)++current, (void)--count)
    {
      NuoConstructAt(NuoAddressof(*current), *first);
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(destination, current);
    throw;
  }
}

template <typename ForwardIt, typename T>
void NuoUninitializedFill(ForwardIt first, ForwardIt last, const T& value)
{
  ForwardIt current = first;
  try
  {
    for (; current != last; ++current)
    {
      NuoConstructAt(NuoAddressof(*current), value);
    }
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

template <typename ForwardIt, typename Size, typename T>
ForwardIt NuoUninitializedFillN(ForwardIt first, Size count, const T& value)
{
  ForwardIt current = first;
  try
  {
    for (; count > 0; ++current, (void)--count)
    {
      NuoConstructAt(NuoAddressof(*current), value);
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

template <typename InputIt, typename ForwardIt>
ForwardIt NuoUninitializedMove(InputIt first, InputIt last,
                               ForwardIt destination)
{
  ForwardIt current = destination;
  try
  {
    for (; first != last; ++first, (void)++current)
    {
      NuoConstructAt(NuoAddressof(*current), std::move(*first));
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(destination, current);
    throw;
  }
}

/* The C++17 signature returns both the source and the destination iterator
   past the last processed element. */
template <typename InputIt, typename Size, typename ForwardIt>
std::pair<InputIt, ForwardIt> NuoUninitializedMoveN(
  InputIt first, Size count, ForwardIt destination)
{
  ForwardIt current = destination;
  try
  {
    for (; count > 0; ++first, (void)++current, (void)--count)
    {
      NuoConstructAt(NuoAddressof(*current), std::move(*first));
    }
    return std::make_pair(first, current);
  }
  catch (...)
  {
    NuoDestroy(destination, current);
    throw;
  }
}

template <typename ForwardIt>
void NuoUninitializedDefaultConstruct(ForwardIt first, ForwardIt last)
{
  ForwardIt current = first;
  try
  {
    for (; current != last; ++current)
    {
      detail::NuoDefaultConstructAt(NuoAddressof(*current));
    }
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

template <typename ForwardIt, typename Size>
ForwardIt NuoUninitializedDefaultConstructN(ForwardIt first, Size count)
{
  ForwardIt current = first;
  try
  {
    for (; count > 0; ++current, (void)--count)
    {
      detail::NuoDefaultConstructAt(NuoAddressof(*current));
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

template <typename ForwardIt>
void NuoUninitializedValueConstruct(ForwardIt first, ForwardIt last)
{
  ForwardIt current = first;
  try
  {
    for (; current != last; ++current)
    {
      detail::NuoValueConstructAt(NuoAddressof(*current));
    }
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

template <typename ForwardIt, typename Size>
ForwardIt NuoUninitializedValueConstructN(ForwardIt first, Size count)
{
  ForwardIt current = first;
  try
  {
    for (; count > 0; ++current, (void)--count)
    {
      detail::NuoValueConstructAt(NuoAddressof(*current));
    }
    return current;
  }
  catch (...)
  {
    NuoDestroy(first, current);
    throw;
  }
}

} /* namespace nuostl */
