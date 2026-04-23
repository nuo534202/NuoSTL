#pragma once

#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_util.h"

namespace nuostl
{

template <typename ForwardIter, typename Compare>
ForwardIter
NuoIsSortedUntil(ForwardIter first, ForwardIter last, Compare comp)
{
  if (first == last)
    return last;

  ForwardIter cur = first;
  cur++;

  for (; cur != last; first = cur, cur++)
  {
    if (!comp(*first, *cur))
      return cur;
  }

  return last;
}

template <typename ForwardIter>
ForwardIter
NuoIsSortedUntil(ForwardIter first, ForwardIter last)
{
  if (first == last)
    return last;

  ForwardIter cur = first;
  cur++;

  for (; cur != last; first = cur, cur++)
  {
    if (*first > *cur)
      return cur;
  }

  return last;
}

template <typename ForwardIter, typename Compare>
bool
NuoIsSorted(ForwardIter first, ForwardIter last, Compare comp)
{
  return NuoIsSortedUntil(first, last, comp) == last;
}

template <typename ForwardIter>
bool
NuoIsSorted(ForwardIter first, ForwardIter last)
{
  return NuoIsSortedUntil(first, last) == last;
}

template <typename ForwardIter, typename Compare>
ForwardIter
NuoNthElement(ForwardIter first, ForwardIter last, size_t n, Compare comp)
{
  NUO_THROW_OUT_OF_RANGE_IF(NuoDistance(first, last) > n,
    "out of range: there is less than n elements from the input!");

  if (first == last)
    return last;

  ForwardIter tar = first, left = first, right = last;
  while (left != right)
  {
    while (comp(*left, *tar) && left != right)
      left++;
    while (comp(*tar, *right) && left != right)
      right--;

    if (left != right)
    {
      NuoSwap(left, right);
      left++, right--;
    }
  }

  left++;
  size_t k = NuoDistance(first, left);

  if (k <= n)
    return NuoNthElement(left, last, n - k);
  else
    return NuoNthElement(first, left, n);
}

template <typename ForwardIter, typename Compare>
ForwardIter
NuoNthElement(ForwardIter first, ForwardIter last, size_t n)
{
  NUO_THROW_OUT_OF_RANGE_IF(NuoDistance(first, last) > n,
    "out of range: there is less than n elements from the input!");

  if (first == last)
    return last;

  ForwardIter tar = first, left = first, right = last;
  while (left != right)
  {
    while (*left <= *tar && left != right)
      left++;
    while (*right >= *tar && left != right)
      right--;

    if (left != right)
    {
      NuoSwap(left, right);
      left++, right--;
    }
  }

  left++;
  size_t k = NuoDistance(first, left);

  if (k <= n)
    return NuoNthElement(left, last, n - k);
  else
    return NuoNthElement(first, left, n);
}

/*
sort
partial_sort
partial_sort_copy
stable_sort
*/

} /* namespace nuostl */