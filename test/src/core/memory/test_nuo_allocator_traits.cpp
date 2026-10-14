#include "core/memory/test_nuo_allocator_traits.hpp"

#include <assert.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_allocator_traits.hpp"
#include "core/memory/nuo_owner_less.hpp"
#include "core/memory/nuo_pointer_traits.hpp"
#include "core/memory/nuo_shared_ptr.hpp"
#include "utils/nuo_allocator.hpp"
#include "utils/nuo_pool_allocator.hpp"

namespace test
{

namespace
{

template <typename T>
class FancyPointer
{
public:
  using element_type = T;
  using difference_type = long;

  template <typename U>
  using rebind = FancyPointer<U>;

  FancyPointer() = default;

  explicit FancyPointer(T* pointer) : pointer_(pointer)
  {
  }

  T* Get() const noexcept
  {
    return pointer_;
  }

  T* operator->() const noexcept
  {
    return pointer_;
  }

  T& operator*() const noexcept
  {
    return *pointer_;
  }

  static FancyPointer pointer_to(T& reference) noexcept
  {
    return FancyPointer(std::addressof(reference));
  }

private:
  T* pointer_ = nullptr;
};

template <typename T>
struct MinimalAllocator
{
  using value_type = T;

  T* allocate(std::size_t count)
  {
    return static_cast<T*>(::operator new(count * sizeof(T)));
  }

  void deallocate(T* pointer, std::size_t) noexcept
  {
    ::operator delete(pointer);
  }
};

template <typename T>
struct FancyAllocator
{
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
  using is_always_equal = std::false_type;

  inline static int construct_calls = 0;
  inline static int destroy_calls = 0;
  inline static int max_size_calls = 0;
  inline static int select_calls = 0;
  inline static const void* last_hint = nullptr;

  FancyAllocator() = default;

  explicit FancyAllocator(int state) : state(state)
  {
  }

  template <typename U>
  FancyAllocator(const FancyAllocator<U>& other) : state(other.state)
  {
  }

  pointer allocate(size_type count)
  {
    return static_cast<pointer>(::operator new(count * sizeof(T)));
  }

  pointer allocate(size_type count, const_void_pointer hint)
  {
    last_hint = hint;
    return allocate(count);
  }

  void deallocate(pointer pointer, size_type) noexcept
  {
    ::operator delete(pointer);
  }

  template <typename U, typename... Args>
  void construct(U* pointer, Args&&... args)
  {
    ++construct_calls;
    nuostl::NuoConstructAt(pointer, std::forward<Args>(args)...);
  }

  template <typename U>
  void destroy(U* pointer)
  {
    ++destroy_calls;
    nuostl::NuoDestroyAt(pointer);
  }

  size_type max_size() const noexcept
  {
    ++max_size_calls;
    return 42;
  }

  FancyAllocator select_on_container_copy_construction() const
  {
    ++select_calls;
    return FancyAllocator(state + 1);
  }

  int state = 0;
};

template <typename T, int Tag = 0>
struct TaggedAllocator
{
  using value_type = T;

  template <typename U>
  struct rebind
  {
    using other = TaggedAllocator<U, Tag>;
  };

  T* allocate(std::size_t count)
  {
    return static_cast<T*>(::operator new(count * sizeof(T)));
  }

  void deallocate(T* pointer, std::size_t) noexcept
  {
    ::operator delete(pointer);
  }
};

struct PairValue
{
  int first = 1;
  int second = 2;
};

} /* namespace */

void TestNuoAllocatorTraits::TestPointerTraits()
{
  static_assert(std::is_same_v<nuostl::NuoPointerTraits<int*>::element_type,
                               int>);
  static_assert(std::is_same_v<
    nuostl::NuoPointerTraits<int*>::difference_type,
    std::ptrdiff_t>);
  static_assert(std::is_same_v<
    nuostl::NuoPointerTraits<int*>::rebind<const int>,
    const int*>);

  int value = 5;
  assert(nuostl::NuoPointerTraits<int*>::pointer_to(value) == &value);

  using Fancy = FancyPointer<int>;
  static_assert(std::is_same_v<nuostl::NuoPointerTraits<Fancy>::element_type,
                               int>);
  static_assert(std::is_same_v<
    nuostl::NuoPointerTraits<Fancy>::difference_type,
    long>);
  static_assert(std::is_same_v<
    nuostl::NuoPointerTraits<Fancy>::rebind<double>,
    FancyPointer<double>>);
  assert(nuostl::NuoPointerTraits<Fancy>::pointer_to(value).Get() == &value);
}

void TestNuoAllocatorTraits::TestToAddress()
{
  int value = 5;
  int* raw = &value;
  assert(nuostl::NuoToAddress(raw) == raw);
  assert(nuostl::NuoToAddress(static_cast<int*>(nullptr)) == nullptr);

  FancyPointer<int> fancy(&value);
  assert(nuostl::NuoToAddress(fancy) == &value);

  auto shared = nuostl::NuoMakeShared<int>(7);
  assert(nuostl::NuoToAddress(shared) == shared.Get());
}

void TestNuoAllocatorTraits::TestAllocatorTraitsTypes()
{
  using Allocator = nuostl::NuoAllocator<int>;
  using Traits = nuostl::NuoAllocatorTraits<Allocator>;

  static_assert(std::is_same_v<Traits::allocator_type, Allocator>);
  static_assert(std::is_same_v<Traits::value_type, int>);
  static_assert(std::is_same_v<Traits::pointer, int*>);
  static_assert(std::is_same_v<Traits::const_pointer, const int*>);
  static_assert(std::is_same_v<Traits::void_pointer, void*>);
  static_assert(std::is_same_v<Traits::const_void_pointer, const void*>);
  static_assert(std::is_same_v<Traits::difference_type, std::ptrdiff_t>);
  static_assert(std::is_same_v<Traits::size_type, std::size_t>);
  static_assert(Traits::propagate_on_container_move_assignment::value);
  static_assert(Traits::is_always_equal::value);
  static_assert(!Traits::propagate_on_container_copy_assignment::value);
  static_assert(!Traits::propagate_on_container_swap::value);
  static_assert(std::is_same_v<Traits::rebind_alloc<double>,
                               nuostl::NuoAllocator<double>>);
  static_assert(std::is_same_v<
    Traits::rebind_traits<double>,
    nuostl::NuoAllocatorTraits<nuostl::NuoAllocator<double>>>);

  static_assert(std::is_same_v<
    std::allocator_traits<Allocator>::rebind_alloc<double>,
    nuostl::NuoAllocator<double>>);
  static_assert(std::allocator_traits<Allocator>::is_always_equal::value);

  using PoolTraits = nuostl::NuoAllocatorTraits<
    nuostl::NuoPoolAllocator<int, 16>>;
  static_assert(std::is_same_v<PoolTraits::pointer, int*>);
  static_assert(PoolTraits::is_always_equal::value);
  static_assert(PoolTraits::propagate_on_container_move_assignment::value);
  static_assert(std::is_same_v<
    PoolTraits::rebind_alloc<double>,
    nuostl::NuoPoolAllocator<double, 16>>);

  using MinimalTraits = nuostl::NuoAllocatorTraits<MinimalAllocator<int>>;
  static_assert(std::is_same_v<MinimalTraits::pointer, int*>);
  static_assert(std::is_same_v<MinimalTraits::const_pointer, const int*>);
  static_assert(std::is_same_v<MinimalTraits::void_pointer, void*>);
  static_assert(std::is_same_v<MinimalTraits::const_void_pointer,
                               const void*>);
  static_assert(std::is_same_v<MinimalTraits::difference_type,
                               std::ptrdiff_t>);
  static_assert(std::is_same_v<MinimalTraits::size_type, std::size_t>);
  static_assert(!MinimalTraits::propagate_on_container_copy_assignment::value);
  static_assert(!MinimalTraits::propagate_on_container_move_assignment::value);
  static_assert(!MinimalTraits::propagate_on_container_swap::value);
  static_assert(MinimalTraits::is_always_equal::value);
  static_assert(std::is_same_v<MinimalTraits::rebind_alloc<long>,
                               MinimalAllocator<long>>);

  using FancyTraits = nuostl::NuoAllocatorTraits<FancyAllocator<int>>;
  static_assert(FancyTraits::propagate_on_container_copy_assignment::value);
  static_assert(FancyTraits::propagate_on_container_move_assignment::value);
  static_assert(FancyTraits::propagate_on_container_swap::value);
  static_assert(!FancyTraits::is_always_equal::value);
  static_assert(std::is_same_v<FancyTraits::rebind_alloc<double>,
                               FancyAllocator<double>>);

  using TaggedTraits = nuostl::NuoAllocatorTraits<TaggedAllocator<int, 3>>;
  static_assert(std::is_same_v<TaggedTraits::rebind_alloc<double>,
                               TaggedAllocator<double, 3>>);
}

void TestNuoAllocatorTraits::TestAllocatorTraitsOperations()
{
  using Allocator = nuostl::NuoAllocator<int>;
  using Traits = nuostl::NuoAllocatorTraits<Allocator>;
  Allocator allocator;

  int* values = Traits::allocate(allocator, 3);
  Traits::construct(allocator, values, 42);
  assert(*values == 42);
  Traits::destroy(allocator, values);
  Traits::deallocate(allocator, values, 3);
  assert(Traits::max_size(allocator) ==
         static_cast<std::size_t>(-1) / sizeof(int));
  Allocator selected = Traits::select_on_container_copy_construction(allocator);
  assert(selected == allocator);

  using MinimalTraits = nuostl::NuoAllocatorTraits<MinimalAllocator<int>>;
  MinimalAllocator<int> minimal;
  int* minimal_values = MinimalTraits::allocate(minimal, 2);
  MinimalTraits::construct(minimal, minimal_values, 7);
  assert(*minimal_values == 7);
  MinimalTraits::destroy(minimal, minimal_values);
  MinimalTraits::deallocate(minimal, minimal_values, 2);
  assert(MinimalTraits::max_size(minimal) ==
         std::numeric_limits<std::size_t>::max() / sizeof(int));
  int* hinted = MinimalTraits::allocate(
    minimal, 1, static_cast<const void*>(nullptr));
  MinimalTraits::deallocate(minimal, hinted, 1);
  MinimalAllocator<int> minimal_copy =
    MinimalTraits::select_on_container_copy_construction(minimal);
  (void)minimal_copy;

  FancyAllocator<int>::construct_calls = 0;
  FancyAllocator<int>::destroy_calls = 0;
  FancyAllocator<int>::max_size_calls = 0;
  FancyAllocator<int>::select_calls = 0;
  FancyAllocator<int>::last_hint = nullptr;

  using FancyTraits = nuostl::NuoAllocatorTraits<FancyAllocator<int>>;
  FancyAllocator<int> fancy(7);
  int* fancy_value = FancyTraits::allocate(fancy, 1);
  FancyTraits::construct(fancy, fancy_value, 9);
  assert(*fancy_value == 9);
  assert(FancyAllocator<int>::construct_calls == 1);
  FancyTraits::destroy(fancy, fancy_value);
  assert(FancyAllocator<int>::destroy_calls == 1);
  FancyTraits::deallocate(fancy, fancy_value, 1);

  int hint = 0;
  int* fancy_hinted = FancyTraits::allocate(fancy, 1, &hint);
  assert(fancy_hinted != nullptr);
  assert(FancyAllocator<int>::last_hint == &hint);
  FancyTraits::deallocate(fancy, fancy_hinted, 1);

  assert(FancyTraits::max_size(fancy) == 42);
  assert(FancyAllocator<int>::max_size_calls == 1);

  FancyAllocator<int> fancy_selected =
    FancyTraits::select_on_container_copy_construction(fancy);
  assert(fancy_selected.state == 8);
  assert(FancyAllocator<int>::select_calls == 1);
}

void TestNuoAllocatorTraits::TestOwnerLess()
{
  auto first = nuostl::NuoMakeShared<int>(1);
  auto first_alias = nuostl::NuoSharedPtr<int>(first, first.Get());
  auto second = nuostl::NuoMakeShared<int>(2);
  nuostl::NuoSharedPtr<int> empty;

  nuostl::NuoOwnerLess<void> less;
  assert(!less(first, first));
  assert(!less(first, first_alias));
  assert(!less(first_alias, first));
  assert(less(first, second) != less(second, first));
  assert(less(empty, first));
  assert(!less(first, empty));
  assert(!less(empty, empty));

  nuostl::NuoWeakPtr<int> weak_first(first);
  nuostl::NuoWeakPtr<int> weak_second(second);
  assert(!less(first, weak_first));
  assert(!less(weak_first, first));
  assert(less(weak_first, weak_second) != less(weak_second, weak_first));
  assert(less(empty, weak_first));
  assert(!less(weak_first, empty));

  auto integer = nuostl::NuoMakeShared<int>(1);
  auto real = nuostl::NuoMakeShared<double>(1.0);
  assert(less(integer, real) != less(real, integer));

  auto pair = nuostl::NuoMakeShared<PairValue>();
  nuostl::NuoSharedPtr<int> alias(pair, &pair->first);
  assert(!less(pair, alias));
  assert(!less(alias, pair));

  nuostl::NuoOwnerLess<int> typed_less;
  assert(!typed_less(first, first));
  assert(typed_less(first, second) != typed_less(second, first));
  nuostl::NuoWeakPtr<int> typed_weak(first);
  assert(!typed_less(first, typed_weak));
  assert(!typed_less(typed_weak, first));
}

void TestNuoAllocatorTraits::TestSuite()
{
  TestPointerTraits();
  TestToAddress();
  TestAllocatorTraitsTypes();
  TestAllocatorTraitsOperations();
  TestOwnerLess();
}

} /* namespace test */
