#include "core/assoc_cont/test_nuo_unordered_map.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>

#include "nuostl.hpp"

namespace
{
/* Custom hash for std::string because NuoHash has no std::string specialization. */
class StringHash
{
public:
  size_t operator()(const std::string& str) const
  {
    size_t result = 0;
    for (size_t i = 0; i < str.size(); ++i)
      result = result * 131 + static_cast<unsigned char>(str[i]);
    return result;
  }
};

/* Custom hash that collapses keys into a small range to force collisions. */
class ModuloHash
{
public:
  size_t operator()(int val) const
  {
    return static_cast<size_t>((val % 10 + 10) % 10);
  }
};

/* Move-only type used to verify move-based insertion. */
class MoveOnly
{
public:
  explicit MoveOnly(int value)
    : value_(value)
  {
  }

  MoveOnly(MoveOnly&& other) noexcept
    : value_(other.value_)
  {
    other.value_ = 0;
  }

  MoveOnly& operator=(MoveOnly&& other) noexcept
  {
    if (this != &other)
    {
      value_ = other.value_;
      other.value_ = 0;
    }
    return *this;
  }

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  int Value() const
  {
    return value_;
  }

private:
  int value_;
};
} /* namespace */

namespace test
{
/* ------------------------------------------------------------- */
/* TestNuoUnorderedMap                                            */
/* ------------------------------------------------------------- */

void TestNuoUnorderedMap::test_constructor_and_assignment()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  /* Default constructor. */
  Map map;
  assert(map.Empty());
  assert(map.Size() == 0);
  assert(map.BucketCount() != 0);

  /* Constructor with an explicit bucket count. */
  Map bucket_map(32);
  assert(bucket_map.Empty());
  assert(bucket_map.BucketCount() >= 32);

  /* Range constructor. */
  Map base;
  base.Emplace(1, "one");
  base.Emplace(2, "two");
  base.Emplace(3, "three");
  Map range_map(base.Begin(), base.End());
  assert(range_map.Size() == 3);
  assert(range_map.At(1) == "one");
  assert(range_map.At(2) == "two");
  assert(range_map.At(3) == "three");

  /* Initializer list constructor. */
  Map list_map = {{1, "one"}, {2, "two"}, {3, "three"}};
  assert(list_map.Size() == 3);
  assert(list_map.At(1) == "one");
  assert(list_map.At(3) == "three");

  /* Copy constructor. */
  Map copy_map(list_map);
  assert(copy_map.Size() == 3);
  assert(copy_map.At(2) == "two");

  /* Move constructor. */
  Map move_map(nuostl::NuoMove(copy_map));
  assert(move_map.Size() == 3);
  assert(move_map.At(1) == "one");
  assert(copy_map.Empty());

  /* Copy assignment. */
  Map assigned;
  assigned = map;
  assert(assigned.Empty());
  assigned = list_map;
  assert(assigned.Size() == 3);
  assert(assigned.At(3) == "three");

  /* Move assignment. */
  Map move_assigned;
  move_assigned = nuostl::NuoMove(assigned);
  assert(move_assigned.Size() == 3);
  assert(move_assigned.At(2) == "two");
  assert(assigned.Empty());

  /* Initializer list assignment. */
  move_assigned = {{10, "ten"}, {20, "twenty"}};
  assert(move_assigned.Size() == 2);
  assert(move_assigned.At(10) == "ten");
  assert(move_assigned.At(20) == "twenty");
}

void TestNuoUnorderedMap::test_iterators_and_capacity()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  Map map;
  map.Emplace(1, "one");
  map.Emplace(2, "two");
  map.Emplace(3, "three");

  /* Begin and End visit every element exactly once. */
  size_t count = 0;
  bool seen1 = false;
  bool seen2 = false;
  bool seen3 = false;
  for (Map::iterator it = map.Begin(); it != map.End(); ++it)
  {
    ++count;
    if (it->first == 1)
      seen1 = true;
    if (it->first == 2 && it->second == "two")
      seen2 = true;
    if (it->first == 3)
      seen3 = true;
  }
  assert(count == 3);
  assert(seen1 && seen2 && seen3);

  /* Const iterators over a const map. */
  const Map& ref = map;
  count = 0;
  for (Map::const_iterator it = ref.CBegin(); it != ref.CEnd(); ++it)
  {
    ++count;
    assert(it->second.size() != 0);
  }
  assert(count == 3);

  /* An empty map has Begin() == End(). */
  Map empty_map;
  assert(empty_map.Begin() == empty_map.End());

  /* Capacity operations. */
  assert(!map.Empty());
  assert(map.Size() == 3);
  assert(map.MaxSize() != 0);
}

void TestNuoUnorderedMap::test_element_access()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  Map map;

  /* operator[] inserts a default value when the key is absent. */
  assert(map[1] == "");
  assert(map.Size() == 1);
  map[1] = "one";
  assert(map[1] == "one");
  assert(map.Size() == 1);

  /* operator[] with an rvalue key. */
  int key = 2;
  map[nuostl::NuoMove(key)] = "two";
  assert(map.At(2) == "two");
  assert(map.Size() == 2);

  /* At returns the mapped value. */
  assert(map.At(1) == "one");
  const Map& ref = map;
  assert(ref.At(2) == "two");

  /* At throws std::out_of_range when the key is absent. */
  bool thrown = false;
  try
  {
    map.At(999);
  }
  catch (const std::out_of_range&)
  {
    thrown = true;
  }
  assert(thrown);
}

void TestNuoUnorderedMap::test_insert_and_emplace()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  Map map;

  /* Emplace inserts a new element and reports success. */
  nuostl::nuo_pair<Map::iterator, bool> res = map.Emplace(1, "one");
  assert(res.second);
  assert(res.first->first == 1);
  assert(res.first->second == "one");
  assert(map.Size() == 1);

  /* Emplace fails for an existing key. */
  res = map.Emplace(1, "ONE");
  assert(!res.second);
  assert(res.first->second == "one");
  assert(map.Size() == 1);

  /* Insert copies a value. */
  Map::value_type value(2, "two");
  res = map.Insert(value);
  assert(res.second);
  assert(map.At(2) == "two");

  /* Insert moves an rvalue value. */
  res = map.Insert(Map::value_type(3, "three"));
  assert(res.second);
  assert(map.At(3) == "three");

  /* Insert with a hint. */
  Map::iterator it = map.Insert(map.CBegin(), Map::value_type(4, "four"));
  assert(it->first == 4);
  assert(map.Size() == 4);

  /* TryEmplace does not consume arguments when the key exists. */
  std::string arg("new");
  res = map.TryEmplace(1, arg);
  assert(!res.second);
  assert(arg == "new");
  assert(map.At(1) == "one");

  /* TryEmplace constructs the value when the key is absent. */
  res = map.TryEmplace(5, arg);
  assert(res.second);
  assert(map.At(5) == "new");

  /* InsertOrAssign assigns when the key exists. */
  res = map.InsertOrAssign(5, "FIVE");
  assert(!res.second);
  assert(map.At(5) == "FIVE");

  /* InsertOrAssign inserts when the key is absent. */
  res = map.InsertOrAssign(6, "six");
  assert(res.second);
  assert(map.At(6) == "six");
}

void TestNuoUnorderedMap::test_erase_and_clear()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  Map map;
  for (int i = 0; i < 10; ++i)
    map.Emplace(i, "");

  /* Erase by key returns the number of erased elements. */
  assert(map.Erase(3) == 1);
  assert(map.Size() == 9);
  assert(map.Find(3) == map.End());
  assert(map.Erase(999) == 0);

  /* Erase by iterator returns the iterator to the next element. */
  Map::iterator it = map.Find(5);
  Map::iterator next = map.Erase(it);
  assert(map.Size() == 8);
  assert(map.Find(5) == map.End());
  assert(next != map.End());
  assert(next->first != 5);

  /* Erasing the last remaining element returns End(). */
  Map only_map;
  only_map.Emplace(1, "one");
  Map::iterator only = only_map.Begin();
  assert(only_map.Erase(only) == only_map.End());
  assert(only_map.Empty());

  /* Clear removes all elements. */
  map.Clear();
  assert(map.Empty());
  assert(map.Size() == 0);
  assert(map.Begin() == map.End());
}

void TestNuoUnorderedMap::test_lookup()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  Map map;
  map.Emplace(1, "one");
  map.Emplace(2, "two");
  map.Emplace(3, "three");

  /* Find returns the matching element. */
  Map::iterator it = map.Find(2);
  assert(it != map.End());
  assert(it->second == "two");
  assert(map.Find(999) == map.End());

  /* Find on a const map. */
  const Map& ref = map;
  Map::const_iterator cit = ref.Find(1);
  assert(cit != ref.CEnd());
  assert(cit->second == "one");
  assert(ref.Find(999) == ref.CEnd());

  /* Count returns 0 or 1 because keys are unique. */
  assert(map.Count(1) == 1);
  assert(map.Count(999) == 0);

  /* Contains. */
  assert(map.Contains(3));
  assert(!map.Contains(999));

  /* EqualRange covers exactly one element for an existing key. */
  nuostl::nuo_pair<Map::iterator, Map::iterator> range = map.EqualRange(1);
  assert(range.first != map.End());
  assert(range.first->first == 1);
  Map::iterator next = range.first;
  ++next;
  assert(next == range.second);

  /* EqualRange is empty for an absent key. */
  range = map.EqualRange(999);
  assert(range.first == map.End());
  assert(range.second == map.End());
}

void TestNuoUnorderedMap::test_bucket_and_hash_policy()
{
  typedef nuostl::NuoUnorderedMap<int, std::string, ModuloHash> Map;

  Map map;
  assert(map.MaxLoadFactor() == 1.0f);
  assert(map.LoadFactor() == 0.0f);

  /* Keys that hash identically land in the same bucket. */
  map.Emplace(1, "one");
  map.Emplace(11, "eleven");
  map.Emplace(21, "twenty-one");
  assert(map.Bucket(1) == map.Bucket(11));
  assert(map.Bucket(11) == map.Bucket(21));
  assert(map.BucketSize(map.Bucket(1)) == 3);
  assert(map.Size() == 3);
  assert(map.BucketCount() != 0);
  assert(map.LoadFactor() > 0.0f);

  /* Local iterators traverse one bucket. */
  size_t index = map.Bucket(1);
  size_t count = 0;
  for (Map::local_iterator it = map.Begin(index); it != map.End(index); ++it)
  {
    ++count;
    assert(it->first % 10 == 1);
  }
  assert(count == 3);

  /* Const local iterators. */
  const Map& ref = map;
  count = 0;
  for (Map::const_local_iterator it = ref.CBegin(index);
       it != ref.CEnd(index); ++it)
  {
    ++count;
    assert(it->first % 10 == 1);
  }
  assert(count == 3);

  /* Rehash increases the bucket count and preserves the elements. */
  size_t old_count = map.BucketCount();
  map.Rehash(2 * old_count);
  assert(map.BucketCount() >= 2 * old_count);
  assert(map.Size() == 3);
  assert(map.Bucket(1) == map.Bucket(11));
  assert(map.BucketSize(map.Bucket(1)) == 3);

  /* Reserve guarantees room for n elements. */
  Map reserved;
  reserved.Reserve(100);
  assert(reserved.Empty());
  assert(reserved.BucketCount() >= 100);

  /* MaxLoadFactor can be changed. */
  Map lf_map;
  lf_map.MaxLoadFactor(0.5f);
  assert(lf_map.MaxLoadFactor() == 0.5f);
  for (int i = 0; i < 20; ++i)
    lf_map.Emplace(i, "v");
  assert(lf_map.Size() == 20);
  assert(lf_map.LoadFactor() <= 0.5f);
  assert(lf_map.MaxBucketCount() != 0);
}

void TestNuoUnorderedMap::test_swap_and_merge()
{
  typedef nuostl::NuoUnorderedMap<int, std::string> Map;

  /* Swap exchanges the whole state. */
  Map a;
  a.Emplace(1, "one");
  a.Emplace(2, "two");
  Map b;
  b.Emplace(3, "three");
  a.Swap(b);
  assert(a.Size() == 1);
  assert(b.Size() == 2);
  assert(a.At(3) == "three");
  assert(b.At(1) == "one");
  assert(b.At(2) == "two");

  /* Merge transfers only the elements whose keys are absent. */
  Map target;
  target.Emplace(1, "ONE");
  target.Emplace(4, "four");
  Map source;
  source.Emplace(1, "one");
  source.Emplace(2, "two");
  source.Emplace(5, "five");
  target.Merge(source);
  assert(target.Size() == 4);
  assert(target.At(1) == "ONE");
  assert(target.At(2) == "two");
  assert(target.At(4) == "four");
  assert(target.At(5) == "five");
  assert(source.Size() == 1);
  assert(source.At(1) == "one");
}

void TestNuoUnorderedMap::test_custom_hash_and_move_only()
{
  /* A map with std::string keys needs a custom hash. */
  typedef nuostl::NuoUnorderedMap<std::string, int, StringHash> StringMap;
  StringMap smap;
  smap.Emplace("apple", 1);
  smap.Emplace("banana", 2);
  smap.Emplace("cherry", 3);
  assert(smap.Size() == 3);
  assert(smap.At("banana") == 2);
  assert(smap.Contains("cherry"));
  assert(!smap.Contains("durian"));
  assert(smap.Count("apple") == 1);

  /* A map with a move-only mapped type. */
  typedef nuostl::NuoUnorderedMap<int, MoveOnly> MoveMap;
  MoveMap mmap;
  mmap.Emplace(1, MoveOnly(10));
  mmap.Emplace(2, MoveOnly(20));
  assert(mmap.Size() == 2);
  assert(mmap.At(1).Value() == 10);
  assert(mmap.At(2).Value() == 20);

  /* Emplacing a duplicate key destroys the temporary node. */
  mmap.Emplace(1, MoveOnly(99));
  assert(mmap.Size() == 2);
  assert(mmap.At(1).Value() == 10);

  /* InsertOrAssign moves the value into the existing element. */
  mmap.InsertOrAssign(2, MoveOnly(200));
  assert(mmap.Size() == 2);
  assert(mmap.At(2).Value() == 200);
}

void TestNuoUnorderedMap::test_nuo_unordered_map()
{
  test_constructor_and_assignment();
  test_iterators_and_capacity();
  test_element_access();
  test_insert_and_emplace();
  test_erase_and_clear();
  test_lookup();
  test_bucket_and_hash_policy();
  test_swap_and_merge();
  test_custom_hash_and_move_only();
}

/* ------------------------------------------------------------- */
/* TestNuoUnorderedMultimap                                       */
/* ------------------------------------------------------------- */

void TestNuoUnorderedMultimap::test_constructor_and_assignment()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  /* Default constructor. */
  Map map;
  assert(map.Empty());
  assert(map.Size() == 0);
  assert(map.BucketCount() != 0);

  /* Duplicate keys are allowed. */
  map.Emplace(1, "one");
  map.Emplace(1, "uno");
  map.Emplace(2, "two");
  assert(map.Size() == 3);
  assert(map.Count(1) == 2);

  /* Copy constructor. */
  Map copy_map(map);
  assert(copy_map.Size() == 3);
  assert(copy_map.Count(1) == 2);

  /* Move constructor. */
  Map move_map(nuostl::NuoMove(copy_map));
  assert(move_map.Size() == 3);
  assert(move_map.Count(1) == 2);
  assert(copy_map.Empty());

  /* Copy and move assignment. */
  Map assigned;
  assigned = map;
  assert(assigned.Size() == 3);
  assigned = nuostl::NuoMove(move_map);
  assert(assigned.Size() == 3);
  assert(move_map.Empty());

  /* Initializer list constructor. */
  Map list_map = {{1, "one"}, {1, "uno"}, {2, "two"}};
  assert(list_map.Size() == 3);
  assert(list_map.Count(1) == 2);
}

void TestNuoUnorderedMultimap::test_iterators_and_capacity()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  Map map;
  map.Emplace(1, "one");
  map.Emplace(1, "uno");
  map.Emplace(2, "two");
  map.Emplace(3, "three");

  /* Begin and End visit every element exactly once. */
  size_t count = 0;
  bool seen1 = false;
  bool seen2 = false;
  bool seen3 = false;
  for (Map::iterator it = map.Begin(); it != map.End(); ++it)
  {
    ++count;
    if (it->first == 1)
      seen1 = true;
    if (it->first == 2)
      seen2 = true;
    if (it->first == 3)
      seen3 = true;
  }
  assert(count == 4);
  assert(seen1 && seen2 && seen3);

  /* An empty map has Begin() == End(). */
  Map empty_map;
  assert(empty_map.Begin() == empty_map.End());

  assert(!map.Empty());
  assert(map.Size() == 4);
  assert(map.MaxSize() != 0);
}

void TestNuoUnorderedMultimap::test_insert_and_emplace()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  Map map;

  /* Emplace returns an iterator. */
  Map::iterator it = map.Emplace(1, "one");
  assert(it != map.End());
  assert(it->second == "one");
  assert(map.Size() == 1);

  /* Duplicate keys are inserted. */
  it = map.Emplace(1, "uno");
  assert(map.Size() == 2);
  assert(map.Count(1) == 2);

  /* Insert copies a value. */
  Map::value_type value(2, "two");
  it = map.Insert(value);
  assert(it != map.End());
  assert(it->second == "two");

  /* Insert moves an rvalue value. */
  it = map.Insert(Map::value_type(2, "dos"));
  assert(it->first == 2);
  assert(map.Count(2) == 2);

  /* Insert with a hint. */
  it = map.Insert(map.CBegin(), Map::value_type(3, "three"));
  assert(it->first == 3);
  assert(map.Size() == 5);
}

void TestNuoUnorderedMultimap::test_erase_and_clear()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  Map map;
  map.Emplace(1, "one");
  map.Emplace(1, "uno");
  map.Emplace(2, "two");

  /* Erase by key removes every matching element. */
  assert(map.Erase(1) == 2);
  assert(map.Size() == 1);
  assert(map.Count(1) == 0);
  assert(map.Erase(999) == 0);

  /* Erase by iterator removes a single element. */
  map.Emplace(2, "dos");
  Map::iterator it = map.Find(2);
  Map::iterator next = map.Erase(it);
  assert(map.Size() == 1);
  assert(map.Count(2) == 1);
  assert(next == map.End());

  /* Clear removes all elements. */
  map.Clear();
  assert(map.Empty());
  assert(map.Size() == 0);
}

void TestNuoUnorderedMultimap::test_lookup()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  Map map;
  map.Emplace(1, "one");
  map.Emplace(1, "uno");
  map.Emplace(2, "two");

  /* Find returns one matching element. */
  assert(map.Find(1) != map.End());
  assert(map.Find(1)->first == 1);
  assert(map.Find(999) == map.End());

  /* Count counts all duplicates. */
  assert(map.Count(1) == 2);
  assert(map.Count(2) == 1);
  assert(map.Count(999) == 0);

  /* Contains. */
  assert(map.Contains(2));
  assert(!map.Contains(3));

  /* EqualRange covers all duplicates. */
  nuostl::nuo_pair<Map::iterator, Map::iterator> range = map.EqualRange(1);
  size_t count = 0;
  for (Map::iterator it = range.first; it != range.second; ++it)
  {
    ++count;
    assert(it->first == 1);
  }
  assert(count == 2);

  /* EqualRange is empty for an absent key. */
  range = map.EqualRange(999);
  assert(range.first == map.End());
  assert(range.second == map.End());
}

void TestNuoUnorderedMultimap::test_bucket_and_hash_policy()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string, ModuloHash> Map;

  Map map;
  assert(map.MaxLoadFactor() == 1.0f);

  /* Keys that hash identically land in the same bucket. */
  map.Emplace(1, "one");
  map.Emplace(11, "eleven");
  map.Emplace(21, "twenty-one");
  map.Emplace(31, "thirty-one");
  assert(map.Bucket(1) == map.Bucket(11));
  assert(map.Bucket(11) == map.Bucket(21));
  assert(map.BucketSize(map.Bucket(1)) == 4);
  assert(map.Size() == 4);
  assert(map.LoadFactor() > 0.0f);

  /* Local iterators traverse one bucket. */
  size_t index = map.Bucket(1);
  size_t count = 0;
  for (Map::local_iterator it = map.Begin(index); it != map.End(index); ++it)
  {
    ++count;
    assert(it->first % 10 == 1);
  }
  assert(count == 4);

  /* Rehash preserves the elements. */
  map.Rehash(2 * map.BucketCount());
  assert(map.Bucket(1) == map.Bucket(11));
  assert(map.BucketSize(map.Bucket(1)) == 4);
  assert(map.Size() == 4);

  /* Reserve guarantees room for n elements. */
  Map reserved;
  reserved.Reserve(100);
  assert(reserved.Empty());
  assert(reserved.BucketCount() >= 100);

  /* MaxLoadFactor can be changed. */
  Map lf_map;
  lf_map.MaxLoadFactor(0.5f);
  assert(lf_map.MaxLoadFactor() == 0.5f);
  for (int i = 0; i < 20; ++i)
    lf_map.Emplace(i, "v");
  assert(lf_map.Size() == 20);
  assert(lf_map.LoadFactor() <= 0.5f);
  assert(lf_map.MaxBucketCount() != 0);
}

void TestNuoUnorderedMultimap::test_swap_and_merge()
{
  typedef nuostl::NuoUnorderedMultimap<int, std::string> Map;

  /* Swap exchanges the whole state. */
  Map a;
  a.Emplace(1, "one");
  a.Emplace(2, "two");
  Map b;
  b.Emplace(3, "three");
  a.Swap(b);
  assert(a.Size() == 1);
  assert(b.Size() == 2);
  assert(a.Count(3) == 1);
  assert(b.Count(1) == 1);
  assert(b.Count(2) == 1);

  /* Merge always transfers every element. */
  Map target;
  target.Emplace(1, "one");
  Map source;
  source.Emplace(1, "uno");
  source.Emplace(2, "two");
  target.Merge(source);
  assert(target.Size() == 3);
  assert(target.Count(1) == 2);
  assert(target.Count(2) == 1);
  assert(source.Empty());
}

void TestNuoUnorderedMultimap::test_custom_hash_and_move_only()
{
  /* A multimap with std::string keys needs a custom hash. */
  typedef nuostl::NuoUnorderedMultimap<std::string, int, StringHash> StringMap;
  StringMap smap;
  smap.Emplace("apple", 1);
  smap.Emplace("apple", 2);
  smap.Emplace("banana", 3);
  assert(smap.Size() == 3);
  assert(smap.Count("apple") == 2);
  assert(smap.Count("banana") == 1);
  assert(smap.Contains("apple"));
  assert(!smap.Contains("cherry"));

  /* A multimap with a move-only mapped type. */
  typedef nuostl::NuoUnorderedMultimap<int, MoveOnly> MoveMap;
  MoveMap mmap;
  mmap.Emplace(1, MoveOnly(10));
  mmap.Emplace(1, MoveOnly(20));
  assert(mmap.Size() == 2);
  assert(mmap.Count(1) == 2);
}

void TestNuoUnorderedMultimap::test_nuo_unordered_multimap()
{
  test_constructor_and_assignment();
  test_iterators_and_capacity();
  test_insert_and_emplace();
  test_erase_and_clear();
  test_lookup();
  test_bucket_and_hash_policy();
  test_swap_and_merge();
  test_custom_hash_and_move_only();
}
} /* namespace test */
