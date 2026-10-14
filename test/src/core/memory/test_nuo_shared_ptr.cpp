#include "core/memory/test_nuo_shared_ptr.hpp"

#include <assert.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_shared_ptr.hpp"

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

struct Other : Base
{
  int Value() const override
  {
    return 2;
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

struct CountedObject
{
  explicit CountedObject(int* destroyed) : destroyed_(destroyed)
  {
  }

  ~CountedObject()
  {
    ++(*destroyed_);
  }

  int* destroyed_;
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

struct VoidDeleter
{
  void operator()(void* pointer) const noexcept
  {
    ::operator delete(pointer);
  }
};

struct Holder
{
  int value = 7;
};

struct Self : nuostl::NuoEnableSharedFromThis<Self>
{
  int value = 5;
};

struct EnableBase
{
  virtual ~EnableBase() = default;
};

struct DerivedFromBase : EnableBase,
                         nuostl::NuoEnableSharedFromThis<EnableBase>
{
  int value = 6;
};

struct Node
{
  explicit Node(int* alive) : alive_(alive)
  {
  }

  ~Node()
  {
    ++(*alive_);
  }

  nuostl::NuoSharedPtr<Node> next;
  nuostl::NuoWeakPtr<Node> previous;
  int* alive_;
};

} /* namespace */

void TestNuoSharedPtr::TestConstructionAndCounting()
{
  using Pointer = nuostl::NuoSharedPtr<int>;

  Pointer empty;
  assert(empty.Get() == nullptr);
  assert(!empty);
  assert(empty.UseCount() == 0);
  assert(!empty.Unique());

  Pointer null(nullptr);
  assert(!null);

  Pointer value(new int(42));
  assert(value);
  assert(value.Get() != nullptr);
  assert(*value == 42);
  assert(value.operator->() == value.Get());
  assert(value.UseCount() == 1);
  assert(value.Unique());

  {
    Pointer copy(value);
    assert(copy == value);
    assert(copy.Get() == value.Get());
    assert(copy.UseCount() == 2);
    assert(value.UseCount() == 2);
    assert(!value.Unique());
    Pointer assigned;
    assigned = copy;
    assert(assigned.Get() == value.Get());
    assert(value.UseCount() == 3);
  }
  assert(value.UseCount() == 1);

  const Pointer& const_value = value;
  assert(*const_value == 42);
  assert(const_value.UseCount() == 1);

  nuostl::NuoSharedPtr<const int> const_pointer(value);
  assert(const_pointer.Get() == value.Get());
  assert(*const_pointer == 42);
  assert(value.UseCount() == 2);
}

void TestNuoSharedPtr::TestMoveSemantics()
{
  nuostl::NuoSharedPtr<int> first(new int(11));
  nuostl::NuoSharedPtr<int> moved(std::move(first));
  assert(!first);
  assert(first.UseCount() == 0);
  assert(moved && *moved == 11 && moved.UseCount() == 1);

  nuostl::NuoSharedPtr<int> assigned;
  assigned = std::move(moved);
  assert(!moved);
  assert(assigned && *assigned == 11);

  nuostl::NuoSharedPtr<int> other(new int(99));
  assigned = std::move(other);
  assert(!other);
  assert(assigned && *assigned == 99);

  assigned = nullptr;
  assert(!assigned);

  nuostl::NuoSharedPtr<Derived> derived(new Derived());
  nuostl::NuoSharedPtr<Base> base(std::move(derived));
  assert(!derived);
  assert(base && base->Value() == 1);

  nuostl::NuoSharedPtr<Derived> second(new Derived());
  nuostl::NuoSharedPtr<Base> converted;
  converted = std::move(second);
  assert(!second);
  assert(converted && converted->Value() == 1);
}

void TestNuoSharedPtr::TestCustomDeleter()
{
  int deletions = 0;
  {
    nuostl::NuoSharedPtr<int> value(new int(21),
                                   CountedDeleter{&deletions});
    assert(*value == 21);
    assert(value.UseCount() == 1);
    value.Reset(new int(34), CountedDeleter{&deletions});
    assert(deletions == 1);
    assert(*value == 34);

    nuostl::NuoSharedPtr<int> copy(value);
    assert(copy.UseCount() == 2);
  }
  assert(deletions == 2);

  nuostl::NuoSharedPtr<void> erased(::operator new(8), VoidDeleter{});
  assert(erased);
  assert(erased.UseCount() == 1);
  erased.Reset();
}

void TestNuoSharedPtr::TestMakeShared()
{
  auto value = nuostl::NuoMakeShared<int>(17);
  assert(value && *value == 17 && value.UseCount() == 1);

  auto pair = nuostl::NuoMakeShared<PairValue>(3, 4);
  assert(pair && pair->first == 3 && pair->second == 4);

  int destroyed = 0;
  {
    auto object = nuostl::NuoMakeShared<CountedObject>(&destroyed);
    assert(object.UseCount() == 1);
    assert(destroyed == 0);
  }
  assert(destroyed == 1);

  auto overwrite = nuostl::NuoMakeSharedForOverwrite<int>();
  assert(overwrite && overwrite.UseCount() == 1);
}

void TestNuoSharedPtr::TestAliasing()
{
  auto holder = nuostl::NuoMakeShared<Holder>();
  holder->value = 23;
  nuostl::NuoSharedPtr<int> alias(holder, &holder->value);
  assert(alias.Get() == &holder->value);
  assert(*alias == 23);
  assert(alias.UseCount() == 2);
  assert(holder.UseCount() == 2);

  holder.Reset();
  assert(alias.UseCount() == 1);
  assert(*alias == 23);

  nuostl::NuoSharedPtr<int> moved_alias(std::move(alias), nullptr);
  assert(!alias);
  assert(moved_alias.Get() == nullptr);
  assert(moved_alias.UseCount() == 1);
}

void TestNuoSharedPtr::TestUniquePtrConversion()
{
  nuostl::NuoSharedPtr<int> shared(nuostl::NuoMakeUnique<int>(9));
  assert(shared && *shared == 9);
  assert(shared.UseCount() == 1);

  nuostl::NuoSharedPtr<Base> base(nuostl::NuoMakeUnique<Derived>());
  assert(base && base->Value() == 1);

  int deletions = 0;
  nuostl::NuoUniquePtr<int, CountedDeleter> unique(
    new int(12), CountedDeleter{&deletions});
  nuostl::NuoSharedPtr<int> counted(std::move(unique));
  assert(!unique);
  assert(counted && *counted == 12);
  counted.Reset();
  assert(deletions == 1);
}

void TestNuoSharedPtr::TestArrays()
{
  nuostl::NuoSharedPtr<int[]> values(new int[3]{1, 2, 3});
  assert(values.Get() != nullptr);
  assert(values[0] == 1 && values[1] == 2 && values[2] == 3);
  assert(values.UseCount() == 1);
  static_assert(std::is_same_v<decltype(values[0]), int&>);

  auto made = nuostl::NuoMakeShared<int[]>(4);
  assert(made);
  assert(made[0] == 0 && made[1] == 0 && made[2] == 0 && made[3] == 0);

  auto overwritten = nuostl::NuoMakeSharedForOverwrite<int[]>(3);
  assert(overwritten);
  overwritten[0] = 7;
  overwritten[1] = 8;
  overwritten[2] = 9;
  assert(overwritten[0] == 7 && overwritten[2] == 9);

  nuostl::NuoWeakPtr<int[]> weak(values);
  assert(!weak.Expired());
  auto locked = weak.Lock();
  assert(locked.Get() == values.Get());
  assert(locked[1] == 2);
  assert(values.UseCount() == 2);

  std::hash<nuostl::NuoSharedPtr<int[]>> hasher;
  assert(hasher(values) == hasher(values));

  nuostl::NuoSharedPtr<Derived[]> derived(new Derived[2]);
  nuostl::NuoSharedPtr<Base[]> base(derived);
  assert(base[0].Value() == 1 && base[1].Value() == 1);
  assert(derived.UseCount() == 2);
  base.Reset(new Derived[2]);
  assert(base[0].Value() == 1 && base.UseCount() == 1);
}

void TestNuoSharedPtr::TestWeakPtr()
{
  nuostl::NuoSharedPtr<int> value(new int(11));
  nuostl::NuoWeakPtr<int> weak(value);
  assert(!weak.Expired());
  assert(weak.UseCount() == 1);

  {
    auto locked = weak.Lock();
    assert(locked.Get() == value.Get());
    assert(locked.UseCount() == 2);
  }
  assert(weak.UseCount() == 1);

  nuostl::NuoWeakPtr<int> copied(weak);
  assert(copied.UseCount() == 1);
  copied = weak;
  assert(copied.UseCount() == 1);

  nuostl::NuoWeakPtr<int> move_source(weak);
  nuostl::NuoWeakPtr<int> moved(std::move(move_source));
  assert(!moved.Expired());
  assert(move_source.Expired());

  value.Reset();
  assert(weak.Expired());
  assert(weak.UseCount() == 0);
  assert(!weak.Lock());
  assert(copied.Expired());
  assert(moved.Expired());
}

void TestNuoSharedPtr::TestEnableSharedFromThis()
{
  auto self = nuostl::NuoMakeShared<Self>();
  auto first = self->SharedFromThis();
  assert(first.Get() == self.Get());
  assert(self.UseCount() == 2);

  const Self& const_self = *self;
  auto const_shared = const_self.SharedFromThis();
  assert(const_shared.Get() == self.Get());
  assert(self.UseCount() == 3);

  auto weak = self->WeakFromThis();
  assert(!weak.Expired());
  auto const_weak = const_self.WeakFromThis();
  assert(!const_weak.Expired());
  auto second = weak.Lock();
  assert(second.Get() == self.Get());
  assert(self.UseCount() == 4);

  first.Reset();
  const_shared.Reset();
  second.Reset();
  self.Reset();
  assert(weak.Expired());
  assert(const_weak.Expired());

  bool caught = false;
  try
  {
    Self unattached;
    unattached.SharedFromThis();
  }
  catch (const std::bad_weak_ptr&)
  {
    caught = true;
  }
  assert(caught);

  auto derived = nuostl::NuoMakeShared<DerivedFromBase>();
  auto base_shared = derived->SharedFromThis();
  assert(base_shared.Get() == static_cast<EnableBase*>(derived.Get()));
  assert(base_shared.UseCount() == 2);
  auto base_weak = derived->WeakFromThis();
  assert(!base_weak.Expired());
  assert(base_weak.Lock().Get() == base_shared.Get());
}

void TestNuoSharedPtr::TestCyclicReferences()
{
  int alive = 0;
  {
    auto first = nuostl::NuoMakeShared<Node>(&alive);
    auto second = nuostl::NuoMakeShared<Node>(&alive);
    first->next = second;
    second->previous = first;
    assert(first.UseCount() == 1);
    assert(second.UseCount() == 2);
    assert(!second->previous.Expired());
  }
  assert(alive == 2);
}

void TestNuoSharedPtr::TestCasts()
{
  auto derived = nuostl::NuoMakeShared<Derived>();
  nuostl::NuoSharedPtr<Base> base(derived);
  assert(base->Value() == 1);

  auto down = nuostl::NuoDynamicPointerCast<Derived>(base);
  assert(down);
  assert(down.Get() == base.Get());
  assert(down->Value() == 1);

  auto static_down = nuostl::NuoStaticPointerCast<Derived>(base);
  assert(static_down.Get() == base.Get());

  auto const_down = nuostl::NuoConstPointerCast<const Derived>(down);
  assert(const_down.Get() == down.Get());
  assert(const_down->Value() == 1);

  auto raw = nuostl::NuoReinterpretPointerCast<int>(base);
  assert(raw.Get() == reinterpret_cast<int*>(base.Get()));

  nuostl::NuoSharedPtr<Base> other = nuostl::NuoMakeShared<Other>();
  auto failed = nuostl::NuoDynamicPointerCast<Derived>(std::move(other));
  assert(!failed);
  assert(!other);
}

void TestNuoSharedPtr::TestComparisonsAndHash()
{
  auto first = nuostl::NuoMakeShared<int>(1);
  auto second = first;
  nuostl::NuoSharedPtr<int> null;

  assert(first == second);
  assert(!(first < second));
  assert(first <= second);
  assert(first >= second);
  assert(first != nullptr);
  assert(nullptr != first);
  assert(first > nullptr);
  assert(nullptr < first);

  assert(null == nullptr);
  assert(nullptr == null);
  assert(!(null < nullptr));
  assert(null <= nullptr);
  assert(null >= nullptr);

  std::hash<nuostl::NuoSharedPtr<int>> hasher;
  assert(hasher(first) == hasher(second));
  assert(hasher(null) == hasher(null));

  auto other = nuostl::NuoMakeShared<int>(2);
  nuostl::NuoWeakPtr<int> weak(first);
  assert(!first.OwnerBefore(first));
  assert(first.OwnerBefore(other) || other.OwnerBefore(first));
  assert(!first.OwnerBefore(weak));
  assert(!weak.OwnerBefore(first));
  assert(weak.OwnerBefore(other) || other.OwnerBefore(weak));

  auto first_value = nuostl::NuoMakeShared<int>(1);
  auto second_value = nuostl::NuoMakeShared<int>(2);
  nuostl::NuoSwap(first_value, second_value);
  assert(*first_value == 2 && *second_value == 1);

  nuostl::NuoWeakPtr<int> first_weak(first_value);
  nuostl::NuoWeakPtr<int> second_weak(second_value);
  nuostl::NuoSwap(first_weak, second_weak);
  assert(first_weak.Lock().Get() == second_value.Get());
  assert(second_weak.Lock().Get() == first_value.Get());
}

void TestNuoSharedPtr::TestOutput()
{
  auto value = nuostl::NuoMakeShared<int>(1);
  std::ostringstream pointer_stream;
  std::ostringstream raw_stream;
  pointer_stream << value;
  raw_stream << value.Get();
  assert(pointer_stream.str() == raw_stream.str());

  nuostl::NuoSharedPtr<char[]> text(new char[6]{'h', 'e', 'l', 'l', 'o', 0});
  std::ostringstream text_stream;
  text_stream << text;
  assert(text_stream.str() == "hello");
}

void TestNuoSharedPtr::TestSuite()
{
  TestConstructionAndCounting();
  TestMoveSemantics();
  TestCustomDeleter();
  TestMakeShared();
  TestAliasing();
  TestUniquePtrConversion();
  TestArrays();
  TestWeakPtr();
  TestEnableSharedFromThis();
  TestCyclicReferences();
  TestCasts();
  TestComparisonsAndHash();
  TestOutput();
}

} /* namespace test */
