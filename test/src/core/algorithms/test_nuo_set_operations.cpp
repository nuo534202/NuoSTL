#include "core/algorithms/test_nuo_set_operations.hpp"

#include <cassert>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

constexpr int kAlphabet = 3;
constexpr int kMaxLength = 5;

template <typename Container, typename Iterator>
std::ptrdiff_t Offset(Container& container, Iterator position)
{
  return std::distance(container.begin(), position);
}

std::vector<std::vector<int>> BuildSortedSequences()
{
  std::vector<std::vector<int>> sequences;
  for (int length = 0; length <= kMaxLength; ++length)
  {
    std::vector<int> values(static_cast<std::size_t>(length), 0);
    int total = 1;
    for (int index = 0; index < length; ++index)
    {
      total *= kAlphabet;
    }
    for (int code = 0; code < total; ++code)
    {
      int rest = code;
      for (int index = 0; index < length; ++index)
      {
        values[static_cast<std::size_t>(index)] = rest % kAlphabet;
        rest /= kAlphabet;
      }
      std::sort(values.begin(), values.end());
      sequences.push_back(values);
    }
  }
  return sequences;
}

void CheckSetOperations(const std::vector<int>& left,
                        const std::vector<int>& right)
{
  std::vector<int> first(left);
  std::vector<int> second(right);
  std::sort(first.begin(), first.end());
  std::sort(second.begin(), second.end());

  const std::size_t capacity = first.size() + second.size();

  const auto verify = [&](auto nuo_algorithm, auto standard_algorithm)
  {
    std::vector<int> actual(capacity, -1);
    std::vector<int> expected(capacity, -1);
    const auto actual_end = nuo_algorithm(first.begin(), first.end(),
                                          second.begin(), second.end(),
                                          actual.begin());
    const auto expected_end = standard_algorithm(first.begin(), first.end(),
                                                 second.begin(), second.end(),
                                                 expected.begin());
    assert(Offset(actual, actual_end) == Offset(expected, expected_end));
    assert(actual == expected);
  };

  verify(nuostl::NuoSetUnion<decltype(first.begin()),
                             decltype(second.begin()),
                             decltype(first.begin())>,
         std::set_union<decltype(first.begin()), decltype(second.begin()),
                        decltype(first.begin())>);
  verify(nuostl::NuoSetIntersection<decltype(first.begin()),
                                    decltype(second.begin()),
                                    decltype(first.begin())>,
         std::set_intersection<decltype(first.begin()),
                               decltype(second.begin()),
                               decltype(first.begin())>);
  verify(nuostl::NuoSetDifference<decltype(first.begin()),
                                  decltype(second.begin()),
                                  decltype(first.begin())>,
         std::set_difference<decltype(first.begin()),
                             decltype(second.begin()),
                             decltype(first.begin())>);
  verify(nuostl::NuoSetSymmetricDifference<decltype(first.begin()),
                                           decltype(second.begin()),
                                           decltype(first.begin())>,
         std::set_symmetric_difference<decltype(first.begin()),
                                       decltype(second.begin()),
                                       decltype(first.begin())>);
}

void CheckSetOperationsWithCompare(const std::vector<int>& left,
                                   const std::vector<int>& right)
{
  std::vector<int> first(left);
  std::vector<int> second(right);
  std::sort(first.begin(), first.end(), std::greater<>());
  std::sort(second.begin(), second.end(), std::greater<>());

  const std::size_t capacity = first.size() + second.size();

  const auto verify = [&](auto nuo_algorithm, auto standard_algorithm)
  {
    std::vector<int> actual(capacity, -1);
    std::vector<int> expected(capacity, -1);
    const auto actual_end = nuo_algorithm(first.begin(), first.end(),
                                          second.begin(), second.end(),
                                          actual.begin(), std::greater<>());
    const auto expected_end = standard_algorithm(first.begin(), first.end(),
                                                 second.begin(), second.end(),
                                                 expected.begin(),
                                                 std::greater<>());
    assert(Offset(actual, actual_end) == Offset(expected, expected_end));
    assert(actual == expected);
  };

  verify(nuostl::NuoSetUnion<decltype(first.begin()),
                             decltype(second.begin()),
                             decltype(first.begin()), std::greater<>>,
         std::set_union<decltype(first.begin()), decltype(second.begin()),
                        decltype(first.begin()), std::greater<>>);
  verify(nuostl::NuoSetIntersection<decltype(first.begin()),
                                    decltype(second.begin()),
                                    decltype(first.begin()), std::greater<>>,
         std::set_intersection<decltype(first.begin()),
                               decltype(second.begin()),
                               decltype(first.begin()), std::greater<>>);
  verify(nuostl::NuoSetDifference<decltype(first.begin()),
                                  decltype(second.begin()),
                                  decltype(first.begin()), std::greater<>>,
         std::set_difference<decltype(first.begin()),
                             decltype(second.begin()),
                             decltype(first.begin()), std::greater<>>);
  verify(nuostl::NuoSetSymmetricDifference<decltype(first.begin()),
                                           decltype(second.begin()),
                                           decltype(first.begin()),
                                           std::greater<>>,
         std::set_symmetric_difference<decltype(first.begin()),
                                       decltype(second.begin()),
                                       decltype(first.begin()),
                                       std::greater<>>);
}

} /* namespace */

void TestNuoSetOperations::TestSuite()
{
  const std::vector<std::vector<int>> sequences = BuildSortedSequences();
  for (const std::vector<int>& left : sequences)
  {
    for (const std::vector<int>& right : sequences)
    {
      CheckSetOperations(left, right);
      CheckSetOperationsWithCompare(left, right);
    }
  }

  /* Empty inputs must still produce an empty result. */
  std::vector<int> empty;
  std::vector<int> values = {1, 2};
  std::vector<int> output(4, -1);
  assert(Offset(output, nuostl::NuoSetUnion(
                          empty.begin(), empty.end(),
                          empty.begin(), empty.end(), output.begin())) == 0);
  assert(Offset(output, nuostl::NuoSetIntersection(
                          values.begin(), values.end(),
                          empty.begin(), empty.end(), output.begin())) == 0);
  assert(Offset(output, nuostl::NuoSetDifference(
                          values.begin(), values.end(),
                          empty.begin(), empty.end(), output.begin())) == 2);
  assert(Offset(output, nuostl::NuoSetSymmetricDifference(
                          empty.begin(), empty.end(),
                          values.begin(), values.end(), output.begin())) == 2);
}

} /* namespace test */
