#include "utils/test_nuo_utility.hpp"

#include <assert.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "utils/nuo_utility.hpp"

namespace test
{

namespace
{

template <typename T, T... Values>
constexpr bool SameSequence(nuostl::NuoIntegerSequence<T, Values...>,
                            std::integer_sequence<T, Values...>)
{
  return true;
}

template <typename T>
concept HasAsConst = requires(T&& value)
{
  nuostl::NuoAsConst(std::forward<T>(value));
};

} /* namespace */

void TestNuoUtility::TestIntegerSequence()
{
  static_assert(std::is_same_v<
    nuostl::NuoIntegerSequence<int, 0, 1, 2>::value_type, int>);
  static_assert(nuostl::NuoIntegerSequence<int, 0, 1, 2>::size() == 3);
  static_assert(nuostl::NuoIntegerSequence<int>::size() == 0);

  static_assert(std::is_same_v<
    nuostl::NuoIndexSequence<0, 1>,
    nuostl::NuoIntegerSequence<std::size_t, 0, 1>>);

  static_assert(std::is_same_v<nuostl::NuoMakeIntegerSequence<int, 0>,
                               nuostl::NuoIntegerSequence<int>>);
  static_assert(std::is_same_v<nuostl::NuoMakeIntegerSequence<int, 1>,
                               nuostl::NuoIntegerSequence<int, 0>>);
  static_assert(std::is_same_v<
    nuostl::NuoMakeIntegerSequence<int, 4>,
    nuostl::NuoIntegerSequence<int, 0, 1, 2, 3>>);
  static_assert(std::is_same_v<
    nuostl::NuoMakeIndexSequence<5>,
    nuostl::NuoIndexSequence<0, 1, 2, 3, 4>>);
  static_assert(std::is_same_v<
    nuostl::NuoIndexSequenceFor<int, double, char>,
    nuostl::NuoIndexSequence<0, 1, 2>>);
  static_assert(nuostl::NuoMakeIndexSequence<17>::size() == 17);
  static_assert(nuostl::NuoMakeIndexSequence<64>::size() == 64);

  static_assert(SameSequence(nuostl::NuoMakeIntegerSequence<int, 0>{},
                             std::make_integer_sequence<int, 0>{}));
  static_assert(SameSequence(nuostl::NuoMakeIntegerSequence<int, 5>{},
                             std::make_integer_sequence<int, 5>{}));
  static_assert(SameSequence(nuostl::NuoMakeIndexSequence<7>{},
                             std::make_index_sequence<7>{}));
}

void TestNuoUtility::TestExchange()
{
  int first = 5;
  int second = 5;
  const int first_old = nuostl::NuoExchange(first, 7);
  const int second_old = std::exchange(second, 7);
  assert(first_old == second_old);
  assert(first == second);

  std::string text = "old";
  const std::string old_text = nuostl::NuoExchange(text, "new");
  assert(old_text == "old");
  assert(text == "new");

  std::unique_ptr<int> pointer = std::make_unique<int>(1);
  const std::unique_ptr<int> old_pointer =
    nuostl::NuoExchange(pointer, std::make_unique<int>(2));
  assert(old_pointer != nullptr && *old_pointer == 1);
  assert(pointer != nullptr && *pointer == 2);
}

void TestNuoUtility::TestAsConst()
{
  int value = 5;
  static_assert(std::is_same_v<decltype(nuostl::NuoAsConst(value)),
                               const int&>);
  assert(&nuostl::NuoAsConst(value) == &value);

  const int const_value = 7;
  static_assert(std::is_same_v<decltype(nuostl::NuoAsConst(const_value)),
                               const int&>);
  assert(nuostl::NuoAsConst(const_value) == 7);

  static_assert(HasAsConst<int&>);
  static_assert(!HasAsConst<int>);
}

void TestNuoUtility::TestDeclval()
{
  static_assert(std::is_same_v<decltype(nuostl::NuoDeclval<int>()), int&&>);
  static_assert(std::is_same_v<decltype(nuostl::NuoDeclval<int&>()), int&>);
  static_assert(std::is_same_v<decltype(nuostl::NuoDeclval<int&&>()),
                               int&&>);
  static_assert(std::is_same_v<decltype(nuostl::NuoDeclval<const int&>()),
                               const int&>);
}

void TestNuoUtility::TestSuite()
{
  TestIntegerSequence();
  TestExchange();
  TestAsConst();
  TestDeclval();
}

} /* namespace test */
