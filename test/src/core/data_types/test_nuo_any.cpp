#include "core/data_types/test_nuo_any.hpp"

#include <assert.h>

#include <string>
#include <type_traits>

#include "core/data_types/nuo_any.hpp"
#include "utils/nuo_util.hpp"

namespace test
{

void TestNuoAny::test_nuo_any()
{
  nuostl::NuoAny empty;
  assert(!empty.HasValue());
  assert(empty.Type() == typeid(void));

  nuostl::NuoAny integer(42);
  assert(integer.HasValue());
  assert(integer.Type() == typeid(int));
  assert(nuostl::NuoAnyCast<int>(integer) == 42);
  assert(*nuostl::NuoAnyCast<int>(&integer) == 42);
  assert(nuostl::NuoAnyCast<double>(&integer) == nullptr);
  assert(nuostl::NuoAnyCast<double>(static_cast<const nuostl::NuoAny*>(&integer)) == nullptr);

  bool caught_bad_cast = false;
  try
  {
    nuostl::NuoAnyCast<double>(integer);
  }
  catch (const nuostl::NuoBadAnyCast&)
  {
    caught_bad_cast = true;
  }
  assert(caught_bad_cast);

  const nuostl::NuoAny const_integer(7);
  assert(nuostl::NuoAnyCast<const int&>(const_integer) == 7);
  static_assert(std::is_same<decltype(nuostl::NuoAnyCast<int&>(integer)), int&>::value,
                "any_cast should preserve mutable references");
  nuostl::NuoAnyCast<int&>(integer) = 9;
  assert(nuostl::NuoAnyCast<int>(integer) == 9);

  nuostl::NuoAny text(std::string("hello"));
  nuostl::NuoAny copied(text);
  assert(nuostl::NuoAnyCast<std::string>(copied) == "hello");
  nuostl::NuoAny moved(nuostl::NuoMove(copied));
  assert(nuostl::NuoAnyCast<std::string>(moved) == "hello");
  assert(!copied.HasValue());

  nuostl::NuoAny assigned;
  assigned = text;
  assert(nuostl::NuoAnyCast<std::string>(assigned) == "hello");
  assigned = nuostl::NuoMove(moved);
  assert(nuostl::NuoAnyCast<std::string>(assigned) == "hello");
  assigned = 11;
  assert(nuostl::NuoAnyCast<int>(assigned) == 11);

  nuostl::NuoAny constructed(std::in_place_type<std::string>, "value");
  assert(nuostl::NuoAnyCast<std::string>(constructed) == "value");
  constructed.Emplace<int>(23);
  assert(nuostl::NuoAnyCast<int>(constructed) == 23);
  constructed.Reset();
  assert(!constructed.HasValue());

  nuostl::NuoAny first(1);
  nuostl::NuoAny second(std::string("second"));
  first.Swap(second);
  assert(nuostl::NuoAnyCast<std::string>(first) == "second");
  assert(nuostl::NuoAnyCast<int>(second) == 1);

  nuostl::NuoAny empty_copy(empty);
  assert(!empty_copy.HasValue());
  nuostl::NuoAny empty_move(nuostl::NuoMove(empty_copy));
  assert(!empty_move.HasValue());
}

} /* namespace test */
