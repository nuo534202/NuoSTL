#include "core/memory/test_nuo_memory.hpp"

#include <assert.h>

#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_addressof.hpp"
#include "core/memory/nuo_uninitialized.hpp"
#include "utils/nuo_destroy.hpp"

namespace test
{

namespace
{

constexpr int kAddressofValue = 5;

static_assert(nuostl::NuoAddressof(kAddressofValue) == &kAddressofValue);

int AddOne(int value)
{
  return value + 1;
}

struct OverloadedAddress
{
  int* operator&() noexcept
  {
    return nullptr;
  }

  const int* operator&() const noexcept
  {
    return nullptr;
  }
};

struct Tracked
{
  inline static int constructions = 0;
  inline static int destructions = 0;
  inline static int copies = 0;
  inline static int moves = 0;

  explicit Tracked(int value) : value(value)
  {
    ++constructions;
  }

  Tracked(const Tracked& other) : value(other.value)
  {
    ++constructions;
    ++copies;
  }

  Tracked(Tracked&& other) noexcept : value(other.value)
  {
    ++constructions;
    ++moves;
    other.value = -1;
  }

  ~Tracked()
  {
    ++destructions;
  }

  int value;

  static void Reset()
  {
    constructions = 0;
    destructions = 0;
    copies = 0;
    moves = 0;
  }
};

struct MoveOnly
{
  inline static int constructions = 0;
  inline static int destructions = 0;

  explicit MoveOnly(int value) : value(value)
  {
    ++constructions;
  }

  MoveOnly(MoveOnly&& other) noexcept : value(other.value)
  {
    ++constructions;
    other.value = -1;
  }

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly& operator=(MoveOnly&&) = delete;

  ~MoveOnly()
  {
    ++destructions;
  }

  int value;

  static void Reset()
  {
    constructions = 0;
    destructions = 0;
  }
};

struct DefaultConstructed
{
  inline static int constructions = 0;
  inline static int destructions = 0;

  DefaultConstructed() : value(42)
  {
    ++constructions;
  }

  ~DefaultConstructed()
  {
    ++destructions;
  }

  int value;

  static void Reset()
  {
    constructions = 0;
    destructions = 0;
  }
};

struct ThrowingCopy
{
  inline static int constructions = 0;
  inline static int destructions = 0;
  inline static int copies_before_throw = 0;

  explicit ThrowingCopy(int value) : value(value)
  {
    ++constructions;
  }

  ThrowingCopy(const ThrowingCopy& other) : value(other.value)
  {
    if (copies_before_throw == 0)
    {
      throw std::runtime_error("copy failed");
    }
    --copies_before_throw;
    ++constructions;
  }

  ~ThrowingCopy()
  {
    ++destructions;
  }

  int value;

  static void Reset(int allowed_copies)
  {
    constructions = 0;
    destructions = 0;
    copies_before_throw = allowed_copies;
  }
};

struct ThrowingDefault
{
  inline static int constructions = 0;
  inline static int destructions = 0;
  inline static int allowed_constructions = 0;

  ThrowingDefault()
  {
    if (allowed_constructions == 0)
    {
      throw std::runtime_error("default construction failed");
    }
    --allowed_constructions;
    ++constructions;
  }

  ~ThrowingDefault()
  {
    ++destructions;
  }

  static void Reset(int allowed)
  {
    constructions = 0;
    destructions = 0;
    allowed_constructions = allowed;
  }
};

template <typename T, std::size_t Size>
struct Storage
{
  alignas(T) unsigned char bytes[Size * sizeof(T)];

  T* Data() noexcept
  {
    return reinterpret_cast<T*>(bytes);
  }
};

} /* namespace */

void TestNuoMemory::TestAddressof()
{
  OverloadedAddress object;
  assert((&object) == nullptr);
  assert(nuostl::NuoAddressof(object) != nullptr);
  assert(nuostl::NuoAddressof(object) == std::addressof(object));

  const OverloadedAddress const_object;
  assert(nuostl::NuoAddressof(const_object) == std::addressof(const_object));

  int number = 5;
  assert(nuostl::NuoAddressof(number) == &number);

  volatile int volatile_number = 6;
  assert(nuostl::NuoAddressof(volatile_number) == &volatile_number);

  assert(nuostl::NuoAddressof(AddOne) == &AddOne);
}

void TestNuoMemory::TestConstructAtAndDestroyAt()
{
  Tracked::Reset();
  Storage<Tracked, 1> storage;
  Tracked* object = nuostl::NuoConstructAt(storage.Data(), 7);
  assert(object->value == 7);
  assert(Tracked::constructions == 1);

  nuostl::NuoDestroyAt(object);
  assert(Tracked::destructions == 1);

  Tracked::Reset();
  Storage<Tracked, 2> array_storage;
  Tracked* first = array_storage.Data();
  nuostl::NuoConstructAt(first, 1);
  nuostl::NuoConstructAt(first + 1, 2);
  using TrackedArray = Tracked[2];
  TrackedArray* array = reinterpret_cast<TrackedArray*>(first);
  nuostl::NuoDestroyAt(array);
  assert(Tracked::destructions == 2);
}

void TestNuoMemory::TestDestroyN()
{
  Tracked::Reset();
  Storage<Tracked, 3> storage;
  Tracked* first = storage.Data();
  for (int index = 0; index < 3; ++index)
  {
    nuostl::NuoConstructAt(first + index, index);
  }

  Tracked* end = nuostl::NuoDestroyN(first, 3);
  assert(end == first + 3);
  assert(Tracked::constructions == 3);
  assert(Tracked::destructions == 3);

  assert(nuostl::NuoDestroyN(first, 0) == first);
}

void TestNuoMemory::TestUninitializedCopy()
{
  Tracked::Reset();
  Tracked source[3] = {Tracked{1}, Tracked{2}, Tracked{3}};
  Storage<Tracked, 3> storage;
  Tracked* destination = storage.Data();

  Tracked* end = nuostl::NuoUninitializedCopy(
    source, source + 3, destination);
  assert(end == destination + 3);
  assert(destination[0].value == 1);
  assert(destination[1].value == 2);
  assert(destination[2].value == 3);
  assert(Tracked::copies == 3);

  nuostl::NuoDestroy(destination, end);
  assert(Tracked::destructions == 3);

  Tracked* unchanged = nuostl::NuoUninitializedCopy(
    source, source, destination);
  assert(unchanged == destination);
}

void TestNuoMemory::TestUninitializedCopyN()
{
  Tracked::Reset();
  Tracked source[3] = {Tracked{4}, Tracked{5}, Tracked{6}};
  Storage<Tracked, 3> storage;
  Tracked* destination = storage.Data();

  Tracked* end = nuostl::NuoUninitializedCopyN(source, 2, destination);
  assert(end == destination + 2);
  assert(destination[0].value == 4);
  assert(destination[1].value == 5);
  assert(Tracked::copies == 2);

  nuostl::NuoDestroy(destination, end);
  assert(Tracked::destructions == 2);

  assert(nuostl::NuoUninitializedCopyN(source, 0, destination) ==
         destination);
}

void TestNuoMemory::TestUninitializedFill()
{
  Tracked::Reset();
  Storage<Tracked, 4> storage;
  Tracked* first = storage.Data();
  nuostl::NuoUninitializedFill(first, first + 4, Tracked(9));

  for (int index = 0; index < 4; ++index)
  {
    assert(first[index].value == 9);
  }
  assert(Tracked::copies == 4);

  nuostl::NuoDestroy(first, first + 4);
  assert(Tracked::destructions == 5);
}

void TestNuoMemory::TestUninitializedFillN()
{
  Tracked::Reset();
  Storage<Tracked, 4> storage;
  Tracked* first = storage.Data();
  Tracked* end = nuostl::NuoUninitializedFillN(first, 3, Tracked(11));

  assert(end == first + 3);
  for (int index = 0; index < 3; ++index)
  {
    assert(first[index].value == 11);
  }
  assert(Tracked::copies == 3);
  assert(nuostl::NuoUninitializedFillN(first, 0, Tracked(0)) == first);

  nuostl::NuoDestroy(first, end);
  assert(Tracked::destructions == 5);
}

void TestNuoMemory::TestUninitializedMove()
{
  MoveOnly::Reset();
  MoveOnly source[2] = {MoveOnly(1), MoveOnly(2)};
  Storage<MoveOnly, 2> storage;
  MoveOnly* destination = storage.Data();

  MoveOnly* end = nuostl::NuoUninitializedMove(
    source, source + 2, destination);
  assert(end == destination + 2);
  assert(destination[0].value == 1);
  assert(destination[1].value == 2);
  assert(source[0].value == -1);
  assert(source[1].value == -1);

  nuostl::NuoDestroy(destination, end);
}

void TestNuoMemory::TestUninitializedMoveN()
{
  MoveOnly::Reset();
  MoveOnly source[2] = {MoveOnly(3), MoveOnly(4)};
  Storage<MoveOnly, 2> storage;
  MoveOnly* destination = storage.Data();

  auto result = nuostl::NuoUninitializedMoveN(source, 1, destination);
  assert(result.first == source + 1);
  assert(result.second == destination + 1);
  assert(destination[0].value == 3);
  assert(source[0].value == -1);

  nuostl::NuoDestroy(destination, result.second);
  auto empty = nuostl::NuoUninitializedMoveN(source, 0, destination);
  assert(empty.first == source);
  assert(empty.second == destination);
}

void TestNuoMemory::TestUninitializedDefaultConstruct()
{
  DefaultConstructed::Reset();
  Storage<DefaultConstructed, 4> storage;
  DefaultConstructed* first = storage.Data();
  nuostl::NuoUninitializedDefaultConstruct(first, first + 4);

  assert(DefaultConstructed::constructions == 4);
  for (int index = 0; index < 4; ++index)
  {
    assert(first[index].value == 42);
  }
  nuostl::NuoDestroy(first, first + 4);
  assert(DefaultConstructed::destructions == 4);

  DefaultConstructed* end = nuostl::NuoUninitializedDefaultConstructN(
    first, 2);
  assert(end == first + 2);
  nuostl::NuoDestroy(first, end);
}

void TestNuoMemory::TestUninitializedValueConstruct()
{
  Storage<int, 4> storage;
  int* first = storage.Data();
  nuostl::NuoUninitializedValueConstruct(first, first + 4);

  for (int index = 0; index < 4; ++index)
  {
    assert(first[index] == 0);
  }
  nuostl::NuoDestroy(first, first + 4);

  int* end = nuostl::NuoUninitializedValueConstructN(first, 3);
  assert(end == first + 3);
  for (int index = 0; index < 3; ++index)
  {
    assert(first[index] == 0);
  }
  nuostl::NuoDestroy(first, end);
}

void TestNuoMemory::TestExceptionSafety()
{
  ThrowingCopy::Reset(2);
  ThrowingCopy source[3] = {ThrowingCopy{1}, ThrowingCopy{2}, ThrowingCopy{3}};
  Storage<ThrowingCopy, 3> storage;
  ThrowingCopy* destination = storage.Data();

  bool caught = false;
  try
  {
    nuostl::NuoUninitializedCopy(source, source + 3, destination);
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }
  assert(caught);
  assert(ThrowingCopy::destructions == 2);
  assert(ThrowingCopy::constructions == 5);

  ThrowingDefault::Reset(2);
  Storage<ThrowingDefault, 3> default_storage;
  caught = false;
  try
  {
    nuostl::NuoUninitializedDefaultConstruct(
      default_storage.Data(), default_storage.Data() + 3);
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }
  assert(caught);
  assert(ThrowingDefault::constructions == 2);
  assert(ThrowingDefault::destructions == 2);
}

void TestNuoMemory::TestConstexprAddressof()
{
  static_assert(std::is_same_v<decltype(nuostl::NuoAddressof(
                                 kAddressofValue)),
                               const int*>);
  static_assert(std::is_same_v<decltype(nuostl::NuoAddressof(AddOne)),
                               int (*)(int)>);
}

void TestNuoMemory::TestSuite()
{
  TestAddressof();
  TestConstructAtAndDestroyAt();
  TestDestroyN();
  TestUninitializedCopy();
  TestUninitializedCopyN();
  TestUninitializedFill();
  TestUninitializedFillN();
  TestUninitializedMove();
  TestUninitializedMoveN();
  TestUninitializedDefaultConstruct();
  TestUninitializedValueConstruct();
  TestExceptionSafety();
  TestConstexprAddressof();
}

} /* namespace test */
