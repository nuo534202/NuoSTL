#include "core/data_types/test_nuo_optional.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/data_types/nuo_optional.hpp"

namespace test
{

namespace
{

struct ThrowOnConstruction
{
  explicit ThrowOnConstruction(bool should_throw)
  {
    if (should_throw)
    {
      throw std::runtime_error("construction failed");
    }
  }
};

} /* namespace */

constexpr nuostl::NuoOptional<int> kConstexprOptional(23);
static_assert(kConstexprOptional.HasValue());
static_assert(kConstexprOptional.Value() == 23);

void TestNuoOptional::test_nuo_optional()
{
  nuostl::NuoOptional<int> empty;
  assert(!empty.HasValue());
  assert(!empty);
  assert(empty == nuostl::NuoNullopt);
  assert(empty.ValueOr(17) == 17);

  bool caught_bad_access = false;
  try
  {
    empty.Value();
  }
  catch (const nuostl::NuoBadOptionalAccess&)
  {
    caught_bad_access = true;
  }
  assert(caught_bad_access);

  nuostl::NuoOptional<int> value(42);
  assert(value.HasValue());
  assert(value.Value() == 42);
  assert(*value == 42);
  assert(value.ValueOr(0) == 42);
  *value = 9;
  assert(value.Value() == 9);
  assert(value.operator->() != nullptr);
  assert(value.Value() == *value.operator->());

  nuostl::NuoOptional<std::string> text(nuostl::NuoInPlace, 3, 'x');
  assert(text.Value() == "xxx");
  text.Emplace("updated");
  assert(text.Value() == "updated");
  text.Reset();
  assert(!text.HasValue());

  nuostl::NuoOptional<int> copied(value);
  assert(copied == value);
  nuostl::NuoOptional<int> assigned;
  assigned = copied;
  assert(assigned.Value() == 9);
  assigned = nuostl::NuoNullopt;
  assert(!assigned);
  assigned = 12;
  assert(assigned.Value() == 12);
  nuostl::NuoOptional<int> assigned_from_empty(99);
  assigned_from_empty = empty;
  assert(!assigned_from_empty);
  assigned_from_empty = copied;
  assert(assigned_from_empty.Value() == copied.Value());

  nuostl::NuoOptional<int> moved(nuostl::NuoMove(assigned));
  assert(moved.Value() == 12);
  assert(assigned.HasValue());
  nuostl::NuoOptional<int> move_assigned;
  move_assigned = nuostl::NuoMove(moved);
  assert(move_assigned.Value() == 12);
  assert(moved.HasValue());

  nuostl::NuoOptional<ThrowOnConstruction> throwing;
  bool caught_construction_error = false;
  try
  {
    throwing.Emplace(true);
  }
  catch (const std::runtime_error&)
  {
    caught_construction_error = true;
  }
  assert(caught_construction_error);
  assert(!throwing.HasValue());

  nuostl::NuoOptional<int> one(1);
  nuostl::NuoOptional<int> two(2);
  one.Swap(two);
  assert(one.Value() == 2);
  assert(two.Value() == 1);
  one.Swap(empty);
  assert(!one);
  assert(empty.Value() == 2);

  nuostl::NuoOptional<int> lower(3);
  nuostl::NuoOptional<int> higher(5);
  assert(lower < higher);
  assert(lower <= higher);
  assert(higher > lower);
  assert(higher >= lower);
  assert(lower != higher);
  assert(nuostl::NuoOptional<int>() < lower);
  assert(lower > nuostl::NuoNullopt);
  assert(lower == 3);
  assert(3 == lower);
  assert(lower < 4);
  assert(2 < lower);
  assert(lower <= 3);
  assert(3 <= lower);
  assert(higher > 4);
  assert(6 > higher);
  assert(higher >= 5);
  assert(5 >= higher);

  static_assert(std::is_same<decltype(*value), int&>::value,
                "mutable optional access should return a reference");
  static_assert(std::is_same<decltype(*static_cast<const nuostl::NuoOptional<int>&>(value)),
                             const int&>::value,
                "const optional access should return a const reference");
}

} /* namespace test */
