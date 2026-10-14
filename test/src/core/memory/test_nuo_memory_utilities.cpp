#include "core/memory/test_nuo_memory_utilities.hpp"

#include <assert.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#include "core/memory/nuo_align.hpp"
#include "core/memory/nuo_assume_aligned.hpp"
#include "core/memory/nuo_launder.hpp"
#include "core/memory/nuo_uninitialized.hpp"
#include "core/memory/nuo_uses_allocator.hpp"

namespace test
{

namespace
{

struct ConstMember
{
  const int value;
};

struct WithoutAllocator
{
};

struct WithAllocator
{
  using allocator_type = std::allocator<int>;
};

struct UnrelatedAllocator
{
};

template <std::size_t Alignment, std::size_t Size>
void CheckAlign()
{
  alignas(Alignment) unsigned char standard_buffer[256];
  alignas(Alignment) unsigned char nuo_buffer[256];
  void* standard_pointer = standard_buffer + 1;
  void* nuo_pointer = nuo_buffer + 1;
  std::size_t standard_space = sizeof(standard_buffer) - 1;
  std::size_t nuo_space = sizeof(nuo_buffer) - 1;

  void* standard_result = std::align(
    Alignment, Size, standard_pointer, standard_space);
  void* nuo_result = nuostl::NuoAlign(
    Alignment, Size, nuo_pointer, nuo_space);

  assert((standard_result == nullptr) == (nuo_result == nullptr));
  if (standard_result != nullptr)
  {
    assert(reinterpret_cast<std::uintptr_t>(nuo_result) % Alignment == 0);
    assert(standard_space == nuo_space);
    assert(static_cast<unsigned char*>(standard_result) - standard_buffer ==
           static_cast<unsigned char*>(nuo_result) - nuo_buffer);
  }
  else
  {
    assert(standard_pointer == standard_buffer + 1);
    assert(nuo_pointer == nuo_buffer + 1);
    assert(standard_space == sizeof(standard_buffer) - 1);
    assert(nuo_space == sizeof(nuo_buffer) - 1);
  }
}

} /* namespace */

void TestNuoMemoryUtilities::TestAlign()
{
  CheckAlign<1, 0>();
  CheckAlign<1, 100>();
  CheckAlign<2, 255>();
  CheckAlign<4, 253>();
  CheckAlign<8, 1>();
  CheckAlign<8, 200>();
  CheckAlign<16, 240>();
  CheckAlign<64, 128>();
  CheckAlign<64, 200>();
  CheckAlign<128, 128>();
  CheckAlign<256, 1>();

  alignas(16) unsigned char buffer[32];
  void* pointer = buffer + 3;
  std::size_t space = sizeof(buffer) - 3;
  void* aligned = nuostl::NuoAlign(16, 16, pointer, space);
  assert(aligned != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(aligned) % 16 == 0);
  assert(space >= 16);

  void* failing_pointer = buffer + 1;
  std::size_t failing_space = 8;
  assert(nuostl::NuoAlign(16, 16, failing_pointer, failing_space) == nullptr);
  assert(failing_pointer == buffer + 1);
  assert(failing_space == 8);
}

void TestNuoMemoryUtilities::TestAlignVal()
{
  static_assert(sizeof(nuostl::NuoAlignValT) == sizeof(std::size_t));
  static_assert(static_cast<std::size_t>(nuostl::NuoAlignValT{}) == 0);
  nuostl::NuoAlignValT alignment =
    static_cast<nuostl::NuoAlignValT>(16);
  assert(static_cast<std::size_t>(alignment) == 16);
}

void TestNuoMemoryUtilities::TestLaunder()
{
  static int static_value = 5;
  static_assert(nuostl::NuoLaunder(&static_value) == &static_value);
  static_assert(std::launder(&static_value) == &static_value);

  int value = 5;
  assert(nuostl::NuoLaunder(&value) == &value);

  alignas(ConstMember) unsigned char storage[sizeof(ConstMember)];
  ConstMember* first = nuostl::NuoConstructAt(
    reinterpret_cast<ConstMember*>(storage), ConstMember{1});
  assert(first->value == 1);
  first->~ConstMember();

  ConstMember* second = nuostl::NuoConstructAt(
    reinterpret_cast<ConstMember*>(storage), ConstMember{2});
  ConstMember* laundered = nuostl::NuoLaunder(second);
  assert(laundered->value == 2);
  laundered->~ConstMember();
}

void TestNuoMemoryUtilities::TestAssumeAligned()
{
  alignas(64) int values[4] = {1, 2, 3, 4};
  int* standard = std::assume_aligned<64>(values);
  int* nuo = nuostl::NuoAssumeAligned<64>(values);
  assert(standard == values);
  assert(nuo == values);
  assert(nuo[0] == 1 && nuo[3] == 4);

  static_assert(std::assume_aligned<8>(static_cast<int*>(nullptr)) ==
                nullptr);
  static_assert(nuostl::NuoAssumeAligned<8>(static_cast<int*>(nullptr)) ==
                nullptr);
}

void TestNuoMemoryUtilities::TestUsesAllocator()
{
  static_assert(!nuostl::NuoUsesAllocatorV<WithoutAllocator,
                                           std::allocator<int>>);
  static_assert(nuostl::NuoUsesAllocatorV<WithAllocator,
                                          std::allocator<int>>);
  static_assert(
    nuostl::NuoUsesAllocatorV<WithAllocator, std::allocator<double>> ==
    std::uses_allocator_v<WithAllocator, std::allocator<double>>);
  static_assert(!nuostl::NuoUsesAllocatorV<WithAllocator,
                                           UnrelatedAllocator>);
  static_assert(nuostl::NuoUsesAllocatorV<std::vector<int>,
                                          std::allocator<int>>);

  static_assert(
    nuostl::NuoUsesAllocatorV<WithAllocator, std::allocator<int>> ==
    std::uses_allocator_v<WithAllocator, std::allocator<int>>);
  static_assert(
    nuostl::NuoUsesAllocatorV<WithoutAllocator, std::allocator<int>> ==
    std::uses_allocator_v<WithoutAllocator, std::allocator<int>>);
  static_assert(
    nuostl::NuoUsesAllocatorV<std::vector<int>, std::allocator<int>> ==
    std::uses_allocator_v<std::vector<int>, std::allocator<int>>);
  static_assert(
    nuostl::NuoUsesAllocatorV<WithAllocator, UnrelatedAllocator> ==
    std::uses_allocator_v<WithAllocator, UnrelatedAllocator>);
}

void TestNuoMemoryUtilities::TestAllocatorArg()
{
  static_assert(std::is_empty_v<nuostl::NuoAllocatorArgT>);
  static_assert(std::is_same_v<decltype(nuostl::NuoAllocatorArg),
                               const nuostl::NuoAllocatorArgT>);
  static_assert(std::is_same_v<decltype(std::allocator_arg),
                               const std::allocator_arg_t>);

  nuostl::NuoAllocatorArgT tag;
  (void)tag;
  std::allocator_arg_t standard_tag;
  (void)standard_tag;
}

void TestNuoMemoryUtilities::TestSuite()
{
  TestAlign();
  TestAlignVal();
  TestLaunder();
  TestAssumeAligned();
  TestUsesAllocator();
  TestAllocatorArg();
}

} /* namespace test */
