#include "core/algorithms/test_nuo_algorithm.hpp"

#include <cassert>
#include <functional>

#include "core/algorithms/nuo_binary_search.hpp"
#include "core/seq_cont/nuo_forward_list.hpp"

namespace test
{

void TestNuoBinarySearchForwardList()
{
  const nuostl::NuoForwardList<int> values = {1, 3, 3, 5};
  auto lower = values.Begin();
  ++lower;
  auto upper = lower;
  ++upper;
  ++upper;
  assert(nuostl::NuoLowerBound(values.Begin(), values.End(), 3) == lower);
  assert(nuostl::NuoUpperBound(values.Begin(), values.End(), 3) == upper);
  auto range = nuostl::NuoEqualRange(values.Begin(), values.End(), 3);
  assert(range.first == lower && range.second == upper);
  auto missing = nuostl::NuoEqualRange(values.Begin(), values.End(), 4);
  assert(missing.first == upper && missing.second == upper);
  assert(nuostl::NuoBinarySearch(values.Begin(), values.End(), 5));
  assert(!nuostl::NuoBinarySearch(values.Begin(), values.End(), 4));

  const nuostl::NuoForwardList<int> descending = {5, 3, 3, 1};
  auto descending_lower = descending.Begin();
  ++descending_lower;
  auto descending_upper = descending_lower;
  ++descending_upper;
  ++descending_upper;
  auto descending_range = nuostl::NuoEqualRange(
    descending.Begin(), descending.End(), 3, std::greater<>());
  assert(descending_range.first == descending_lower);
  assert(descending_range.second == descending_upper);

  nuostl::NuoForwardList<int> empty;
  auto empty_range = nuostl::NuoEqualRange(empty.Begin(), empty.End(), 3);
  assert(empty_range.first == empty.End() && empty_range.second == empty.End());
  assert(!nuostl::NuoBinarySearch(empty.Begin(), empty.End(), 3));
}

} /* namespace test */
