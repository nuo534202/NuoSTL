#include "utils/test_nuo_functional.hpp"

#include <array>
#include <iterator>
#include <type_traits>
#include <vector>

#include "core/algorithms/nuo_sequence_operations.hpp"
#include "test_framework.hpp"
#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_functional.hpp"

namespace test
{

namespace
{

template <typename Projection, typename Result>
void CheckProjectionAlgorithm(const Projection& projection,
                              const std::vector<Result>& expected)
{
  const std::array<int, 0> empty_first = {};
  const std::array<double, 0> empty_second = {};
  std::vector<Result> empty_projected;
  nuostl::NuoTransform(empty_first.begin(), empty_first.end(),
                       empty_second.begin(),
                       std::back_inserter(empty_projected), projection);
  NUO_TEST_CHECK(empty_projected.empty());

  const std::array<int, 3> first = {1, 2, 3};
  const std::array<double, 3> second = {4.5, 5.5, 6.5};
  std::vector<Result> projected(first.size());
  const auto projected_end =
      nuostl::NuoTransform(first.begin(), first.end(), second.begin(),
                           projected.begin(), projection);
  NUO_TEST_CHECK(projected_end == projected.end());
  NUO_TEST_CHECK(projected == expected);
}

} /* namespace */

void TestNuoFunctional::TestLogicalNot()
{
  const nuostl::NuoLogicalNot<int> logical_not;
  static_assert(std::is_same_v<decltype(logical_not(0)), bool>);
  NUO_TEST_CHECK(logical_not(0));
  NUO_TEST_CHECK(!logical_not(1));

  const std::array<int, 3> falsy = {0, 0, 0};
  NUO_TEST_CHECK(nuostl::NuoAllOf(falsy.begin(), falsy.end(), logical_not));

  const std::array<int, 3> mixed = {0, 1, 0};
  NUO_TEST_CHECK(!nuostl::NuoAllOf(mixed.begin(), mixed.end(), logical_not));

  const std::array<int, 0> empty = {};
  NUO_TEST_CHECK(nuostl::NuoAllOf(empty.begin(), empty.end(), logical_not));
}

void TestNuoFunctional::TestProjectFirst()
{
  const nuostl::NuoProjectFirst<int, double> project_first;
  static_assert(std::is_same_v<decltype(project_first(3, 4.5)), int>);
  NUO_TEST_CHECK(project_first(3, 4.5) == 3);
  CheckProjectionAlgorithm(project_first, std::vector<int>({1, 2, 3}));
}

void TestNuoFunctional::TestProjectSecond()
{
  const nuostl::NuoProjectSecond<int, double> project_second;
  static_assert(std::is_same_v<decltype(project_second(3, 4.5)), double>);
  NUO_TEST_CHECK(project_second(3, 4.5) == 4.5);
  CheckProjectionAlgorithm(project_second,
                           std::vector<double>({4.5, 5.5, 6.5}));
}

void TestNuoFunctional::TestSuite()
{
  TestLogicalNot();
  TestProjectFirst();
  TestProjectSecond();
}

} /* namespace test */
