#include "core/algorithms/test_nuo_algorithm.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <forward_list>
#include <string>
#include <stdexcept>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

constexpr int kSearchValues[] = {1, 2, 2, 4};
static_assert(nuostl::NuoLowerBound(
  kSearchValues, kSearchValues + 4, 2) == kSearchValues + 1);
static_assert(nuostl::NuoUpperBound(
  kSearchValues, kSearchValues + 4, 2) == kSearchValues + 3);
constexpr auto kSearchRange = nuostl::NuoEqualRange(
  kSearchValues, kSearchValues + 4, 2);
static_assert(kSearchRange.first == kSearchValues + 1);
static_assert(kSearchRange.second == kSearchValues + 3);
static_assert(nuostl::NuoBinarySearch(kSearchValues, kSearchValues + 4, 4));
static_assert(!nuostl::NuoBinarySearch(kSearchValues, kSearchValues + 4, 3));
static_assert(nuostl::NuoLowerBound(
  static_cast<const int*>(nullptr), static_cast<const int*>(nullptr), 1) ==
  nullptr);

template <typename Compare>
void AssertSearchMatchesStandard(const std::vector<int>& values, Compare comp)
{
  for (int key = -3; key <= 3; ++key)
  {
    assert(nuostl::NuoLowerBound(values.begin(), values.end(), key, comp) ==
           std::lower_bound(values.begin(), values.end(), key, comp));
    assert(nuostl::NuoUpperBound(values.begin(), values.end(), key, comp) ==
           std::upper_bound(values.begin(), values.end(), key, comp));
    auto actual = nuostl::NuoEqualRange(
      values.begin(), values.end(), key, comp);
    auto expected = std::equal_range(
      values.begin(), values.end(), key, comp);
    assert(actual.first == expected.first);
    assert(actual.second == expected.second);
    assert(nuostl::NuoBinarySearch(values.begin(), values.end(), key, comp) ==
           std::binary_search(values.begin(), values.end(), key, comp));
  }
}

void TestBinarySearchEdges()
{
  /* Exhaust every sorted multiset of length up to six over three values. */
  for (int negative_count = 0; negative_count <= 6; ++negative_count)
  {
    for (int zero_count = 0; zero_count <= 6 - negative_count; ++zero_count)
    {
      for (int positive_count = 0;
           positive_count <= 6 - negative_count - zero_count; ++positive_count)
      {
        std::vector<int> values(negative_count, -2);
        values.insert(values.end(), zero_count, 0);
        values.insert(values.end(), positive_count, 2);
        AssertSearchMatchesStandard(values, std::less<>());
        std::reverse(values.begin(), values.end());
        AssertSearchMatchesStandard(values, std::greater<>());
      }
    }
  }

  /* Each bound requires partitioning by its own comparison direction. */
  const int lower_partition[] = {2, 1, 4, 3};
  const int upper_partition[] = {3, 1, 5, 4};
  assert(nuostl::NuoLowerBound(
    lower_partition, lower_partition + 4, 3) == lower_partition + 2);
  assert(nuostl::NuoUpperBound(
    upper_partition, upper_partition + 4, 3) == upper_partition + 2);

  const std::vector<std::string> words = {"a", "bb", "cc", "ddd"};
  /* Lower bound needs only element-to-key comparison; upper needs the reverse. */
  assert(nuostl::NuoLowerBound(
    words.begin(), words.end(), size_t{2},
    [](const std::string& word, size_t length)
    {
      return word.size() < length;
    }) == words.begin() + 1);
  assert(nuostl::NuoUpperBound(
    words.begin(), words.end(), size_t{2},
    [](size_t length, const std::string& word)
    {
      return length < word.size();
    }) == words.begin() + 3);
  /* Comparator equivalence is independent of operator==. */
  auto by_length = [](const std::string& left, const std::string& right)
  {
    return left.size() < right.size();
  };
  auto equivalent = nuostl::NuoEqualRange(
    words.begin(), words.end(), std::string("zz"), by_length);
  assert(equivalent.first == words.begin() + 1);
  assert(equivalent.second == words.begin() + 3);
  assert(nuostl::NuoBinarySearch(
    words.begin(), words.end(), std::string("zz"), by_length));

  const nuostl::NuoDeque<int> deque = {1, 3, 3, 5};
  auto deque_range = nuostl::NuoEqualRange(deque.Begin(), deque.End(), 3);
  assert(deque_range.first == deque.Begin() + 1);
  assert(deque_range.second == deque.Begin() + 3);
  assert(nuostl::NuoBinarySearch(deque.Begin(), deque.End(), 5));

  const int* null_range = nullptr;
  assert(nuostl::NuoLowerBound(null_range, null_range, 1) == null_range);
  assert(nuostl::NuoUpperBound(null_range, null_range, 1) == null_range);
  auto null_equal = nuostl::NuoEqualRange(null_range, null_range, 1);
  assert(null_equal.first == null_range && null_equal.second == null_range);
  assert(!nuostl::NuoBinarySearch(null_range, null_range, 1));

  std::vector<int> large(1024, 7);
  int comparisons = 0;
  auto counted = [&comparisons](int left, int right)
  {
    ++comparisons;
    return left < right;
  };
  assert(nuostl::NuoLowerBound(
    large.begin(), large.end(), 7, counted) == large.begin());
  assert(comparisons <= 11);
  comparisons = 0;
  assert(nuostl::NuoUpperBound(
    large.begin(), large.end(), 7, counted) == large.end());
  assert(comparisons <= 11);
  comparisons = 0;
  auto large_range = nuostl::NuoEqualRange(
    large.begin(), large.end(), 7, counted);
  assert(large_range.first == large.begin() && large_range.second == large.end());
  assert(comparisons <= 22);

  bool caught = false;
  try
  {
    nuostl::NuoLowerBound(
      large.begin(), large.end(), 7,
      [](int, int) -> bool
      {
        throw std::runtime_error("comparison failed");
      });
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }
  assert(caught);
  assert(std::all_of(large.begin(), large.end(),
                    [](int value)
                    {
                      return value == 7;
                    }));
}

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
  TestBinarySearchEdges();
  TestNuoBinarySearchForwardList();
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

  const std::vector<int> repeated = {1, 2, 2, 2, 4, 6};
  assert(nuostl::NuoLowerBound(repeated.begin(), repeated.end(), 2) ==
         repeated.begin() + 1);
  assert(nuostl::NuoLowerBound(repeated.begin(), repeated.end(), 3) ==
         repeated.begin() + 4);
  assert(nuostl::NuoUpperBound(repeated.begin(), repeated.end(), 2) ==
         repeated.begin() + 4);
  assert(nuostl::NuoUpperBound(repeated.begin(), repeated.end(), 9) ==
         repeated.end());
  auto repeated_range = nuostl::NuoEqualRange(
    repeated.begin(), repeated.end(), 2);
  assert(repeated_range.first == repeated.begin() + 1);
  assert(repeated_range.second == repeated.begin() + 4);

  const std::forward_list<int> forward_values = {1, 3, 3, 5, 8};
  auto forward_lower = nuostl::NuoLowerBound(
    forward_values.begin(), forward_values.end(), 3);
  assert(*forward_lower == 3);
  auto forward_upper = nuostl::NuoUpperBound(
    forward_values.begin(), forward_values.end(), 3);
  assert(*forward_upper == 5);
  assert(nuostl::NuoBinarySearch(
    forward_values.begin(), forward_values.end(), 5));
  assert(!nuostl::NuoBinarySearch(
    forward_values.begin(), forward_values.end(), 4));

  const std::vector<int> descending = {9, 7, 7, 4, 1};
  auto descending_lower = nuostl::NuoLowerBound(
    descending.begin(), descending.end(), 7, std::greater<int>());
  auto descending_upper = nuostl::NuoUpperBound(
    descending.begin(), descending.end(), 7, std::greater<int>());
  assert(descending_lower == descending.begin() + 1);
  assert(descending_upper == descending.begin() + 3);
  assert(nuostl::NuoBinarySearch(
    descending.begin(), descending.end(), 4, std::greater<int>()));

  std::vector<int> empty_range;
  assert(nuostl::NuoLowerBound(empty_range.begin(), empty_range.end(), 1) ==
         empty_range.end());
  auto empty_equal_range = nuostl::NuoEqualRange(
    empty_range.begin(), empty_range.end(), 1);
  assert(empty_equal_range.first == empty_range.end());
  assert(empty_equal_range.second == empty_range.end());

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
