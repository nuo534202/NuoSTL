#include "core/data_types/test_nuo_tuple.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include "nuostl.hpp"

namespace test
{

namespace
{

struct TrackedValue
{
  static inline int live_count = 0;

  explicit TrackedValue(int value);
  ~TrackedValue();
};

TrackedValue::TrackedValue(int value)
{
  if (value < 0)
  {
    throw std::runtime_error("construction failed");
  }
  ++live_count;
}

TrackedValue::~TrackedValue()
{
  --live_count;
}

struct ExplicitValue
{
  explicit ExplicitValue(int value);
  int value;
};

ExplicitValue::ExplicitValue(int value) : value(value)
{
}

struct LessOnly
{
  int value;
};

struct CopyOnly
{
  explicit CopyOnly(int value);
  CopyOnly(const CopyOnly&) = default;
  CopyOnly(CopyOnly&&) = delete;
  int value;
};

CopyOnly::CopyOnly(int value) : value(value)
{
}

bool operator<(const LessOnly& left, const LessOnly& right)
{
  return left.value < right.value;
}

} /* namespace */

void TestNuoTuple::TestNuoTupleSuite()
{
  using Tuple = nuostl::NuoTuple<int, std::string>;
  static_assert(std::tuple_size_v<Tuple> == 2);
  static_assert(std::is_same_v<std::tuple_element_t<0, Tuple>, int>);
  static_assert(std::is_same_v<std::tuple_element_t<1, Tuple>, std::string>);

  constexpr nuostl::NuoTuple<int, int> constexpr_tuple(2, 3);
  static_assert(constexpr_tuple.Get<0>() == 2);
  static_assert(constexpr_tuple.Get<1>() == 3);

  Tuple values(7, "seven");
  assert(values.Get<0>() == 7);
  assert(values.Get<1>() == "seven");
  assert(values.Get<int>() == 7);
  assert(values.Get<std::string>() == "seven");
  assert(nuostl::NuoGet<0>(values) == 7);
  assert(nuostl::NuoGet<1>(values) == "seven");
  assert(nuostl::NuoGet<int>(values) == 7);

  const Tuple const_values(8, "eight");
  static_assert(std::is_same_v<decltype(nuostl::NuoGet<0>(const_values)),
                               const int&>);
  assert(nuostl::NuoGet<0>(const_values) == 8);
  assert(nuostl::NuoGet<std::string>(const_values) == "eight");

  auto [number, word] = values;
  assert(number == 7);
  assert(word == "seven");
  number = 9;
  assert(values.Get<0>() == 7);

  auto made = nuostl::NuoMakeTuple(11, std::string("eleven"));
  static_assert(std::is_same_v<decltype(made),
                               nuostl::NuoTuple<int, std::string>>);
  assert(made.Get<0>() == 11);
  assert(made.Get<1>() == "eleven");

  int first = 0;
  std::string second;
  auto tied = nuostl::NuoTie(first, second);
  tied.Get<0>() = 12;
  tied.Get<1>() = "twelve";
  assert(first == 12);
  assert(second == "twelve");
  static_assert(std::is_same_v<decltype(tied.Get<0>()), int&>);

  int movable = 13;
  auto forwarded = nuostl::NuoForwardAsTuple(std::move(movable));
  static_assert(std::is_same_v<decltype(forwarded.Get<0>()), int&>);
  static_assert(std::is_same_v<decltype(std::move(forwarded).Get<0>()), int&&>);
  assert(forwarded.Get<0>() == 13);
  tied = nuostl::NuoMakeTuple(21, std::string("assigned"));
  assert(first == 21 && second == "assigned");
  const auto converted = nuostl::NuoTuple<long, std::string>(values);
  assert(converted.Get<0>() == 7);
  auto references = nuostl::NuoMakeTuple(std::ref(first));
  references.Get<0>() = 22;
  assert(first == 22);
  auto& [bound_number, bound_word] = values;
  bound_number = 23;
  assert(values.Get<0>() == 23);
  assert(bound_word == "seven");
  static_assert(!std::is_default_constructible_v<nuostl::NuoTuple<int&>>);
  static_assert(!std::is_copy_constructible_v<
    nuostl::NuoTuple<std::unique_ptr<int>>>);
  static_assert(!std::is_copy_assignable_v<nuostl::NuoTuple<const int>>);
  static_assert(std::is_trivially_destructible_v<nuostl::NuoTuple<int>>);
  static_assert(std::is_same_v<decltype(nuostl::NuoGet<0>(
    std::declval<const Tuple&&>())), const int&&>);

  Tuple left(1, "left");
  Tuple right(2, "right");
  assert(left < right);
  assert(left <= right);
  assert(right > left);
  assert(right >= left);
  assert(left != right);
  left.Swap(right);
  assert(left.Get<0>() == 2);
  assert(right.Get<1>() == "left");
  nuostl::NuoSwap(left, right);
  assert(left.Get<0>() == 1);
  assert(right.Get<1>() == "right");

  nuostl::NuoTuple<> empty;
  static_assert(std::tuple_size_v<decltype(empty)> == 0);
  assert(empty == nuostl::NuoTuple<>());

  nuostl::NuoTuple<std::unique_ptr<int>> move_only(
    std::make_unique<int>(42));
  assert(*move_only.Get<0>() == 42);
  auto moved = std::move(move_only);
  assert(*moved.Get<0>() == 42);

  Tuple defaults;
  assert(defaults.Get<0>() == 0 && defaults.Get<1>().empty());
  Tuple copy(values);
  assert(copy == values);
  defaults = copy;
  assert(defaults == copy);
  nuostl::NuoTuple<int, int> repeated_types(1, 2);
  assert(repeated_types.Get<0>() == 1 && repeated_types.Get<1>() == 2);
  auto deduced = nuostl::NuoTuple(1, 2.0);
  static_assert(std::is_same_v<decltype(deduced), nuostl::NuoTuple<int, double>>);
  static_assert(std::is_constructible_v<nuostl::NuoTuple<ExplicitValue>, int>);
  static_assert(!std::is_convertible_v<int, nuostl::NuoTuple<ExplicitValue>>);
  static_assert(std::is_convertible_v<int, nuostl::NuoTuple<int>>);
  static_assert(!std::is_constructible_v<nuostl::NuoTuple<int, int>, int>);
  static_assert(noexcept(nuostl::NuoTuple<int>(1)));
  static_assert(noexcept(repeated_types.Swap(repeated_types)));
  const auto text = nuostl::NuoMakeTuple("literal");
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(text)>,
                               const char* const>);
  assert(std::string(text.Get<0>()) == "literal");
  nuostl::NuoTuple<int> single(31);
  nuostl::NuoTuple<nuostl::NuoTuple<int>> nested(single);
  assert(nested.Get<0>().Get<0>() == 31);
  nuostl::NuoTuple<long> widened(single);
  assert(widened.Get<0>() == 31);
  nuostl::NuoTuple<long> wide_assigned;
  wide_assigned = single;
  assert(wide_assigned.Get<0>() == 31);
  nuostl::NuoTuple<std::unique_ptr<long>> move_assigned;
  move_assigned = nuostl::NuoMakeTuple(std::make_unique<long>(32));
  assert(*move_assigned.Get<0>() == 32);
  int other_first = 33;
  auto other_tie = nuostl::NuoTie(other_first);
  auto first_tie = nuostl::NuoTie(first);
  first_tie.Swap(other_tie);
  assert(first == 33 && other_first == 22);
  first_tie = other_tie;
  assert(first == 22);

  const nuostl::NuoTuple<int, int> lower(1, 100);
  const nuostl::NuoTuple<long, long> higher(2, -100);
  assert(lower < higher && higher > lower);
  assert((lower <=> higher) == std::strong_ordering::less);
  assert((nuostl::NuoTuple<>() <=> empty) == std::strong_ordering::equal);
  nuostl::NuoTuple<double> nan(std::numeric_limits<double>::quiet_NaN());
  nuostl::NuoTuple<double> zero(0.0);
  assert((nan <=> zero) == std::partial_ordering::unordered);
  assert(!(nan <= zero) && !(nan >= zero) && nan != zero);
  const auto less_only = nuostl::NuoTuple<LessOnly>(LessOnly{1});
  const auto greater_only = nuostl::NuoTuple<LessOnly>(LessOnly{2});
  assert(less_only < greater_only);
  static_assert(std::is_same_v<decltype(less_only <=> greater_only),
                               std::weak_ordering>);
  /* Check tuple results against std::tuple for all small integer pairs. */
  for (int a = -2; a <= 2; ++a)
  {
    for (int b = -2; b <= 2; ++b)
    {
      for (int c = -2; c <= 2; ++c)
      {
        assert((nuostl::NuoTuple(a, b) <=> nuostl::NuoTuple(b, c)) ==
               (std::tuple(a, b) <=> std::tuple(b, c)));
      }
    }
  }

  bool construction_failed = false;
  try
  {
    nuostl::NuoTuple<TrackedValue, TrackedValue> throwing(1, -1);
  }
  catch (const std::runtime_error&)
  {
    construction_failed = true;
  }
  assert(construction_failed && TrackedValue::live_count == 0);

  const auto concatenated = nuostl::NuoTupleCat(
    nuostl::NuoTuple<>(), single, nuostl::NuoMakeTuple(std::string("cat")));
  assert(concatenated.Get<0>() == 31 && concatenated.Get<1>() == "cat");
  auto cat_refs = nuostl::NuoTupleCat(nuostl::NuoTie(first));
  cat_refs.Get<0>() = 34;
  assert(first == 34);
  auto cat_move = nuostl::NuoTupleCat(
    nuostl::NuoMakeTuple(std::make_unique<int>(35)));
  assert(*cat_move.Get<0>() == 35);
  static_assert(std::is_same_v<decltype(nuostl::NuoTupleCat()),
                               nuostl::NuoTuple<>>);
  assert(nuostl::NuoApply(
    [](int a, int b)
    {
      return a + b;
    }, repeated_types) == 3);
  assert(nuostl::NuoApply(
    []()
    {
      return 36;
    }, empty) == 36);
  const auto constructed = nuostl::NuoMakeFromTuple<std::string>(
    nuostl::NuoMakeTuple(3, 'x'));
  assert(constructed == "xxx");
  auto applied_move = nuostl::NuoApply(
    [](std::unique_ptr<int> pointer)
    {
      return *pointer;
    }, std::move(cat_move));
  assert(applied_move == 35);
  int extracted = 0;
  nuostl::NuoTie(extracted, std::ignore) = repeated_types;
  assert(extracted == 1);

  using std::swap;
  swap(first_tie, other_tie);
  assert(first == 22 && other_first == 34);
  ExplicitValue object(37);
  auto member_args = nuostl::NuoTie(object);
  static_assert(std::is_same_v<decltype(nuostl::NuoApply(
    &ExplicitValue::value, member_args)), int&>);
  nuostl::NuoApply(&ExplicitValue::value, member_args) = 38;
  assert(object.value == 38);
  auto size_args = nuostl::NuoTie(second);
  assert(nuostl::NuoApply(&std::string::size, size_args) == second.size());
  constexpr auto constexpr_cat = nuostl::NuoTupleCat(
    nuostl::NuoMakeTuple(1, 2), nuostl::NuoMakeTuple(3));
  static_assert(constexpr_cat.Get<2>() == 3);
  static_assert(nuostl::NuoApply(
    [](int a, int b, int c)
    {
      return a + b + c;
    }, constexpr_cat) == 6);
  CopyOnly copy_only_value(39);
  nuostl::NuoTuple<CopyOnly> copy_only_tuple(copy_only_value);
  const auto copy_only_cat = nuostl::NuoTupleCat(
    copy_only_tuple, nuostl::NuoMakeTuple(40), nuostl::NuoTuple<>());
  assert(copy_only_cat.Get<0>().value == 39);
  assert(copy_only_cat.Get<1>() == 40);
  nuostl::NuoTuple<int, std::string> brace_values({41}, {"brace"});
  assert(brace_values.Get<0>() == 41);
}

} /* namespace test */
