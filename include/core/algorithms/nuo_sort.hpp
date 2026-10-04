#pragma once

#include <functional>

#include "core/algorithms/nuo_heap.hpp"

namespace nuostl
{

template <typename RandomIterator, typename Compare>
void NuoSort(RandomIterator first, RandomIterator last, Compare comp)
{
  NuoMakeHeap(first, last, comp);
  NuoSortHeap(first, last, comp);
}

template <typename RandomIterator>
void NuoSort(RandomIterator first, RandomIterator last)
{
  NuoSort(first, last, std::less<>());
}

} /* namespace nuostl */
