#pragma once

#include <functional>

#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

template <typename ForwardIter, typename Compare>
ForwardIter NuoIsSortedUntil(ForwardIter first, ForwardIter last,
                             Compare comp)
{
  if (first == last)
  {
    return last;
  }
  ForwardIter previous = first;
  ForwardIter current = first;
  ++current;
  for (; current != last; ++previous, ++current)
  {
    if (comp(*current, *previous))
    {
      return current;
    }
  }
  return last;
}

template <typename ForwardIter>
ForwardIter NuoIsSortedUntil(ForwardIter first, ForwardIter last)
{
  return NuoIsSortedUntil(first, last, std::less<>());
}

template <typename ForwardIter, typename Compare>
bool NuoIsSorted(ForwardIter first, ForwardIter last, Compare comp)
{
  return NuoIsSortedUntil(first, last, comp) == last;
}

template <typename ForwardIter>
bool NuoIsSorted(ForwardIter first, ForwardIter last)
{
  return NuoIsSortedUntil(first, last) == last;
}

template <typename RandomIter, typename Compare>
RandomIter NuoNthElement(RandomIter first, RandomIter last, size_t n,
                         Compare comp)
{
  const size_t length = static_cast<size_t>(last - first);
  if (length == 0)
  {
    NUO_THROW_OUT_OF_RANGE_IF(n != 0,
                              "nth element index is out of range");
    return last;
  }
  NUO_THROW_OUT_OF_RANGE_IF(n >= length,
                            "nth element index is out of range");

  RandomIter nth = first;
  nth += static_cast<ptrdiff_t>(n);
  for (RandomIter current = first; current != nth; ++current)
  {
    RandomIter best = current;
    RandomIter candidate = current;
    ++candidate;
    for (; candidate != last; ++candidate)
    {
      if (comp(*candidate, *best))
      {
        best = candidate;
      }
    }
    NuoSwap(*current, *best);
  }

  RandomIter best = nth;
  RandomIter candidate = nth;
  ++candidate;
  for (; candidate != last; ++candidate)
  {
    if (comp(*candidate, *best))
    {
      best = candidate;
    }
  }
  NuoSwap(*nth, *best);
  return nth;
}

template <typename RandomIter>
RandomIter NuoNthElement(RandomIter first, RandomIter last, size_t n)
{
  return NuoNthElement(first, last, n, std::less<>());
}

} /* namespace nuostl */
