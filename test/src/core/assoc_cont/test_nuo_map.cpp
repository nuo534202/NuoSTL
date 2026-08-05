#include "core/assoc_cont/test_nuo_map.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "nuostl.hpp"

namespace test
{
namespace
{

struct ReverseInt
{
  bool operator()(int lhs, int rhs) const
  {
    return lhs > rhs;
  }
};

class MoveOnly
{
public:
  explicit MoveOnly(int value = 0)
    : value_(value)
  {
  }

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  MoveOnly(MoveOnly&& rhs) noexcept
    : value_(rhs.value_)
  {
    rhs.value_ = 0;
  }

  MoveOnly& operator=(MoveOnly&& rhs) noexcept
  {
    value_ = rhs.value_;
    rhs.value_ = 0;
    return *this;
  }

  int Value() const
  {
    return value_;
  }

private:
  int value_;
};

} /* namespace */

void TestNuoMap::test_constructor_and_assignment()
{
  using Map = nuostl::NuoMap<int, std::string>;
  using Value = Map::value_type;

  Map empty;
  assert(empty.Empty());

  Value values[] = {Value(3, "three"), Value(1, "one"), Value(2, "two"),
                    Value(2, "duplicate")};
  Map range(values, values + 4);
  assert(range.Size() == 3);
  assert(range.Begin()->first == 1);
  assert(range.At(2) == "two");

  Map initialized{Value(4, "four"), Value(2, "two"), Value(4, "ignored")};
  assert(initialized.Size() == 2);
  assert(initialized.At(4) == "four");

  Map copied(initialized);
  assert(copied == initialized);

  Map moved(nuostl::NuoMove(copied));
  assert(moved == initialized);
  assert(copied.Empty());

  Map copy_assigned;
  copy_assigned = range;
  assert(copy_assigned == range);

  Map move_assigned;
  move_assigned = nuostl::NuoMove(copy_assigned);
  assert(move_assigned == range);
  assert(copy_assigned.Empty());

  move_assigned = {Value(8, "eight"), Value(7, "seven"), Value(8, "ignored")};
  assert(move_assigned.Size() == 2);
  assert(move_assigned.Begin()->first == 7);

  nuostl::NuoMap<int, int, ReverseInt> reversed{ReverseInt()};
  reversed.Insert(typename decltype(reversed)::value_type(1, 10));
  reversed.Insert(typename decltype(reversed)::value_type(3, 30));
  reversed.Insert(typename decltype(reversed)::value_type(2, 20));
  assert(reversed.Begin()->first == 3);
}

void TestNuoMap::test_iterators_and_capacity()
{
  using Map = nuostl::NuoMap<int, std::string>;
  using Value = Map::value_type;
  Map map{Value(3, "three"), Value(1, "one"), Value(2, "two")};

  static_assert(std::is_const_v<std::remove_reference_t<decltype(map.Begin()->first)>>);
  assert(!map.Empty());
  assert(map.Size() == 3);
  assert(map.MaxSize() >= map.Size());

  int expected = 1;
  for (auto iterator = map.Begin(); iterator != map.End(); ++iterator)
  {
    assert(iterator->first == expected);
    iterator->second += "!";
    ++expected;
  }
  assert(map.At(2) == "two!");

  expected = 3;
  for (auto iterator = map.RBegin(); iterator != map.REnd(); ++iterator)
  {
    assert(iterator->first == expected);
    --expected;
  }

  const Map& const_map = map;
  assert(const_map.CBegin()->first == 1);
  assert(const_map.CRBegin()->first == 3);
  assert(const_map.CEnd() == const_map.End());
  assert(const_map.CREnd() == const_map.REnd());
}

void TestNuoMap::test_element_access()
{
  nuostl::NuoMap<int, std::string> map;

  map[2] = "two";
  assert(map.Size() == 1);
  assert(map[2] == "two");

  int key = 1;
  map[nuostl::NuoMove(key)] = "one";
  assert(map.At(1) == "one");

  map.At(2) = "updated";
  assert(map.At(2) == "updated");

  const auto& const_map = map;
  assert(const_map.At(1) == "one");

  bool mutable_threw = false;
  try
  {
    map.At(99);
  }
  catch (const std::out_of_range&)
  {
    mutable_threw = true;
  }
  assert(mutable_threw);

  bool const_threw = false;
  try
  {
    const_map.At(99);
  }
  catch (const std::out_of_range&)
  {
    const_threw = true;
  }
  assert(const_threw);
}

void TestNuoMap::test_insert_and_emplace()
{
  using Map = nuostl::NuoMap<int, std::string>;
  using Value = Map::value_type;
  Map map;

  Value one(1, "one");
  auto first = map.Insert(one);
  assert(first.second);
  assert(first.first->second == "one");

  auto duplicate = map.Insert(Value(1, "ignored"));
  assert(!duplicate.second);
  assert(duplicate.first->second == "one");

  auto emplaced = map.Emplace(2, std::string("two"));
  assert(emplaced.second);
  auto hinted = map.EmplaceHint(map.End(), 4, std::string("four"));
  assert(hinted->first == 4);

  auto inserted_hint = map.Insert(map.Find(4), Value(3, "three"));
  assert(inserted_hint->first == 3);

  Value range[] = {Value(5, "five"), Value(6, "six"), Value(5, "ignored")};
  map.Insert(range, range + 3);
  map.Insert({Value(7, "seven"), Value(7, "ignored"), Value(8, "eight")});
  assert(map.Size() == 8);

  auto try_inserted = map.TryEmplace(9, "nine");
  assert(try_inserted.second);
  auto try_duplicate = map.TryEmplace(9, "ignored");
  assert(!try_duplicate.second);
  assert(try_duplicate.first->second == "nine");

  auto assigned = map.InsertOrAssign(9, std::string("updated"));
  assert(!assigned.second);
  assert(assigned.first->second == "updated");
  auto newly_assigned = map.InsertOrAssign(10, std::string("ten"));
  assert(newly_assigned.second);
  assert(map.Size() == 10);

  nuostl::NuoMap<int, MoveOnly> move_only;
  auto move_emplaced = move_only.TryEmplace(1, 10);
  assert(move_emplaced.second);
  assert(move_emplaced.first->second.Value() == 10);
  auto move_assigned = move_only.InsertOrAssign(1, MoveOnly(11));
  assert(!move_assigned.second);
  assert(move_assigned.first->second.Value() == 11);
  using MoveValue = typename decltype(move_only)::value_type;
  auto move_inserted = move_only.Insert(MoveValue(2, MoveOnly(20)));
  assert(move_inserted.second);
  assert(move_inserted.first->second.Value() == 20);
}

void TestNuoMap::test_erase()
{
  using Map = nuostl::NuoMap<int, int>;
  using Value = Map::value_type;
  Map map{Value(1, 10), Value(2, 20), Value(3, 30), Value(4, 40), Value(5, 50)};

  auto next = map.Erase(map.Find(1));
  assert(next->first == 2);
  assert(map.Erase(1) == 0);
  assert(map.Erase(3) == 1);

  auto range_end = map.Erase(map.Find(2), map.Find(5));
  assert(range_end->first == 5);
  assert(map.Size() == 1);

  map.Clear();
  assert(map.Empty());
}

void TestNuoMap::test_lookup_and_observers()
{
  using Map = nuostl::NuoMap<int, int>;
  using Value = Map::value_type;
  Map map{Value(2, 20), Value(4, 40), Value(6, 60)};

  assert(map.Find(4) != map.End());
  assert(map.Find(5) == map.End());
  assert(map.Count(4) == 1);
  assert(map.Count(5) == 0);
  assert(map.Contains(6));
  assert(!map.Contains(7));

  assert(map.LowerBound(3)->first == 4);
  assert(map.LowerBound(4)->first == 4);
  assert(map.UpperBound(4)->first == 6);
  assert(map.UpperBound(6) == map.End());

  auto found = map.EqualRange(4);
  assert(found.first->first == 4);
  assert(found.second->first == 6);
  auto missing = map.EqualRange(5);
  assert(missing.first == missing.second);
  assert(missing.first->first == 6);

  auto key_compare = map.KeyComp();
  assert(key_compare(1, 2));
  auto value_compare = map.ValueComp();
  assert(value_compare(Value(1, 100), Value(2, 0)));

  const Map& const_map = map;
  assert(const_map.Find(2)->second == 20);
  assert(const_map.LowerBound(1)->first == 2);
  assert(const_map.UpperBound(4)->first == 6);
  assert(const_map.EqualRange(5).first->first == 6);
}

void TestNuoMap::test_swap_and_comparison()
{
  using Map = nuostl::NuoMap<int, int>;
  using Value = Map::value_type;
  Map first{Value(1, 10), Value(2, 20)};
  Map same{Value(1, 10), Value(2, 20)};
  Map greater{Value(1, 10), Value(3, 30)};
  Map other{Value(9, 90)};

  assert(first == same);
  assert(first != greater);
  assert(first < greater);
  assert(greater > first);
  assert(first <= same);
  assert(first >= same);

  first.Swap(other);
  assert(first.Begin()->first == 9);
  assert(other.Size() == 2);
  nuostl::swap(first, other);
  assert(first.Size() == 2);
  assert(other.Begin()->first == 9);
}

void TestNuoMap::test_merge()
{
  using Map = nuostl::NuoMap<int, int>;
  using Multimap = nuostl::NuoMultimap<int, int>;
  using MapValue = Map::value_type;
  using MultiValue = Multimap::value_type;

  Map target{MapValue(1, 10), MapValue(3, 30)};
  Map source{MapValue(2, 20), MapValue(3, 300), MapValue(4, 40)};
  target.Merge(source);
  assert(target.Size() == 4);
  assert(source.Size() == 1);
  assert(source.Begin()->first == 3);
  assert(target.At(3) == 30);

  Multimap multi_source{MultiValue(5, 50), MultiValue(5, 500), MultiValue(4, 400)};
  target.Merge(multi_source);
  assert(target.Count(5) == 1);
  assert(multi_source.Count(5) == 1);
  assert(multi_source.Count(4) == 1);
}

void TestNuoMap::test_nuo_map()
{
  test_constructor_and_assignment();
  test_iterators_and_capacity();
  test_element_access();
  test_insert_and_emplace();
  test_erase();
  test_lookup_and_observers();
  test_swap_and_comparison();
  test_merge();
}

void TestNuoMultimap::test_constructor_and_assignment()
{
  using Multimap = nuostl::NuoMultimap<int, std::string>;
  using Value = Multimap::value_type;

  Multimap empty;
  assert(empty.Empty());

  Value values[] = {Value(2, "a"), Value(1, "one"), Value(2, "b")};
  Multimap range(values, values + 3);
  assert(range.Size() == 3);
  assert(range.Count(2) == 2);

  Multimap initialized{Value(3, "a"), Value(3, "b"), Value(1, "one")};
  assert(initialized.Size() == 3);

  Multimap copied(initialized);
  assert(copied == initialized);
  Multimap moved(nuostl::NuoMove(copied));
  assert(moved == initialized);
  assert(copied.Empty());

  Multimap assigned;
  assigned = range;
  assert(assigned == range);
  Multimap move_assigned;
  move_assigned = nuostl::NuoMove(assigned);
  assert(move_assigned == range);
  assert(assigned.Empty());

  move_assigned = {Value(8, "a"), Value(8, "b")};
  assert(move_assigned.Count(8) == 2);

  nuostl::NuoMultimap<int, int, ReverseInt> reversed{ReverseInt()};
  reversed.Insert(typename decltype(reversed)::value_type(1, 10));
  reversed.Insert(typename decltype(reversed)::value_type(3, 30));
  assert(reversed.Begin()->first == 3);
}

void TestNuoMultimap::test_iterators_and_capacity()
{
  using Multimap = nuostl::NuoMultimap<int, int>;
  using Value = Multimap::value_type;
  Multimap map{Value(2, 20), Value(1, 10), Value(2, 21)};

  assert(!map.Empty());
  assert(map.Size() == 3);
  assert(map.MaxSize() >= map.Size());
  assert(map.Begin()->first == 1);
  assert(map.RBegin()->first == 2);
  map.Begin()->second = 11;
  assert(map.Begin()->second == 11);

  const Multimap& const_map = map;
  assert(const_map.CBegin()->first == 1);
  assert(const_map.CRBegin()->first == 2);
}

void TestNuoMultimap::test_insert_and_emplace()
{
  using Multimap = nuostl::NuoMultimap<int, std::string>;
  using Value = Multimap::value_type;
  Multimap map;

  Value first(1, "a");
  map.Insert(first);
  map.Insert(Value(1, "b"));
  map.Emplace(1, std::string("c"));
  map.EmplaceHint(map.End(), 2, std::string("two"));
  map.Insert(map.End(), Value(3, "three"));

  Value range[] = {Value(2, "other"), Value(4, "a"), Value(4, "b")};
  map.Insert(range, range + 3);
  map.Insert({Value(5, "a"), Value(5, "b")});

  assert(map.Count(1) == 3);
  assert(map.Count(2) == 2);
  assert(map.Count(4) == 2);
  assert(map.Count(5) == 2);
  assert(map.Size() == 10);
}

void TestNuoMultimap::test_erase()
{
  using Multimap = nuostl::NuoMultimap<int, int>;
  using Value = Multimap::value_type;
  Multimap map{Value(1, 10), Value(1, 11), Value(2, 20), Value(3, 30),
               Value(4, 40)};

  auto next = map.Erase(map.Begin());
  assert(next->first == 1);
  assert(map.Count(1) == 1);
  assert(map.Erase(1) == 1);

  auto range_end = map.Erase(map.LowerBound(2), map.LowerBound(4));
  assert(range_end->first == 4);
  assert(map.Size() == 1);
  map.Clear();
  assert(map.Empty());
}

void TestNuoMultimap::test_lookup_and_observers()
{
  using Multimap = nuostl::NuoMultimap<int, int>;
  using Value = Multimap::value_type;
  Multimap map{Value(2, 20), Value(2, 21), Value(4, 40), Value(6, 60)};

  assert(map.Find(2) != map.End());
  assert(map.Count(2) == 2);
  assert(map.Count(3) == 0);
  assert(map.Contains(4));
  assert(!map.Contains(5));
  assert(map.LowerBound(3)->first == 4);
  assert(map.UpperBound(2)->first == 4);

  auto range = map.EqualRange(2);
  int count = 0;
  for (auto iterator = range.first; iterator != range.second; ++iterator)
  {
    assert(iterator->first == 2);
    ++count;
  }
  assert(count == 2);

  auto missing = map.EqualRange(3);
  assert(missing.first == missing.second);
  assert(missing.first->first == 4);

  assert(map.KeyComp()(1, 2));
  assert(map.ValueComp()(Value(1, 0), Value(2, 0)));

  const Multimap& const_map = map;
  assert(const_map.Find(6)->second == 60);
  assert(const_map.EqualRange(3).first->first == 4);
}

void TestNuoMultimap::test_swap_and_comparison()
{
  using Multimap = nuostl::NuoMultimap<int, int>;
  using Value = Multimap::value_type;
  Multimap first{Value(1, 10), Value(1, 11)};
  Multimap same{Value(1, 10), Value(1, 11)};
  Multimap greater{Value(1, 10), Value(2, 20)};
  Multimap other{Value(9, 90)};

  assert(first == same);
  assert(first != greater);
  assert(first < greater);
  assert(greater > first);
  assert(first <= same);
  assert(first >= same);

  first.Swap(other);
  assert(first.Begin()->first == 9);
  nuostl::swap(first, other);
  assert(first.Count(1) == 2);
}

void TestNuoMultimap::test_merge()
{
  using Map = nuostl::NuoMap<int, int>;
  using Multimap = nuostl::NuoMultimap<int, int>;
  using MapValue = Map::value_type;
  using MultiValue = Multimap::value_type;

  Multimap target{MultiValue(1, 10)};
  Multimap source{MultiValue(1, 11), MultiValue(2, 20)};
  target.Merge(source);
  assert(target.Size() == 3);
  assert(source.Empty());
  assert(target.Count(1) == 2);

  Map map_source{MapValue(2, 200), MapValue(3, 30)};
  target.Merge(map_source);
  assert(map_source.Empty());
  assert(target.Count(2) == 2);
  assert(target.Count(3) == 1);
}

void TestNuoMultimap::test_nuo_multimap()
{
  test_constructor_and_assignment();
  test_iterators_and_capacity();
  test_insert_and_emplace();
  test_erase();
  test_lookup_and_observers();
  test_swap_and_comparison();
  test_merge();
}

} /* namespace test */
