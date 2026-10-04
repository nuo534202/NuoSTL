#include "core/algorithms/test_nuo_algorithm.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

template <typename Iterator, typename Compare>
void AssertNthElementPartition(Iterator first, Iterator nth, Iterator last,
                               Compare comp)
{
  for (Iterator current = first; current != nth; ++current)
  {
    assert(!comp(*nth, *current));
  }
  for (Iterator current = nth; current != last; ++current)
  {
    assert(!comp(*current, *nth));
  }
}

} /* namespace */

void TestNuoAlgorithm::TestNuoAlgorithmSuite()
{
  std::array<int, 4> source = {1, 2, 3, 4};
  std::array<int, 4> copied = {};
  assert(nuostl::NuoCopy(source.begin(), source.end(), copied.begin()) ==
         copied.end());
  assert(copied == source);

  std::array<int, 6> overlapping = {1, 2, 3, 4, 0, 0};
  nuostl::NuoCopyBackward(overlapping.begin(), overlapping.begin() + 4,
                          overlapping.end());
  assert((overlapping == std::array<int, 6>{1, 2, 1, 2, 3, 4}));

  nuostl::NuoFill(copied.begin(), copied.end(), 7);
  assert((copied == std::array<int, 4>{7, 7, 7, 7}));
  assert(nuostl::NuoFillN(copied.begin(), 2, 9) == copied.begin() + 2);
  assert((copied == std::array<int, 4>{9, 9, 7, 7}));
  assert(nuostl::NuoEqual(source.begin(), source.end(), source.begin()));

  const std::array<int, 2> lex_left = {1, 2};
  const std::array<int, 2> lex_right = {1, 3};
  assert(nuostl::NuoLexicographicalCompare(
    lex_left.begin(), lex_left.end(), lex_right.begin(), lex_right.end()));

  assert(nuostl::NuoAccumulate(source.begin(), source.end(), 0) == 10);
  assert(nuostl::NuoAccumulate(source.begin(), source.end(), 1,
                               std::multiplies<int>()) == 24);
  assert(nuostl::NuoFind(source.begin(), source.end(), 3) == source.begin() + 2);
  assert(nuostl::NuoFind(source.begin(), source.end(), 8) == source.end());

  int sum = 0;
  nuostl::NuoForEach(source.begin(), source.end(),
                     [&sum](int value)
                     {
                       sum += value;
                     });
  assert(sum == 10);

  std::array<int, 4> unary_result = {};
  nuostl::NuoTransform(source.begin(), source.end(), unary_result.begin(),
                       [](int value)
                       {
                         return value * 2;
                       });
  assert((unary_result == std::array<int, 4>{2, 4, 6, 8}));

  std::array<int, 4> binary_result = {};
  nuostl::NuoTransform(source.begin(), source.end(), unary_result.begin(),
                       binary_result.begin(), std::plus<int>());
  assert((binary_result == std::array<int, 4>{3, 6, 9, 12}));

  const std::array<int, 3> left = {1, 3, 5};
  const std::array<int, 3> right = {2, 4, 6};
  std::array<int, 6> merged = {};
  assert(nuostl::NuoMerge(left.begin(), left.end(), right.begin(), right.end(),
                          merged.begin()) == merged.end());
  assert((merged == std::array<int, 6>{1, 2, 3, 4, 5, 6}));
  assert(nuostl::NuoBinarySearch(merged.begin(), merged.end(), 4));
  assert(!nuostl::NuoBinarySearch(merged.begin(), merged.end(), 7));

  std::vector<int> sorted_values = {1, 1, 2, 3, 5};
  assert(nuostl::NuoIsSorted(sorted_values.begin(), sorted_values.end()));
  assert(nuostl::NuoIsSortedUntil(sorted_values.begin(),
                                  sorted_values.end()) == sorted_values.end());

  std::vector<int> unsorted_values = {1, 3, 2, 4};
  assert(!nuostl::NuoIsSorted(unsorted_values.begin(), unsorted_values.end()));
  assert(nuostl::NuoIsSortedUntil(unsorted_values.begin(),
                                  unsorted_values.end()) ==
         unsorted_values.begin() + 2);

  std::vector<int> descending_values = {5, 4, 4, 2, 1};
  assert(nuostl::NuoIsSorted(descending_values.begin(),
                             descending_values.end(), std::greater<int>()));
  descending_values[2] = 6;
  assert(nuostl::NuoIsSortedUntil(descending_values.begin(),
                                  descending_values.end(),
                                  std::greater<int>()) ==
         descending_values.begin() + 2);

  std::vector<int> empty_values;
  assert(nuostl::NuoIsSorted(empty_values.begin(), empty_values.end()));
  assert(nuostl::NuoIsSortedUntil(empty_values.begin(),
                                  empty_values.end()) == empty_values.end());

  std::vector<int> nth_values = {9, 1, 5, 3, 7, 3, 2};
  std::vector<int> nth_original = nth_values;
  std::vector<int> sorted_copy = nth_original;
  std::sort(sorted_copy.begin(), sorted_copy.end());
  std::vector<int>::iterator nth =
    nuostl::NuoNthElement(nth_values.begin(), nth_values.end(), 3);
  assert(nth == nth_values.begin() + 3);
  assert(*nth == sorted_copy[3]);
  AssertNthElementPartition(nth_values.begin(), nth, nth_values.end(),
                            std::less<int>());

  std::vector<int> nth_descending = nth_original;
  std::vector<int>::iterator nth_descending_position =
    nuostl::NuoNthElement(nth_descending.begin(), nth_descending.end(), 2,
                          std::greater<int>());
  std::sort(sorted_copy.begin(), sorted_copy.end(), std::greater<int>());
  assert(*nth_descending_position == sorted_copy[2]);
  AssertNthElementPartition(nth_descending.begin(), nth_descending_position,
                            nth_descending.end(), std::greater<int>());

  std::vector<int> one_value = {42};
  assert(nuostl::NuoNthElement(one_value.begin(), one_value.end(), 0) ==
         one_value.begin());
  assert(*one_value.begin() == 42);

  assert(nuostl::NuoNthElement(empty_values.begin(), empty_values.end(), 0) ==
         empty_values.end());

  bool caught_out_of_range = false;
  try
  {
    nuostl::NuoNthElement(nth_values.begin(), nth_values.end(),
                          nth_values.size());
  }
  catch (const std::out_of_range&)
  {
    caught_out_of_range = true;
  }
  assert(caught_out_of_range);

  caught_out_of_range = false;
  try
  {
    nuostl::NuoNthElement(empty_values.begin(), empty_values.end(), 1);
  }
  catch (const std::out_of_range&)
  {
    caught_out_of_range = true;
  }
  assert(caught_out_of_range);

  std::array<int, 5> sortable = {5, 1, 4, 2, 3};
  nuostl::NuoSort(sortable.begin(), sortable.end());
  assert((sortable == std::array<int, 5>{1, 2, 3, 4, 5}));
  nuostl::NuoSort(sortable.begin(), sortable.end(), std::greater<int>());
  assert((sortable == std::array<int, 5>{5, 4, 3, 2, 1}));

  std::vector<int> heap_values = {3, 1, 4, 1, 5, 9, 2};
  nuostl::NuoMakeHeap(heap_values.begin(), heap_values.end());
  assert(nuostl::NuoIsHeap(heap_values.begin(), heap_values.end()));
  heap_values.push_back(10);
  nuostl::NuoPushHeap(heap_values.begin(), heap_values.end());
  assert(nuostl::NuoIsHeap(heap_values.begin(), heap_values.end()));
  nuostl::NuoPopHeap(heap_values.begin(), heap_values.end());
  assert(heap_values.back() == 10);
  heap_values[heap_values.size() - 1] = 0;
  nuostl::NuoMakeHeap(heap_values.begin(), heap_values.end());
  nuostl::NuoSortHeap(heap_values.begin(), heap_values.end());
  assert(std::is_sorted(heap_values.begin(), heap_values.end()));

  std::array<int, 5> min_heap = {3, 1, 4, 2, 5};
  nuostl::NuoMakeHeap(min_heap.begin(), min_heap.end(), std::greater<int>());
  assert(nuostl::NuoIsHeap(min_heap.begin(), min_heap.end(),
                           std::greater<int>()));
  assert(min_heap.front() == 1);
}

} /* namespace test */
