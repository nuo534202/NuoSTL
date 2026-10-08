#pragma once

#include <functional>
#include <iterator>

#include "core/data_types/nuo_pair.hpp"

namespace nuostl
{

namespace NuoBinarySearchDetail
{

template <typename ForwardIterator>
constexpr auto Distance(ForwardIterator first, ForwardIterator last)
  -> typename std::iterator_traits<ForwardIterator>::difference_type
{
  if (first == last)
  {
    return 0;
  }
  if constexpr (requires
                {
                  last - first;
                })
  {
    return last - first;
  }
  else
  {
    typename std::iterator_traits<ForwardIterator>::difference_type count = 0;
    for (; first != last; ++first)
    {
      ++count;
    }
    return count;
  }
}

template <typename ForwardIterator>
constexpr void Advance(
  ForwardIterator& iterator,
  typename std::iterator_traits<ForwardIterator>::difference_type count)
{
  if constexpr (requires
                {
                  iterator += count;
                })
  {
    iterator += count;
  }
  else
  {
    while (count > 0)
    {
      ++iterator;
      --count;
    }
  }
}

} /* namespace NuoBinarySearchDetail */

template <typename ForwardIterator, typename T, typename Compare>
constexpr ForwardIterator NuoLowerBound(
  ForwardIterator first,
  ForwardIterator last,
  const T& value,
  Compare comp)
{
  using Difference =
    typename std::iterator_traits<ForwardIterator>::difference_type;
  Difference count = NuoBinarySearchDetail::Distance(first, last);
  while (count > 0)
  {
    const Difference step = count / 2;
    ForwardIterator middle = first;
    NuoBinarySearchDetail::Advance(middle, step);
    if (comp(*middle, value))
    {
      first = ++middle;
      count -= step + 1;
    }
    else
    {
      count = step;
    }
  }
  return first;
}

template <typename ForwardIterator, typename T>
constexpr ForwardIterator NuoLowerBound(
  ForwardIterator first,
  ForwardIterator last,
  const T& value)
{
  return NuoLowerBound(first, last, value, std::less<>());
}

template <typename ForwardIterator, typename T, typename Compare>
constexpr ForwardIterator NuoUpperBound(
  ForwardIterator first,
  ForwardIterator last,
  const T& value,
  Compare comp)
{
  using Difference =
    typename std::iterator_traits<ForwardIterator>::difference_type;
  Difference count = NuoBinarySearchDetail::Distance(first, last);
  while (count > 0)
  {
    const Difference step = count / 2;
    ForwardIterator middle = first;
    NuoBinarySearchDetail::Advance(middle, step);
    if (!comp(value, *middle))
    {
      first = ++middle;
      count -= step + 1;
    }
    else
    {
      count = step;
    }
  }
  return first;
}

template <typename ForwardIterator, typename T>
constexpr ForwardIterator NuoUpperBound(
  ForwardIterator first,
  ForwardIterator last,
  const T& value)
{
  return NuoUpperBound(first, last, value, std::less<>());
}

template <typename ForwardIterator, typename T, typename Compare>
constexpr nuo_pair<ForwardIterator, ForwardIterator> NuoEqualRange(
  ForwardIterator first,
  ForwardIterator last,
  const T& value,
  Compare comp)
{
  ForwardIterator lower = NuoLowerBound(first, last, value, comp);
  ForwardIterator upper = NuoUpperBound(lower, last, value, comp);
  return nuo_pair<ForwardIterator, ForwardIterator>(lower, upper);
}

template <typename ForwardIterator, typename T>
constexpr nuo_pair<ForwardIterator, ForwardIterator> NuoEqualRange(
  ForwardIterator first,
  ForwardIterator last,
  const T& value)
{
  return NuoEqualRange(first, last, value, std::less<>());
}

template <typename ForwardIterator, typename T, typename Compare>
constexpr bool NuoBinarySearch(
  ForwardIterator first,
  ForwardIterator last,
  const T& value,
  Compare comp)
{
  first = NuoLowerBound(first, last, value, comp);
  return first != last && !comp(value, *first);
}

template <typename ForwardIterator, typename T>
constexpr bool NuoBinarySearch(
  ForwardIterator first,
  ForwardIterator last,
  const T& value)
{
  return NuoBinarySearch(first, last, value, std::less<>());
}

} /* namespace nuostl */
