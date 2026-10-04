#pragma once

#include <functional>
#include <iterator>

#include "utils/nuo_algorithm.hpp"

namespace nuostl
{

template <typename RandomIterator, typename Distance, typename Compare>
void NuoHeapSiftDown(RandomIterator first, Distance hole, Distance length,
                     Compare comp)
{
  using Value = typename std::iterator_traits<RandomIterator>::value_type;
  Value value = NuoMove(first[hole]);

  while (hole < length / 2)
  {
    Distance child = hole * 2 + 1;
    if (child + 1 < length && comp(first[child], first[child + 1]))
    {
      ++child;
    }
    if (!comp(value, first[child]))
    {
      break;
    }
    first[hole] = NuoMove(first[child]);
    hole = child;
  }
  first[hole] = NuoMove(value);
}

template <typename RandomIterator, typename Compare>
RandomIterator NuoIsHeapUntil(RandomIterator first, RandomIterator last,
                              Compare comp)
{
  using Difference = typename std::iterator_traits<RandomIterator>::difference_type;
  const Difference length = last - first;
  for (Difference child = 1; child < length; ++child)
  {
    const Difference parent = (child - 1) / 2;
    if (comp(first[parent], first[child]))
    {
      return first + child;
    }
  }
  return last;
}

template <typename RandomIterator>
RandomIterator NuoIsHeapUntil(RandomIterator first, RandomIterator last)
{
  return NuoIsHeapUntil(first, last, std::less<>());
}

template <typename RandomIterator, typename Compare>
bool NuoIsHeap(RandomIterator first, RandomIterator last, Compare comp)
{
  return NuoIsHeapUntil(first, last, comp) == last;
}

template <typename RandomIterator>
bool NuoIsHeap(RandomIterator first, RandomIterator last)
{
  return NuoIsHeapUntil(first, last) == last;
}

template <typename RandomIterator, typename Compare>
void NuoMakeHeap(RandomIterator first, RandomIterator last, Compare comp)
{
  using Difference = typename std::iterator_traits<RandomIterator>::difference_type;
  const Difference length = last - first;
  if (length < 2)
  {
    return;
  }
  for (Difference parent = length / 2; parent > 0;)
  {
    --parent;
    NuoHeapSiftDown(first, parent, length, comp);
  }
}

template <typename RandomIterator>
void NuoMakeHeap(RandomIterator first, RandomIterator last)
{
  NuoMakeHeap(first, last, std::less<>());
}

template <typename RandomIterator, typename Compare>
void NuoPushHeap(RandomIterator first, RandomIterator last, Compare comp)
{
  using Difference = typename std::iterator_traits<RandomIterator>::difference_type;
  using Value = typename std::iterator_traits<RandomIterator>::value_type;
  const Difference length = last - first;
  if (length < 2)
  {
    return;
  }

  Difference hole = length - 1;
  Value value = NuoMove(first[hole]);
  while (hole > 0)
  {
    const Difference parent = (hole - 1) / 2;
    if (!comp(first[parent], value))
    {
      break;
    }
    first[hole] = NuoMove(first[parent]);
    hole = parent;
  }
  first[hole] = NuoMove(value);
}

template <typename RandomIterator>
void NuoPushHeap(RandomIterator first, RandomIterator last)
{
  NuoPushHeap(first, last, std::less<>());
}

template <typename RandomIterator, typename Compare>
void NuoPopHeap(RandomIterator first, RandomIterator last, Compare comp)
{
  using Difference = typename std::iterator_traits<RandomIterator>::difference_type;
  const Difference length = last - first;
  if (length < 2)
  {
    return;
  }
  NuoSwap(first[0], first[length - 1]);
  NuoHeapSiftDown(first, Difference(0), length - 1, comp);
}

template <typename RandomIterator>
void NuoPopHeap(RandomIterator first, RandomIterator last)
{
  NuoPopHeap(first, last, std::less<>());
}

template <typename RandomIterator, typename Compare>
void NuoSortHeap(RandomIterator first, RandomIterator last, Compare comp)
{
  while (last - first > 1)
  {
    NuoPopHeap(first, last, comp);
    --last;
  }
}

template <typename RandomIterator>
void NuoSortHeap(RandomIterator first, RandomIterator last)
{
  NuoSortHeap(first, last, std::less<>());
}

} /* namespace nuostl */
