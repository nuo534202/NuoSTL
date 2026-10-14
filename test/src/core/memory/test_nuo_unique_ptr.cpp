#include "core/memory/test_nuo_unique_ptr.hpp"

#include <assert.h>

#include <cstddef>
#include <functional>
#include <sstream>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_unique_ptr.hpp"

namespace test
{

namespace
{

struct Base
{
  virtual ~Base() = default;

  virtual int Value() const
  {
    return 0;
  }
};

struct Derived : Base
{
  int Value() const override
  {
    return 1;
  }
};

struct PairValue
{
  PairValue(int first, int second) : first(first), second(second)
  {
  }

  int first;
  int second;
};

struct CountedDeleter
{
  void operator()(int* pointer) const noexcept
  {
    ++(*count);
    delete pointer;
  }

  int* count = nullptr;
};

struct EmptyDeleter
{
  void operator()(int* pointer) const noexcept
  {
    delete pointer;
  }
};

struct OwnedValue
{
  int value = 7;
};

struct ReferenceDeleter
{
  void operator()(int* pointer) noexcept
  {
    ++count;
    delete pointer;
  }

  int count = 0;
};

} /* namespace */

void TestNuoUniquePtr::TestDefaultDelete()
{
  nuostl::NuoDefaultDelete<int> scalar_deleter;
  int* scalar = new int(3);
  scalar_deleter(scalar);

  nuostl::NuoDefaultDelete<int[]> array_deleter;
  int* array = new int[3]{1, 2, 3};
  array_deleter(array);

  nuostl::NuoDefaultDelete<Base> base_deleter =
    nuostl::NuoDefaultDelete<Derived>();
  Base* base = new Derived();
  base_deleter(base);
}

void TestNuoUniquePtr::TestConstructionAndAccess()
{
  using Pointer = nuostl::NuoUniquePtr<int>;

  static_assert(!std::is_copy_constructible_v<Pointer>);
  static_assert(!std::is_copy_assignable_v<Pointer>);
  static_assert(std::is_nothrow_move_constructible_v<Pointer>);
  static_assert(std::is_nothrow_move_assignable_v<Pointer>);

  Pointer empty;
  assert(empty.Get() == nullptr);
  assert(!empty);
  assert(!static_cast<bool>(empty));

  Pointer null(nullptr);
  assert(!null);

  Pointer value(new int(42));
  assert(value);
  assert(value.Get() != nullptr);
  assert(*value == 42);
  assert(value.operator->() == value.Get());
  static_assert(std::is_same_v<decltype(*value), int&>);
  static_assert(std::is_same_v<decltype(value.operator->()), int*>);

  const Pointer& const_value = value;
  assert(*const_value == 42);
  assert(const_value.Get() == value.Get());
}

void TestNuoUniquePtr::TestMoveSemantics()
{
  nuostl::NuoUniquePtr<int> first(new int(11));
  nuostl::NuoUniquePtr<int> moved(std::move(first));
  assert(!first);
  assert(moved && *moved == 11);

  nuostl::NuoUniquePtr<int> assigned;
  assigned = std::move(moved);
  assert(!moved);
  assert(assigned && *assigned == 11);

  nuostl::NuoUniquePtr<int> other(new int(99));
  assigned = std::move(other);
  assert(!other);
  assert(assigned && *assigned == 99);

  assigned = nullptr;
  assert(!assigned);
}

void TestNuoUniquePtr::TestResetAndRelease()
{
  nuostl::NuoUniquePtr<int> value(new int(5));
  int* first = value.Get();
  value.Reset(first);
  assert(value.Get() == first);
  assert(*value == 5);

  value.Reset(new int(8));
  assert(*value == 8);

  int* released = value.Release();
  assert(!value);
  assert(released != nullptr && *released == 8);
  delete released;

  value.Reset(new int(13));
  value.Reset();
  assert(!value);

  nuostl::NuoUniquePtr<int> left(new int(1));
  nuostl::NuoUniquePtr<int> right(new int(2));
  left.Swap(right);
  assert(*left == 2 && *right == 1);
  nuostl::NuoSwap(left, right);
  assert(*left == 1 && *right == 2);
}

void TestNuoUniquePtr::TestCustomDeleter()
{
  int deletions = 0;
  {
    nuostl::NuoUniquePtr<int, CountedDeleter> value(
      new int(21), CountedDeleter{&deletions});
    assert(value.GetDeleter().count == &deletions);
    assert(*value == 21);
    value.Reset(new int(34));
    assert(deletions == 1);
    assert(*value == 34);
  }
  assert(deletions == 2);

  int first_deletions = 0;
  int second_deletions = 0;
  nuostl::NuoUniquePtr<int, CountedDeleter> first(
    new int(1), CountedDeleter{&first_deletions});
  nuostl::NuoUniquePtr<int, CountedDeleter> second(
    new int(2), CountedDeleter{&second_deletions});
  first.Swap(second);
  assert(first.GetDeleter().count == &second_deletions);
  assert(second.GetDeleter().count == &first_deletions);
  assert(*first == 2 && *second == 1);

  using ReferencePointer = nuostl::NuoUniquePtr<int, ReferenceDeleter&>;
  static_assert(!std::is_default_constructible_v<ReferencePointer>);
  static_assert(std::is_move_constructible_v<ReferencePointer>);

  ReferenceDeleter reference_deleter;
  {
    ReferencePointer value(new int(55), reference_deleter);
    assert(value.GetDeleter().count == 0);
    ReferencePointer moved(std::move(value));
    assert(!value);
    assert(*moved == 55);
  }
  assert(reference_deleter.count == 1);
}

void TestNuoUniquePtr::TestPolymorphism()
{
  nuostl::NuoUniquePtr<Derived> derived =
    nuostl::NuoMakeUnique<Derived>();
  nuostl::NuoUniquePtr<Base> base(std::move(derived));
  assert(!derived);
  assert(base && base->Value() == 1);

  nuostl::NuoUniquePtr<Derived> derived_again =
    nuostl::NuoMakeUnique<Derived>();
  nuostl::NuoUniquePtr<Base> assigned;
  assigned = std::move(derived_again);
  assert(!derived_again);
  assert(assigned && assigned->Value() == 1);
}

void TestNuoUniquePtr::TestArrays()
{
  static_assert(!std::is_copy_constructible_v<nuostl::NuoUniquePtr<int[]>>);
  static_assert(!std::is_copy_assignable_v<nuostl::NuoUniquePtr<int[]>>);

  nuostl::NuoUniquePtr<int[]> empty;
  assert(!empty);
  assert(empty.Get() == nullptr);

  nuostl::NuoUniquePtr<int[]> values(new int[4]{1, 2, 3, 4});
  assert(values[0] == 1);
  assert(values[3] == 4);
  assert(values.Get() == &values[0]);
  static_assert(std::is_same_v<decltype(values[0]), int&>);

  values.Reset(new int[2]{7, 8});
  assert(values[0] == 7 && values[1] == 8);

  int* released = values.Release();
  assert(!values);
  assert(released[0] == 7 && released[1] == 8);
  delete[] released;

  nuostl::NuoUniquePtr<Derived[]> derived(new Derived[2]);
  assert(derived[0].Value() == 1 && derived[1].Value() == 1);
}

void TestNuoUniquePtr::TestMakeUnique()
{
  auto scalar = nuostl::NuoMakeUnique<int>(17);
  assert(scalar && *scalar == 17);

  auto pair = nuostl::NuoMakeUnique<PairValue>(3, 4);
  assert(pair && pair->first == 3 && pair->second == 4);

  auto array = nuostl::NuoMakeUnique<int[]>(4);
  assert(array);
  assert(array[0] == 0 && array[1] == 0 && array[2] == 0 && array[3] == 0);

  auto owned = nuostl::NuoMakeUniqueForOverwrite<OwnedValue>();
  assert(owned && owned->value == 7);

  auto overwritten = nuostl::NuoMakeUniqueForOverwrite<int[]>(3);
  assert(overwritten);
  overwritten[0] = 1;
  overwritten[1] = 2;
  overwritten[2] = 3;
  assert(overwritten[0] == 1 && overwritten[2] == 3);
}

void TestNuoUniquePtr::TestComparisons()
{
  nuostl::NuoUniquePtr<int> first(new int(1));
  nuostl::NuoUniquePtr<int> second(new int(2));
  nuostl::NuoUniquePtr<int> null;

  assert(first == first);
  assert(first != second);
  assert(!(first < first));
  assert(first <= first);
  assert(first >= first);
  assert(!(first > first));

  assert(first != nullptr);
  assert(nullptr != first);
  assert(first > nullptr);
  assert(nullptr < first);
  assert(!(first < nullptr));
  assert(!(nullptr > first));

  assert(null == nullptr);
  assert(nullptr == null);
  assert(!(null != nullptr));
  assert(null <= nullptr);
  assert(null >= nullptr);
  assert(!(null < nullptr));
  assert(!(null > nullptr));

  nuostl::NuoUniquePtr<Derived> derived(new Derived());
  nuostl::NuoUniquePtr<Base> base(new Derived());
  assert(derived != base);
  assert(derived <= base || derived >= base);
}

void TestNuoUniquePtr::TestHashAndLayout()
{
  nuostl::NuoUniquePtr<int> first(new int(1));
  nuostl::NuoUniquePtr<int> second(new int(2));
  nuostl::NuoUniquePtr<int> null;

  std::hash<nuostl::NuoUniquePtr<int>> hasher;
  assert(hasher(first) == hasher(first));
  assert(hasher(first) != hasher(second));
  assert(hasher(null) == hasher(null));

  assert(sizeof(nuostl::NuoUniquePtr<int>) == sizeof(int*));
  assert(sizeof(nuostl::NuoUniquePtr<int, EmptyDeleter>) == sizeof(int*));
  assert(sizeof(nuostl::NuoUniquePtr<int[]>) == sizeof(int*));
}

void TestNuoUniquePtr::TestOutput()
{
  nuostl::NuoUniquePtr<char[]> text(new char[6]{'h', 'e', 'l', 'l', 'o', 0});
  std::ostringstream stream;
  stream << text;
  assert(stream.str() == "hello");
}

void TestNuoUniquePtr::TestSuite()
{
  TestDefaultDelete();
  TestConstructionAndAccess();
  TestMoveSemantics();
  TestResetAndRelease();
  TestCustomDeleter();
  TestPolymorphism();
  TestArrays();
  TestMakeUnique();
  TestComparisons();
  TestHashAndLayout();
  TestOutput();
}

} /* namespace test */
