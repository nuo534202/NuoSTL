#include "core/data_types/test_nuo_variant.hpp"

#include <cassert>

#include <memory>
#include <vector>
#include <variant>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "nuostl.hpp"

namespace test
{

namespace
{

struct ThrowingValue
{
  static inline int live_count = 0;
  explicit ThrowingValue(int value);
  ThrowingValue(const ThrowingValue& other);
  ThrowingValue(ThrowingValue&& other);
  ThrowingValue& operator=(const ThrowingValue&) = default;
  ThrowingValue& operator=(ThrowingValue&&) = default;
  ~ThrowingValue();
  static inline bool fail_copy = false;
  int value;
};

ThrowingValue::ThrowingValue(int number) : value(number)
{
  if (number < 0)
  {
    throw std::runtime_error("construction failed");
  }
  ++live_count;
}

ThrowingValue::ThrowingValue(const ThrowingValue& other)
  : ThrowingValue(fail_copy ? -1 : other.value)
{
}

ThrowingValue::ThrowingValue(ThrowingValue&& other)
  : ThrowingValue(other.value)
{
}

ThrowingValue::~ThrowingValue()
{
  --live_count;
}

} /* namespace */

void TestNuoVariant::TestNuoVariantSuite()
{
  using Variant = nuostl::NuoVariant<int, std::string>;
  static_assert(nuostl::NuoVariantSize<Variant>::value == 2);
  static_assert(std::is_same_v<nuostl::NuoVariantAlternativeT<0, Variant>, int>);

  static_assert(std::is_same_v<nuostl::NuoVariantAlternativeT<1, const Variant>,
                               const std::string>);
  static_assert(!std::is_copy_constructible_v<
    nuostl::NuoVariant<int, std::unique_ptr<int>>>);
  static_assert(!std::is_constructible_v<nuostl::NuoVariant<int>, double>);
  static_assert(!std::is_constructible_v<nuostl::NuoVariant<int, int>, int>);
  static_assert(std::is_trivially_copyable_v<nuostl::NuoVariant<int, double>>);
  constexpr nuostl::NuoVariant<int, long> constant(7);
  static_assert(constant.Index() == 0);
  static_assert(nuostl::NuoGet<0>(constant) == 7);

  Variant number(7);
  assert(number.Index() == 0);
  assert(nuostl::NuoHoldsAlternative<int>(number));
  assert(nuostl::NuoGet<int>(number) == 7);
  assert(nuostl::NuoGet<0>(number) == 7);

  number = std::string("seven");
  assert(number.Index() == 1);
  assert(nuostl::NuoGet<std::string>(number) == "seven");
  assert(nuostl::NuoGetIf<std::string>(&number) != nullptr);
  assert(nuostl::NuoGetIf<int>(&number) == nullptr);

  number.Emplace<0>(42);
  assert(nuostl::NuoGet<0>(number) == 42);
  number.Emplace<std::string>("text");
  assert(nuostl::NuoGet<std::string>(number) == "text");

  int visited = nuostl::NuoVisit(
    [](const auto& value)
    {
      using Value = std::decay_t<decltype(value)>;
      if constexpr (std::is_same_v<Value, std::string>)
      {
        return static_cast<int>(value.size());
      }
      else
      {
        return value;
      }
    }, number);
  assert(visited == 4);
  int visitor_calls = 0;
  bool visitor_exception = false;
  try
  {
    nuostl::NuoVisit([&visitor_calls](const auto&) -> int
    {
      ++visitor_calls;
      throw std::runtime_error("visitor failed");
    }, number);
  }
  catch (const std::runtime_error&)
  {
    visitor_exception = true;
  }
  assert(visitor_exception && visitor_calls == 1);
  assert(nuostl::NuoGet<1>(number) == "text");

  Variant copy(number);
  assert(copy == number);
  copy = 2;
  assert(copy != number);
  assert(copy < number && copy <= number);
  assert(number > copy && number >= copy);

  Variant text("hello");
  assert(text.Index() == 1);
  assert(nuostl::NuoGet<1>(text) == "hello");
  const Variant const_text(text);
  static_assert(std::is_same_v<decltype(nuostl::NuoGet<1>(std::move(text))),
                               std::string&&>);
  static_assert(std::is_same_v<decltype(nuostl::NuoGet<1>(std::move(const_text))),
                               const std::string&&>);
  assert(nuostl::NuoGetIf<1>(&const_text) != nullptr);
  assert(nuostl::NuoGetIf<int>(static_cast<Variant*>(nullptr)) == nullptr);
  assert(nuostl::NuoVisit([](const auto&) { return 1; }, const_text) == 1);
  assert(nuostl::NuoVisit([](const auto&, const auto&) { return 2; },
                        copy, const_text) == 2);
  nuostl::NuoVisit<void>([](auto& value) { (void)value; }, copy);

  nuostl::NuoVariant<int, int> repeated(std::in_place_index<1>, 9);
  assert(repeated.Index() == 1 && nuostl::NuoGet<1>(repeated) == 9);
  auto repeated_copy = repeated;
  assert(repeated == repeated_copy);
  repeated.Emplace<0>(3);
  assert(repeated < repeated_copy);

  nuostl::NuoVariant<int, std::unique_ptr<int>> owning(
    std::in_place_index<1>, std::make_unique<int>(11));
  auto moved = std::move(owning);
  assert(*nuostl::NuoGet<1>(moved) == 11);
  assert(owning.Index() == 1 && !nuostl::NuoGet<1>(owning));
  owning = 4;
  owning.Swap(moved);
  assert(*nuostl::NuoGet<1>(owning) == 11);
  assert(nuostl::NuoGet<0>(moved) == 4);
  auto* owning_alias = &owning;
  owning = std::move(*owning_alias);
  assert(*nuostl::NuoGet<1>(owning) == 11);

  nuostl::NuoVariant<int, std::vector<int>> sequence(
    std::in_place_type<std::vector<int>>, {1, 2, 3});
  assert(nuostl::NuoGet<1>(sequence).size() == 3);
  sequence.Emplace<1>({4, 5});
  assert(nuostl::NuoGet<1>(sequence).size() == 2);

  nuostl::NuoVariant<nuostl::NuoMonostate, int> empty;
  assert(empty.Index() == 0);
  assert(nuostl::NuoHoldsAlternative<nuostl::NuoMonostate>(empty));
  assert(nuostl::NuoGet<nuostl::NuoMonostate>(empty) == nuostl::NuoMonostate{});
  empty.Emplace<nuostl::NuoMonostate>();
  empty = nuostl::NuoMonostate{};
  empty = 9;
  assert(empty.Index() == 1);

  bool caught = false;
  try
  {
    nuostl::NuoGet<int>(number);
  }
  catch (const nuostl::NuoBadVariantAccess&)
  {
    caught = true;
  }
  assert(caught);

  {
    nuostl::NuoVariant<int, ThrowingValue> tracked(std::in_place_index<1>, 7);
    assert(ThrowingValue::live_count == 1);
    caught = false;
    try
    {
      tracked.Emplace<1>(-1);
    }
    catch (const std::runtime_error&)
    {
      caught = true;
    }
    assert(caught && tracked.ValuelessByException());
    assert(tracked.Index() == nuostl::NuoVariantNpos);
    assert(ThrowingValue::live_count == 0);
    assert(nuostl::NuoGetIf<1>(&tracked) == nullptr);
    auto valueless_copy = tracked;
    assert(valueless_copy.ValuelessByException());
    caught = false;
    try
    {
      nuostl::NuoVisit([](const auto&) {}, tracked);
    }
    catch (const nuostl::NuoBadVariantAccess&)
    {
      caught = true;
    }
    assert(caught);
    tracked.Swap(valueless_copy);
    assert(tracked.ValuelessByException() &&
           valueless_copy.ValuelessByException());
    tracked.Emplace<1>(8);
    assert(!tracked.ValuelessByException());
    valueless_copy = tracked;
    assert(ThrowingValue::live_count == 2);
    tracked = 3;
    tracked.Swap(valueless_copy);
    assert(tracked.Index() == 1 && valueless_copy.Index() == 0);
    tracked = 3;
    assert(ThrowingValue::live_count == 0);
  }
  assert(ThrowingValue::live_count == 0);

  {
    using Tracked = nuostl::NuoVariant<int, ThrowingValue>;
    Tracked source(std::in_place_index<1>, 4);
    Tracked target(2);
    ThrowingValue::fail_copy = true;
    caught = false;
    try
    {
      target = source;
    }
    catch (const std::runtime_error&)
    {
      caught = true;
    }
    assert(caught && target.ValuelessByException());
    assert(source.Index() == 1 && ThrowingValue::live_count == 1);
    ThrowingValue::fail_copy = false;
    target = source;
    assert(target.Index() == 1 && ThrowingValue::live_count == 2);
    auto* target_alias = &target;
    target = *target_alias;
    assert(ThrowingValue::live_count == 2);
  }
  assert(ThrowingValue::live_count == 0);

  /* Compare every index/value combination against the standard library. */
  for (int left_index = 0; left_index < 2; ++left_index)
  {
    for (int right_index = 0; right_index < 2; ++right_index)
    {
      for (int left_value = -2; left_value <= 2; ++left_value)
      {
        for (int right_value = -2; right_value <= 2; ++right_value)
        {
          nuostl::NuoVariant<int, long> left(left_value);
          nuostl::NuoVariant<int, long> right(right_value);
          std::variant<int, long> standard_left(left_value);
          std::variant<int, long> standard_right(right_value);
          if (left_index == 1)
          {
            left.Emplace<1>(left_value);
            standard_left.emplace<1>(left_value);
          }
          if (right_index == 1)
          {
            right.Emplace<1>(right_value);
            standard_right.emplace<1>(right_value);
          }
          assert((left == right) == (standard_left == standard_right));
          assert((left != right) == (standard_left != standard_right));
          assert((left < right) == (standard_left < standard_right));
          assert((left <= right) == (standard_left <= standard_right));
          assert((left > right) == (standard_left > standard_right));
          assert((left >= right) == (standard_left >= standard_right));
          assert((left <=> right) == (standard_left <=> standard_right));
        }
      }
    }
  }
}

} /* namespace test */
