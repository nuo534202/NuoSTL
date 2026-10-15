#include "core/memory/test_nuo_allocate_shared.hpp"

#include <assert.h>

#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

#include "core/memory/nuo_shared_ptr.hpp"
#include "utils/nuo_allocator.hpp"

namespace test
{

namespace
{

struct AllocatorState
{
  inline static int allocations = 0;
  inline static int deallocations = 0;
  inline static int constructs = 0;
  inline static int destroys = 0;

  static void Reset()
  {
    allocations = 0;
    deallocations = 0;
    constructs = 0;
    destroys = 0;
  }
};

template <typename T>
struct TrackingAllocator
{
  using value_type = T;

  template <typename U>
  struct rebind
  {
    using other = TrackingAllocator<U>;
  };

  TrackingAllocator() = default;

  template <typename U>
  TrackingAllocator(const TrackingAllocator<U>&) noexcept
  {
  }

  T* allocate(std::size_t count)
  {
    ++AllocatorState::allocations;
    return static_cast<T*>(::operator new(count * sizeof(T)));
  }

  void deallocate(T* pointer, std::size_t) noexcept
  {
    ++AllocatorState::deallocations;
    ::operator delete(pointer);
  }

  template <typename U, typename... Args>
  void construct(U* pointer, Args&&... args)
  {
    ++AllocatorState::constructs;
    nuostl::NuoConstructAt(pointer, std::forward<Args>(args)...);
  }

  template <typename U>
  void destroy(U* pointer)
  {
    ++AllocatorState::destroys;
    nuostl::NuoDestroyAt(pointer);
  }

  friend bool operator==(const TrackingAllocator&,
                         const TrackingAllocator&) noexcept
  {
    return true;
  }

  friend bool operator!=(const TrackingAllocator&,
                         const TrackingAllocator&) noexcept
  {
    return false;
  }
};

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

struct Self : nuostl::NuoEnableSharedFromThis<Self>
{
  int value = 5;
};

} /* namespace */

void TestNuoAllocateShared::TestBasicAllocation()
{
  AllocatorState::Reset();
  {
    auto pointer =
      nuostl::NuoAllocateShared<int>(TrackingAllocator<int>(), 42);
    assert(pointer.UseCount() == 1);
    assert(*pointer == 42);
    assert(AllocatorState::allocations == 1);
    assert(AllocatorState::constructs == 1);
    assert(AllocatorState::deallocations == 0);
    assert(AllocatorState::destroys == 0);

    auto copy = pointer;
    assert(pointer.UseCount() == 2);
    assert(copy.Get() == pointer.Get());
    assert(AllocatorState::allocations == 1);
  }
  assert(AllocatorState::deallocations == 1);
  assert(AllocatorState::destroys == 1);
}

void TestNuoAllocateShared::TestWeakLifetime()
{
  AllocatorState::Reset();
  {
    auto pointer =
      nuostl::NuoAllocateShared<int>(TrackingAllocator<int>(), 7);
    nuostl::NuoWeakPtr<int> weak(pointer);
    assert(weak.UseCount() == 1);

    pointer.Reset();
    assert(weak.Expired());
    assert(AllocatorState::destroys == 1);
    assert(AllocatorState::deallocations == 0);

    weak.Reset();
    assert(AllocatorState::deallocations == 1);
  }
}

void TestNuoAllocateShared::TestThrowingConstruction()
{
  AllocatorState::Reset();
  bool caught = false;
  try
  {
    auto pointer = nuostl::NuoAllocateShared<ThrowOnConstruction>(
      TrackingAllocator<ThrowOnConstruction>(), true);
    (void)pointer;
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }
  assert(caught);
  assert(AllocatorState::allocations == 1);
  assert(AllocatorState::deallocations == 1);
  assert(AllocatorState::constructs == 1);
  assert(AllocatorState::destroys == 0);
}

void TestNuoAllocateShared::TestEnableSharedFromThis()
{
  AllocatorState::Reset();
  {
    auto pointer =
      nuostl::NuoAllocateShared<Self>(TrackingAllocator<Self>());
    auto from_this = pointer->SharedFromThis();
    assert(from_this.Get() == pointer.Get());
    assert(pointer.UseCount() == 2);
    assert(AllocatorState::allocations == 1);
  }
  assert(AllocatorState::deallocations == 1);
  assert(AllocatorState::destroys == 1);
}

void TestNuoAllocateShared::TestOtherAllocators()
{
  auto default_allocator = nuostl::NuoAllocateShared<int>(
    std::allocator<int>(), 11);
  assert(default_allocator.UseCount() == 1);
  assert(*default_allocator == 11);

  auto nuo_allocator = nuostl::NuoAllocateShared<int>(
    nuostl::NuoAllocator<int>(), 12);
  assert(nuo_allocator.UseCount() == 1);
  assert(*nuo_allocator == 12);
}

void TestNuoAllocateShared::TestStdParity()
{
  AllocatorState::Reset();
  {
    std::shared_ptr<int> pointer =
      std::allocate_shared<int>(TrackingAllocator<int>(), 5);
    assert(*pointer == 5);
  }
  const int standard_allocations = AllocatorState::allocations;
  const int standard_deallocations = AllocatorState::deallocations;
  const int standard_constructs = AllocatorState::constructs;
  const int standard_destroys = AllocatorState::destroys;

  AllocatorState::Reset();
  {
    auto pointer =
      nuostl::NuoAllocateShared<int>(TrackingAllocator<int>(), 5);
    assert(*pointer == 5);
  }
  assert(standard_allocations == AllocatorState::allocations);
  assert(standard_deallocations == AllocatorState::deallocations);
  assert(standard_constructs == AllocatorState::constructs);
  assert(standard_destroys == AllocatorState::destroys);
}

void TestNuoAllocateShared::TestSuite()
{
  TestBasicAllocation();
  TestWeakLifetime();
  TestThrowingConstruction();
  TestEnableSharedFromThis();
  TestOtherAllocators();
  TestStdParity();
}

} /* namespace test */
