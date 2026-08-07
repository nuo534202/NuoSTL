#include "core/assoc_cont/test_nuo_unordered_set.hpp"

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

  bool operator==(const MoveOnly& rhs) const
  {
    return value_ == rhs.value_;
  }

  int Value() const
  {
    return value_;
  }

private:
  int value_;
};

/* Custom hash for the move-only type. */
class MoveOnlyHash
{
public:
  size_t operator()(const MoveOnly& key) const
  {
    return static_cast<size_t>(key.Value());
  }
};
} /* namespace */

namespace test
{
/* ------------------------------------------------------------- */
/* TestNuoUnorderedSet                                            */
/* ------------------------------------------------------------- */

void TestNuoUnorderedSet::test_constructor_and_assignment()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  /* Default constructor. */
  Set set;
  assert(set.Empty());
  assert(set.Size() == 0);
  assert(set.BucketCount() != 0);

  /* Constructor with an explicit bucket count. */
  Set bucket_set(32);
  assert(bucket_set.Empty());
  assert(bucket_set.BucketCount() >= 32);

  /* Range constructor. */
  Set base;
  base.Insert(1);
  base.Insert(2);
  base.Insert(3);
  Set range_set(base.Begin(), base.End());
  assert(range_set.Size() == 3);
  assert(range_set.Contains(1));
  assert(range_set.Contains(2));
  assert(range_set.Contains(3));

  /* Initializer list constructor. */
  Set list_set = {1, 2, 2, 3, 3, 3};
  assert(list_set.Size() == 3);

  /* Copy constructor. */
  Set copy_set(list_set);
  assert(copy_set.Size() == 3);
  assert(copy_set.Contains(2));

  /* Move constructor leaves the source empty. */
  Set move_set(nuostl::NuoMove(copy_set));
  assert(move_set.Size() == 3);
  assert(move_set.Contains(1));
  assert(copy_set.Empty());

  /* Copy assignment. */
  Set assigned;
  assigned = list_set;
  assert(assigned.Size() == 3);
  assert(assigned.Contains(3));

  /* Move assignment. */
  Set move_assigned;
  move_assigned = nuostl::NuoMove(assigned);
  assert(move_assigned.Size() == 3);
  assert(move_assigned.Contains(2));
  assert(assigned.Empty());

  /* Initializer list assignment replaces the content. */
  move_assigned = {10, 20};
  assert(move_assigned.Size() == 2);
  assert(move_assigned.Contains(10));
  assert(!move_assigned.Contains(1));
}

void TestNuoUnorderedSet::test_iterators_and_capacity()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  set.Insert(4);

  /* Iterating over the container visits every element exactly once. */
  size_t count = 0;
  bool seen1 = false;
  bool seen2 = false;
  bool seen3 = false;
  bool seen4 = false;
  for (Set::iterator it = set.Begin(); it != set.End(); ++it)
  {
    ++count;
    if (*it == 1)
      seen1 = true;
    if (*it == 2)
      seen2 = true;
    if (*it == 3)
      seen3 = true;
    if (*it == 4)
      seen4 = true;
  }
  assert(count == 4);
  assert(seen1 && seen2 && seen3 && seen4);

  /* Const iterators are implicitly created from non-const iterators.
     Values cannot be mutated through set iterators. */
  Set::const_iterator cit = set.CBegin();
  assert(cit == set.CEnd() || *cit == 1 || *cit == 2 || *cit == 3 || *cit == 4);
  assert(set.CBegin() == set.Begin());

  /* An empty set has Begin() == End(). */
  Set empty_set;
  assert(empty_set.Begin() == empty_set.End());
  assert(empty_set.CBegin() == empty_set.CEnd());

  assert(!set.Empty());
  assert(set.Size() == 4);
  assert(set.MaxSize() != 0);
}

void TestNuoUnorderedSet::test_insert_and_emplace()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  Set set;

  /* Emplace returns a pair (iterator, bool). */
  nuostl::nuo_pair<Set::iterator, bool> r = set.Emplace(1);
  assert(r.second);
  assert(*(r.first) == 1);
  assert(set.Size() == 1);

  /* Duplicate keys are rejected by Emplace. */
  r = set.Emplace(1);
  assert(!r.second);
  assert((*(r.first)) == 1);
  assert(set.Size() == 1);

  /* Insert copies a value. */
  const int value = 2;
  r = set.Insert(value);
  assert(r.second);
  assert((*(r.first)) == 2);

  /* Insert moves an rvalue value. */
  r = set.Insert(3);
  assert(r.second);
  assert((*(r.first)) == 3);

  /* Inserting a duplicate returns false. */
  r = set.Insert(3);
  assert(!r.second);
  assert(set.Size() == 3);

  /* Insert with a hint. */
  Set::iterator it =
      set.Insert(set.CBegin(), 4);
  assert(it != set.End());
  assert((*it) == 4);
  assert(set.Size() == 4);

  /* Range insert. */
  int arr[] = {5, 6, 6, 7};
  set.Insert(arr, arr + 4);
  assert(set.Size() == 7);

  /* Initializer list insert. */
  set.Insert({8, 8, 9});
  assert(set.Size() == 9);
  assert(set.Contains(8));
  assert(set.Contains(9));

  /* EmplaceHint ignores the hint and inserts the value. */
  Set::iterator her = set.EmplaceHint(set.CBegin(), 10);
  assert(her != set.End());
  assert((*her) == 10);
  assert(set.Contains(10));
}

void TestNuoUnorderedSet::test_erase_and_clear()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);

  /* Erase by key returns 1 if present, 0 if not. */
  assert(set.Erase(2) == 1);
  assert(set.Size() == 2);
  assert(!set.Contains(2));
  assert(set.Erase(999) == 0);

  /* Erase by iterator removes a single element. */
  Set::iterator it = set.Find(1);
  Set::iterator next = set.Erase(it);
  assert(set.Size() == 1);
  assert(!set.Contains(1));
  assert(next != set.End());

  /* Erase a range. */
  set.Insert(4);
  set.Insert(5);
  Set::iterator first = set.Find(4);
  Set::iterator last = set.Find(5);
  Set::iterator after = set.Erase(first, last);
  assert(after != set.End());

  /* Clear removes all elements. */
  set.Clear();
  assert(set.Empty());
  assert(set.Size() == 0);
}

void TestNuoUnorderedSet::test_lookup()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(2);

  /* Find returns an iterator to the matching element. */
  Set::iterator it = set.Find(2);
  assert(it != set.End());
  assert((*it) == 2);
  assert(set.Find(999) == set.End());

  /* Find on a const container. */
  const Set& cref = set;
  Set::const_iterator cit = cref.Find(1);
  assert(cit != cref.End());
  assert((*cit) == 1);

  /* Count returns 1 for a present key and 0 for an absent key. */
  assert(set.Count(1) == 1);
  assert(set.Count(999) == 0);

  /* Contains. */
  assert(set.Contains(2));
  assert(!set.Contains(3));

  /* EqualRange covers exactly the single matching element. */
  nuostl::nuo_pair<Set::iterator, Set::iterator> range = set.EqualRange(1);
  assert(range.first != set.End());
  assert((*(range.first)) == 1);
  Set::iterator probe = range.first;
  ++probe;
  assert(probe == range.second);

  /* EqualRange is empty for an absent key. */
  range = set.EqualRange(999);
  assert(range.first == set.End());
  assert(range.second == set.End());

  /* Const EqualRange. */
  nuostl::nuo_pair<Set::const_iterator, Set::const_iterator> crange =
      cref.EqualRange(1);
  assert(crange.first != cref.End());
  assert(crange.second != crange.first);
}

void TestNuoUnorderedSet::test_bucket_and_hash_policy()
{
  typedef nuostl::NuoUnorderedSet<int, ModuloHash> Set;

  Set set;
  assert(set.MaxLoadFactor() == 1.0f);

  /* Keys with the same bucket land together. */
  set.Insert(1);
  set.Insert(11);
  set.Insert(21);
  assert(set.Bucket(1) == set.Bucket(11));
  assert(set.Bucket(11) == set.Bucket(21));
  assert(set.BucketSize(set.Bucket(1)) == 3);
  assert(set.Size() == 3);

  /* Local iterators traverse one bucket. */
  size_t index = set.Bucket(1);
  size_t count = 0;
  for (Set::local_iterator it = set.Begin(index); it != set.End(index); ++it)
  {
    ++count;
    assert((*it) % 10 == 1);
  }
  assert(count == 3);

  /* Rehash preserves the elements. */
  set.Rehash(2 * set.BucketCount());
  assert(set.Bucket(1) == set.Bucket(11));
  assert(set.BucketSize(set.Bucket(1)) == 3);
  assert(set.Size() == 3);

  /* Reserve guarantees room for n elements. */
  Set reserved;
  reserved.Reserve(300);
  assert(reserved.Empty());
  assert(reserved.BucketCount() >= 300);

  /* MaxLoadFactor can be changed. */
  Set lf_set;
  lf_set.MaxLoadFactor(0.5f);
  assert(lf_set.MaxLoadFactor() == 0.5f);
  for (int i = 0; i < 20; ++i)
    lf_set.Insert(i);
  assert(lf_set.Size() == 20);
  assert(lf_set.LoadFactor() <= 0.5f);
  assert(lf_set.MaxBucketCount() != 0);
}

void TestNuoUnorderedSet::test_swap_and_merge()
{
  typedef nuostl::NuoUnorderedSet<int> Set;

  /* Swap exchanges the whole state. */
  Set a;
  a.Insert(1);
  a.Insert(2);
  Set b;
  b.Insert(3);
  a.Swap(b);
  assert(a.Size() == 1);
  assert(b.Size() == 2);
  assert(a.Contains(3));
  assert(b.Contains(1));
  assert(b.Contains(2));

  /* Merge transfers keys absent from the target, deduplicating them. */
  Set target;
  target.Insert(1);
  Set source;
  source.Insert(1);
  source.Insert(2);
  source.Insert(3);
  target.Merge(source);
  assert(target.Size() == 3);
  assert(target.Contains(1));
  assert(target.Contains(2));
  assert(target.Contains(3));
  assert(source.Size() == 1);
  assert(source.Contains(1));

  /* Merge from a multiset transfers unique keys only. */
  nuostl::NuoUnorderedMultiset<int> msrc;
  msrc.Insert(4);
  msrc.Insert(4);
  msrc.Insert(5);
  target.Merge(msrc);
  assert(target.Size() == 5);
  assert(target.Contains(4));
  assert(target.Contains(5));
  assert(msrc.Size() == 1);
  assert(msrc.Count(5) == 0);
}

void TestNuoUnorderedSet::test_custom_hash_and_move_only()
{
  /* A set with std::string keys needs a custom hash. */
  typedef nuostl::NuoUnorderedSet<std::string, StringHash> StringSet;
  StringSet sset;
  sset.Insert("apple");
  sset.Insert("apple");
  sset.Insert("banana");
  assert(sset.Size() == 2);
  assert(sset.Contains("apple"));
  assert(!sset.Contains("cherry"));

  /* A set with a move-only value type inserts by move. */
  typedef nuostl::NuoUnorderedSet<MoveOnly, MoveOnlyHash> MoveSet;
  MoveSet mset;
  nuostl::nuo_pair<MoveSet::iterator, bool> mr = mset.Insert(MoveOnly(10));
  assert(mr.second);
  assert(mr.first != mset.End());
  assert(mr.first->Value() == 10);
  mr = mset.Insert(MoveOnly(10));
  assert(!mr.second);
  assert(mset.Size() == 1);
}

void TestNuoUnorderedSet::test_nuo_unordered_set()
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

/* ------------------------------------------------------------- */
/* TestNuoUnorderedMultiset                                       */
/* ------------------------------------------------------------- */

void TestNuoUnorderedMultiset::test_constructor_and_assignment()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  /* Default constructor. */
  Set set;
  assert(set.Empty());
  assert(set.Size() == 0);

  /* Initializer list constructor keeps duplicates. */
  Set list_set = {1, 1, 2, 3};
  assert(list_set.Size() == 4);
  assert(list_set.Count(1) == 2);

  /* Range constructor keeps duplicates. */
  Set base;
  base.Insert(2);
  base.Insert(2);
  Set range_set(base.Begin(), base.End());
  assert(range_set.Size() == 2);
  assert(range_set.Count(2) == 2);

  /* Copy constructor. */
  Set copy_set(list_set);
  assert(copy_set.Size() == 4);

  /* Move constructor empties the source. */
  Set moved(nuostl::NuoMove(copy_set));
  assert(moved.Size() == 4);
  assert(copy_set.Empty());

  /* Copy assignment. */
  Set assigned;
  assigned = list_set;
  assert(assigned.Size() == 4);

  /* Move assignment. */
  Set move_assigned;
  move_assigned = nuostl::NuoMove(assigned);
  assert(move_assigned.Size() == 4);

  /* Initializer list assignment replaces the content. */
  move_assigned = {7, 7, 7};
  assert(move_assigned.Size() == 3);
  assert(move_assigned.Count(7) == 3);
}

void TestNuoUnorderedMultiset::test_iterators_and_capacity()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(1);
  set.Insert(2);

  /* Iterating visits every element. */
  size_t count = 0;
  for (Set::iterator it = set.Begin(); it != set.End(); ++it)
  {
    ++count;
    assert((*it) == 1 || (*it) == 2);
  }
  assert(count == 3);

  assert(!set.Empty());
  assert(set.Size() == 3);
  assert(set.MaxSize() != 0);

  Set empty_set;
  assert(empty_set.Begin() == empty_set.End());
}

void TestNuoUnorderedMultiset::test_insert_and_emplace()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  Set set;

  /* Emplace returns an iterator. */
  Set::iterator it = set.Emplace(1);
  assert(it != set.End());
  assert((*it) == 1);
  assert(set.Size() == 1);

  /* Duplicate values are inserted. */
  it = set.Emplace(1);
  assert(set.Size() == 2);
  assert(set.Count(1) == 2);

  /* Insert copies a value. */
  const int value = 2;
  it = set.Insert(value);
  assert(it != set.End());
  assert((*it) == 2);

  /* Insert moves an rvalue value. */
  it = set.Insert(3);
  assert((*it) == 3);

  /* Insert with a hint. */
  it = set.Insert(set.CBegin(), 4);
  assert((*it) == 4);
  assert(set.Size() == 5);

  /* Range insert keeps duplicates. */
  int arr[] = {5, 5, 6};
  set.Insert(arr, arr + 3);
  assert(set.Size() == 8);
  assert(set.Count(5) == 2);

  /* Initializer list insert. */
  set.Insert({7, 7});
  assert(set.Size() == 10);
  assert(set.Count(7) == 2);
}

void TestNuoUnorderedMultiset::test_erase_and_clear()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(1);
  set.Insert(2);

  /* Erase by key removes every matching element. */
  assert(set.Erase(1) == 2);
  assert(set.Size() == 1);
  assert(set.Count(1) == 0);
  assert(set.Erase(999) == 0);

  /* Erase by iterator removes a single element. */
  set.Insert(2);
  Set::iterator it = set.Find(2);
  Set::iterator next = set.Erase(it);
  assert(set.Size() == 1);
  assert(set.Count(2) == 1);
  assert(next == set.End());

  /* Clear removes all elements. */
  set.Clear();
  assert(set.Empty());
  assert(set.Size() == 0);
}

void TestNuoUnorderedMultiset::test_lookup()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  Set set;
  set.Insert(1);
  set.Insert(1);
  set.Insert(2);

  /* Find returns one matching element. */
  assert(set.Find(1) != set.End());
  assert(set.Find(999) == set.End());

  /* Count counts all duplicates. */
  assert(set.Count(1) == 2);
  assert(set.Count(2) == 1);
  assert(set.Count(999) == 0);

  /* Contains. */
  assert(set.Contains(2));
  assert(!set.Contains(3));

  /* EqualRange covers all duplicates. */
  nuostl::nuo_pair<Set::iterator, Set::iterator> range = set.EqualRange(1);
  size_t count = 0;
  for (Set::iterator it = range.first; it != range.second; ++it)
  {
    ++count;
    assert((*it) == 1);
  }
  assert(count == 2);

  /* EqualRange is empty for an absent key. */
  range = set.EqualRange(999);
  assert(range.first == set.End());
  assert(range.second == set.End());
}

void TestNuoUnorderedMultiset::test_bucket_and_hash_policy()
{
  typedef nuostl::NuoUnorderedMultiset<int, ModuloHash> Set;

  Set set;
  set.Insert(1);
  set.Insert(11);
  set.Insert(21);
  set.Insert(31);
  assert(set.Bucket(1) == set.Bucket(11));
  assert(set.BucketSize(set.Bucket(1)) == 4);
  assert(set.Size() == 4);

  /* Local iterators traverse one bucket. */
  size_t index = set.Bucket(1);
  size_t count = 0;
  for (Set::local_iterator it = set.Begin(index); it != set.End(index); ++it)
  {
    ++count;
    assert((*it) % 10 == 1);
  }
  assert(count == 4);

  /* Rehash preserves the elements. */
  set.Rehash(2 * set.BucketCount());
  assert(set.BucketSize(set.Bucket(1)) == 4);
  assert(set.Size() == 4);

  /* MaxLoadFactor controls the load factor. */
  Set lf_set;
  lf_set.MaxLoadFactor(0.5f);
  assert(lf_set.MaxLoadFactor() == 0.5f);
  for (int i = 0; i < 20; ++i)
    lf_set.Insert(i);
  assert(lf_set.Size() == 20);
  assert(lf_set.LoadFactor() <= 0.5f);
  assert(lf_set.MaxBucketCount() != 0);
}

void TestNuoUnorderedMultiset::test_swap_and_merge()
{
  typedef nuostl::NuoUnorderedMultiset<int> Set;

  /* Swap exchanges the whole state. */
  Set a;
  a.Insert(1);
  a.Insert(1);
  Set b;
  b.Insert(2);
  a.Swap(b);
  assert(a.Size() == 1);
  assert(b.Size() == 2);
  assert(a.Contains(2));
  assert(b.Count(1) == 2);

  /* Merge always transfers every element, preserving duplicates. */
  nuostl::NuoUnorderedMultiset<int> target;
  target.Insert(1);
  nuostl::NuoUnorderedMultiset<int> source;
  source.Insert(1);
  source.Insert(2);
  source.Insert(2);
  target.Merge(source);
  assert(target.Size() == 4);
  assert(target.Count(1) == 2);
  assert(target.Count(2) == 2);
  assert(source.Empty());

  /* Merge from a set transfers all of its unique elements. */
  nuostl::NuoUnorderedSet<int> ss;
  ss.Insert(3);
  ss.Insert(4);
  target.Merge(ss);
  assert(target.Size() == 6);
  assert(target.Count(3) == 1);
  assert(target.Count(4) == 1);
  assert(ss.Empty());
}

void TestNuoUnorderedMultiset::test_custom_hash_and_move_only()
{
  /* A multiset with std::string keys needs a custom hash. */
  typedef nuostl::NuoUnorderedMultiset<std::string, StringHash> StringSet;
  StringSet sset;
  sset.Insert("apple");
  sset.Insert("apple");
  sset.Insert("banana");
  assert(sset.Size() == 3);
  assert(sset.Count("apple") == 2);
  assert(sset.Contains("apple"));
  assert(!sset.Contains("cherry"));

  /* A multiset with a move-only value type inserts by move. */
  typedef nuostl::NuoUnorderedMultiset<MoveOnly, MoveOnlyHash> MoveSet;
  MoveSet mset;
  mset.Insert(MoveOnly(10));
  mset.Insert(MoveOnly(10));
  assert(mset.Size() == 2);
  assert(mset.Count(MoveOnly(10)) == 2);
}

void TestNuoUnorderedMultiset::test_nuo_unordered_multiset()
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