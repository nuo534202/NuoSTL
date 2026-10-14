#include "core/memory/test_nuo_uses_allocator_construction.hpp"

#include <assert.h>

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_uninitialized.hpp"
#include "core/memory/nuo_uses_allocator.hpp"
#include "core/memory/nuo_uses_allocator_construction.hpp"

namespace test
{

namespace
{

struct NoAllocator
{
  explicit NoAllocator(int value) : value(value)
  {
  }

  int value;
};

struct LeadingNuo
{
  using allocator_type = std::allocator<int>;

  LeadingNuo(nuostl::NuoAllocatorArgT, allocator_type allocator, int value)
    : value(value)
    , allocator(allocator)
  {
  }

  int value = 0;
  allocator_type allocator;
};

struct LeadingStd
{
  using allocator_type = std::allocator<int>;

  LeadingStd(std::allocator_arg_t, allocator_type allocator, int value)
    : value(value)
    , allocator(allocator)
  {
  }

  int value = 0;
  allocator_type allocator;
};

struct TrailingAllocator
{
  using allocator_type = std::allocator<int>;

  TrailingAllocator(int value, allocator_type allocator)
    : value(value)
    , allocator(allocator)
  {
  }

  int value = 0;
  allocator_type allocator;
};

struct BothConventions
{
  using allocator_type = std::allocator<int>;

  BothConventions(nuostl::NuoAllocatorArgT, allocator_type allocator, int value)
    : value(value)
    , allocator(allocator)
    , convention(1)
  {
  }

  BothConventions(int value, allocator_type allocator)
    : value(value)
    , allocator(allocator)
    , convention(2)
  {
  }

  int value = 0;
  allocator_type allocator;
  int convention = 0;
};

struct DefaultLeadingNuo
{
  using allocator_type = std::allocator<int>;

  DefaultLeadingNuo() = default;

  DefaultLeadingNuo(nuostl::NuoAllocatorArgT, allocator_type allocator)
    : allocator(allocator)
  {
  }

  allocator_type allocator;
};

struct DefaultLeadingStd
{
  using allocator_type = std::allocator<int>;

  DefaultLeadingStd() = default;

  DefaultLeadingStd(std::allocator_arg_t, allocator_type allocator)
    : allocator(allocator)
  {
  }

  allocator_type allocator;
};

} /* namespace */

void TestNuoUsesAllocatorConstruction::TestGeneralConstruction()
{
  std::allocator<int> allocator;

  NoAllocator no_allocator =
    nuostl::NuoMakeObjUsingAllocator<NoAllocator>(allocator, 5);
  NoAllocator std_no_allocator =
    std::make_obj_using_allocator<NoAllocator>(allocator, 5);
  assert(no_allocator.value == 5);
  assert(no_allocator.value == std_no_allocator.value);

  LeadingNuo leading =
    nuostl::NuoMakeObjUsingAllocator<LeadingNuo>(allocator, 6);
  assert(leading.value == 6);
  assert(leading.allocator == allocator);

  LeadingStd std_leading =
    nuostl::NuoMakeObjUsingAllocator<LeadingStd>(allocator, 7);
  LeadingStd standard_leading =
    std::make_obj_using_allocator<LeadingStd>(allocator, 7);
  assert(std_leading.value == 7);
  assert(std_leading.value == standard_leading.value);

  TrailingAllocator trailing =
    nuostl::NuoMakeObjUsingAllocator<TrailingAllocator>(allocator, 8);
  TrailingAllocator std_trailing =
    std::make_obj_using_allocator<TrailingAllocator>(allocator, 8);
  assert(trailing.value == 8);
  assert(trailing.value == std_trailing.value);

  BothConventions both =
    nuostl::NuoMakeObjUsingAllocator<BothConventions>(allocator, 9);
  assert(both.value == 9);
  assert(both.convention == 1);
}

void TestNuoUsesAllocatorConstruction::TestTupleShapes()
{
  std::allocator<int> allocator;

  using NoAllocatorTuple =
    decltype(nuostl::NuoUsesAllocatorConstructionArgs<NoAllocator>(
      allocator, 5));
  static_assert(std::is_same_v<NoAllocatorTuple, std::tuple<int&&>>);

  using LeadingTuple =
    decltype(nuostl::NuoUsesAllocatorConstructionArgs<LeadingNuo>(
      allocator, 6));
  static_assert(std::is_same_v<
    LeadingTuple,
    std::tuple<nuostl::NuoAllocatorArgT, const std::allocator<int>&,
               int&&>>);

  using StdLeadingTuple =
    decltype(nuostl::NuoUsesAllocatorConstructionArgs<LeadingStd>(
      allocator, 7));
  static_assert(std::is_same_v<
    StdLeadingTuple,
    std::tuple<std::allocator_arg_t, const std::allocator<int>&, int&&>>);

  using TrailingTuple =
    decltype(nuostl::NuoUsesAllocatorConstructionArgs<TrailingAllocator>(
      allocator, 8));
  static_assert(std::is_same_v<
    TrailingTuple,
    std::tuple<int&&, const std::allocator<int>&>>);

  int leading_value = 6;
  auto leading_tuple = nuostl::NuoUsesAllocatorConstructionArgs<LeadingNuo>(
    allocator, leading_value);
  assert(std::get<2>(leading_tuple) == leading_value);
}

void TestNuoUsesAllocatorConstruction::TestPairConstruction()
{
  std::allocator<int> allocator;

  using NuoPair = std::pair<LeadingNuo, TrailingAllocator>;
  NuoPair pair =
    nuostl::NuoMakeObjUsingAllocator<NuoPair>(allocator, 1, 2);
  assert(pair.first.value == 1);
  assert(pair.second.value == 2);

  using StdPair = std::pair<LeadingStd, TrailingAllocator>;
  StdPair std_pair =
    std::make_obj_using_allocator<StdPair>(allocator, 1, 2);
  assert(std_pair.first.value == pair.first.value);
  assert(std_pair.second.value == pair.second.value);

  using PlainPair = std::pair<NoAllocator, NoAllocator>;
  PlainPair plain =
    nuostl::NuoMakeObjUsingAllocator<PlainPair>(allocator, 3, 4);
  PlainPair std_plain =
    std::make_obj_using_allocator<PlainPair>(allocator, 3, 4);
  assert(plain.first.value == 3);
  assert(plain.second.value == 4);
  assert(std_plain.second.value == plain.second.value);

  using DefaultPair = std::pair<DefaultLeadingNuo, DefaultLeadingNuo>;
  DefaultPair defaults =
    nuostl::NuoMakeObjUsingAllocator<DefaultPair>(allocator);
  assert(defaults.first.allocator == allocator);
  assert(defaults.second.allocator == allocator);

  using DefaultStdPair = std::pair<DefaultLeadingStd, DefaultLeadingStd>;
  DefaultStdPair std_defaults =
    std::make_obj_using_allocator<DefaultStdPair>(allocator);
  assert(std_defaults.first.allocator == defaults.first.allocator);
  assert(std_defaults.second.allocator == defaults.second.allocator);

  using PiecewisePair = std::pair<LeadingNuo, LeadingNuo>;
  PiecewisePair piecewise =
    nuostl::NuoMakeObjUsingAllocator<PiecewisePair>(
      allocator, std::piecewise_construct,
      std::forward_as_tuple(5), std::forward_as_tuple(6));
  assert(piecewise.first.value == 5);
  assert(piecewise.second.value == 6);

  using StdPiecewisePair = std::pair<LeadingStd, LeadingStd>;
  StdPiecewisePair std_piecewise =
    std::make_obj_using_allocator<StdPiecewisePair>(
      allocator, std::piecewise_construct,
      std::forward_as_tuple(5), std::forward_as_tuple(6));
  assert(std_piecewise.first.value == piecewise.first.value);
  assert(std_piecewise.second.value == piecewise.second.value);

  using InnerPair = std::pair<LeadingNuo, NoAllocator>;
  using NestedPair = std::pair<NoAllocator, InnerPair>;
  NestedPair nested =
    nuostl::NuoMakeObjUsingAllocator<NestedPair>(
      allocator, std::piecewise_construct,
      std::forward_as_tuple(7), std::forward_as_tuple(8, 9));
  assert(nested.first.value == 7);
  assert(nested.second.first.value == 8);
  assert(nested.second.second.value == 9);

  using StdInnerPair = std::pair<LeadingStd, NoAllocator>;
  using StdNestedPair = std::pair<NoAllocator, StdInnerPair>;
  StdNestedPair std_nested =
    std::make_obj_using_allocator<StdNestedPair>(
      allocator, std::piecewise_construct,
      std::forward_as_tuple(7), std::forward_as_tuple(8, 9));
  assert(std_nested.first.value == nested.first.value);
  assert(std_nested.second.first.value == nested.second.first.value);
  assert(std_nested.second.second.value == nested.second.second.value);
}

void TestNuoUsesAllocatorConstruction::TestUninitializedConstruction()
{
  std::allocator<int> allocator;

  alignas(LeadingNuo) unsigned char storage[sizeof(LeadingNuo)];
  LeadingNuo* single =
    nuostl::NuoUninitializedConstructUsingAllocator<LeadingNuo>(
      reinterpret_cast<LeadingNuo*>(storage), allocator, 7);
  assert(single->value == 7);
  assert(single->allocator == allocator);
  nuostl::NuoDestroyAt(single);

  using Pair = std::pair<LeadingNuo, NoAllocator>;
  alignas(Pair) unsigned char pair_storage[sizeof(Pair)];
  Pair* pair =
    nuostl::NuoUninitializedConstructUsingAllocator<Pair>(
      reinterpret_cast<Pair*>(pair_storage), allocator, 1, 2);
  assert(pair->first.value == 1);
  assert(pair->second.value == 2);

  using StdPair = std::pair<LeadingStd, NoAllocator>;
  alignas(StdPair) unsigned char std_storage[sizeof(StdPair)];
  StdPair* std_pair =
    std::uninitialized_construct_using_allocator<StdPair>(
      reinterpret_cast<StdPair*>(std_storage), allocator, 1, 2);
  assert(std_pair->first.value == pair->first.value);
  assert(std_pair->second.value == pair->second.value);

  std::destroy_at(std_pair);
  nuostl::NuoDestroyAt(pair);
}

void TestNuoUsesAllocatorConstruction::TestSuite()
{
  TestGeneralConstruction();
  TestTupleShapes();
  TestPairConstruction();
  TestUninitializedConstruction();
}

} /* namespace test */
