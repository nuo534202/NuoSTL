#include "core/algorithms/test_nuo_numeric.hpp"

#include <cassert>

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

constexpr int kAlphabet = 3;
constexpr int kMaxLength = 5;

std::vector<std::vector<int>> BuildSequences()
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
      sequences.push_back(values);
    }
  }
  return sequences;
}

void CheckIota()
{
  std::vector<int> values(6, -1);
  nuostl::NuoIota(values.begin(), values.end(), 4);
  std::vector<int> expected(6, -1);
  std::iota(expected.begin(), expected.end(), 4);
  assert(values == expected);
  assert(values.front() == 4 && values.back() == 9);

  nuostl::NuoIota(values.begin(), values.begin(), 100);
  assert(values.front() == 4);

  std::vector<int> empty;
  nuostl::NuoIota(empty.begin(), empty.end(), 1);
  assert(empty.empty());

  std::array<double, 4> doubles = {};
  nuostl::NuoIota(doubles.begin(), doubles.end(), 0.5);
  for (std::size_t index = 0; index < doubles.size(); ++index)
  {
    assert(doubles[index] == 0.5 + static_cast<double>(index));
  }

  std::vector<char> letters(3, 'x');
  nuostl::NuoIota(letters.begin(), letters.end(), 'a');
  assert(letters[0] == 'a' && letters[1] == 'b' && letters[2] == 'c');

  nuostl::nuo_vector<int> container(4, 0);
  nuostl::NuoIota(container.begin(), container.end(), 7);
  for (std::size_t index = 0; index < container.size(); ++index)
  {
    assert(container[index] == 7 + static_cast<int>(index));
  }
}

void CheckAdjacentDifference(const std::vector<int>& values)
{
  std::vector<int> actual(values.size() + 2, -99);
  std::vector<int> expected(values.size() + 2, -99);
  const auto actual_end = nuostl::NuoAdjacentDifference(
    values.begin(), values.end(), actual.begin());
  const auto expected_end = std::adjacent_difference(
    values.begin(), values.end(), expected.begin());
  assert(std::distance(actual.begin(), actual_end) ==
         std::distance(expected.begin(), expected_end));
  assert(actual == expected);

  std::vector<int> sum_actual(values.size() + 2, -99);
  std::vector<int> sum_expected(values.size() + 2, -99);
  nuostl::NuoAdjacentDifference(values.begin(), values.end(),
                                sum_actual.begin(), std::plus<>());
  std::adjacent_difference(values.begin(), values.end(),
                           sum_expected.begin(), std::plus<>());
  assert(sum_actual == sum_expected);

  std::vector<int> product_actual(values.size() + 2, -99);
  std::vector<int> product_expected(values.size() + 2, -99);
  nuostl::NuoAdjacentDifference(values.begin(), values.end(),
                                product_actual.begin(), std::multiplies<>());
  std::adjacent_difference(values.begin(), values.end(),
                           product_expected.begin(), std::multiplies<>());
  assert(product_actual == product_expected);

  /* The in-place form must read the previous value before overwriting it. */
  std::vector<int> in_place(values);
  std::vector<int> in_place_expected(values);
  nuostl::NuoAdjacentDifference(in_place.begin(), in_place.end(),
                                in_place.begin());
  std::adjacent_difference(in_place_expected.begin(), in_place_expected.end(),
                           in_place_expected.begin());
  assert(in_place == in_place_expected);

  std::vector<int> in_place_sum(values);
  std::vector<int> in_place_sum_expected(values);
  nuostl::NuoAdjacentDifference(in_place_sum.begin(), in_place_sum.end(),
                                in_place_sum.begin(), std::plus<>());
  std::adjacent_difference(in_place_sum_expected.begin(),
                           in_place_sum_expected.end(),
                           in_place_sum_expected.begin(), std::plus<>());
  assert(in_place_sum == in_place_sum_expected);
}

void CheckPartialSum(const std::vector<int>& values)
{
  std::vector<int> actual(values.size() + 2, -99);
  std::vector<int> expected(values.size() + 2, -99);
  const auto actual_end =
    nuostl::NuoPartialSum(values.begin(), values.end(), actual.begin());
  const auto expected_end =
    std::partial_sum(values.begin(), values.end(), expected.begin());
  assert(std::distance(actual.begin(), actual_end) ==
         std::distance(expected.begin(), expected_end));
  assert(actual == expected);

  std::vector<int> product_actual(values.size() + 2, -99);
  std::vector<int> product_expected(values.size() + 2, -99);
  nuostl::NuoPartialSum(values.begin(), values.end(), product_actual.begin(),
                        std::multiplies<>());
  std::partial_sum(values.begin(), values.end(), product_expected.begin(),
                   std::multiplies<>());
  assert(product_actual == product_expected);

  std::vector<int> max_actual(values.size() + 2, -99);
  std::vector<int> max_expected(values.size() + 2, -99);
  nuostl::NuoPartialSum(values.begin(), values.end(), max_actual.begin(),
                        [](int left, int right)
                        {
                          return left > right ? left : right;
                        });
  std::partial_sum(values.begin(), values.end(), max_expected.begin(),
                   [](int left, int right)
                   {
                     return left > right ? left : right;
                   });
  assert(max_actual == max_expected);

  std::vector<int> in_place(values);
  std::vector<int> in_place_expected(values);
  nuostl::NuoPartialSum(in_place.begin(), in_place.end(), in_place.begin());
  std::partial_sum(in_place_expected.begin(), in_place_expected.end(),
                   in_place_expected.begin());
  assert(in_place == in_place_expected);

  std::vector<int> in_place_product(values);
  std::vector<int> in_place_product_expected(values);
  nuostl::NuoPartialSum(in_place_product.begin(), in_place_product.end(),
                        in_place_product.begin(), std::multiplies<>());
  std::partial_sum(in_place_product_expected.begin(),
                   in_place_product_expected.end(),
                   in_place_product_expected.begin(), std::multiplies<>());
  assert(in_place_product == in_place_product_expected);

  /* A wider accumulator type must not truncate intermediate results. */
  const std::vector<int> wide = {1000000, 1000000, 1000000, 1000000};
  std::vector<long> wide_actual(wide.size(), -1);
  std::vector<long> wide_expected(wide.size(), -1);
  nuostl::NuoPartialSum(wide.begin(), wide.end(), wide_actual.begin());
  std::partial_sum(wide.begin(), wide.end(), wide_expected.begin());
  assert(wide_actual == wide_expected);
}

void CheckInnerProduct(const std::vector<int>& left,
                       const std::vector<int>& right)
{
  const std::size_t overlap = std::min(left.size(), right.size());

  std::vector<int> left_prefix(left.begin(), left.begin() +
    static_cast<std::ptrdiff_t>(overlap));
  std::vector<int> right_prefix(right.begin(), right.begin() +
    static_cast<std::ptrdiff_t>(overlap));

  assert(nuostl::NuoInnerProduct(left_prefix.begin(), left_prefix.end(),
                                 right_prefix.begin(), 0) ==
         std::inner_product(left_prefix.begin(), left_prefix.end(),
                            right_prefix.begin(), 0));

  assert(nuostl::NuoInnerProduct(left_prefix.begin(), left_prefix.end(),
                                 right_prefix.begin(), 100) ==
         std::inner_product(left_prefix.begin(), left_prefix.end(),
                            right_prefix.begin(), 100));

  assert(nuostl::NuoInnerProduct(left_prefix.begin(), left_prefix.end(),
                                 right_prefix.begin(), 0,
                                 std::plus<>(), std::multiplies<>()) ==
         std::inner_product(left_prefix.begin(), left_prefix.end(),
                            right_prefix.begin(), 0,
                            std::plus<>(), std::multiplies<>()));

  /* Swapped operations count equal pairs instead of multiplying them. */
  const auto equal_pairs = [](int first, int second)
  {
    return first == second;
  };
  assert(nuostl::NuoInnerProduct(left_prefix.begin(), left_prefix.end(),
                                 right_prefix.begin(), 0,
                                 std::plus<>(), equal_pairs) ==
         std::inner_product(left_prefix.begin(), left_prefix.end(),
                            right_prefix.begin(), 0,
                            std::plus<>(), equal_pairs));

  /* Enumerate the pairs differently from the default product. */
  assert(nuostl::NuoInnerProduct(left_prefix.begin(), left_prefix.end(),
                                 right_prefix.begin(), 1,
                                 std::multiplies<>(),
                                 [](int first, int second)
                                 {
                                   return first + second + 1;
                                 }) ==
         std::inner_product(left_prefix.begin(), left_prefix.end(),
                            right_prefix.begin(), 1,
                            std::multiplies<>(),
                            [](int first, int second)
                            {
                              return first + second + 1;
                            }));

  /* Floating point accumulation must match the standard as well. */
  const std::vector<double> doubles = {0.5, 1.5, 2.5};
  assert(nuostl::NuoInnerProduct(doubles.begin(), doubles.end(),
                                 doubles.begin(), 0.0) ==
         std::inner_product(doubles.begin(), doubles.end(),
                            doubles.begin(), 0.0));
}

void CheckEmptyAndSingle()
{
  const std::vector<int> empty;
  std::vector<int> output(3, -1);

  assert(nuostl::NuoAdjacentDifference(empty.begin(), empty.end(),
                                       output.begin()) == output.begin());
  assert(nuostl::NuoPartialSum(empty.begin(), empty.end(),
                               output.begin()) == output.begin());
  assert(output[0] == -1);

  assert(nuostl::NuoInnerProduct(empty.begin(), empty.end(), empty.begin(),
                                 42) == 42);

  const std::vector<int> single = {7};
  nuostl::NuoAdjacentDifference(single.begin(), single.end(), output.begin());
  assert(output[0] == 7);
  nuostl::NuoPartialSum(single.begin(), single.end(), output.begin());
  assert(output[0] == 7);
  assert(nuostl::NuoInnerProduct(single.begin(), single.end(), single.begin(),
                                 0) == 49);
}

} /* namespace */

void TestNuoNumeric::TestSuite()
{
  CheckIota();
  CheckEmptyAndSingle();

  const std::vector<std::vector<int>> sequences = BuildSequences();
  for (const std::vector<int>& values : sequences)
  {
    CheckAdjacentDifference(values);
    CheckPartialSum(values);
  }
  for (const std::vector<int>& left : sequences)
  {
    for (const std::vector<int>& right : sequences)
    {
      CheckInnerProduct(left, right);
    }
  }
}

} /* namespace test */
