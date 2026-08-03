#include "core/seq_cont/test_nuo_array.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/seq_cont/nuo_array.hpp"

namespace test
{

/* constexpr checks are evaluated at compile time */
constexpr nuostl::NuoArray<int, 3> kConstArray = {1, 2, 3};
static_assert(kConstArray.Size() == 3);
static_assert(kConstArray.MaxSize() == 3);
static_assert(!kConstArray.Empty());
static_assert(kConstArray[0] == 1);
static_assert(kConstArray[1] == 2);
static_assert(kConstArray[2] == 3);
static_assert(kConstArray.At(2) == 3);
static_assert(kConstArray.Front() == 1);
static_assert(kConstArray.Back() == 3);

void TestNuoArray::test_constructor()
{
  /* aggregate initialization (brace init) */
  nuostl::NuoArray<int, 3> a1 = {1, 2, 3};
  assert(a1.Size() == 3);
  assert(a1[0] == 1);
  assert(a1[1] == 2);
  assert(a1[2] == 3);

  /* deduction guide : NuoArray<int, 3> */
  nuostl::NuoArray a2{4, 5, 6};
  static_assert(std::is_same<decltype(a2), nuostl::NuoArray<int, 3>>::value,
                "guide should deduce NuoArray<int, 3>");
  assert(a2[0] == 4);
  assert(a2[2] == 6);

  /* default constructor : elements are uninitialized, no assertion on value */
  nuostl::NuoArray<std::string, 2> a3;
  a3[0] = "hello";
  a3[1] = "world";
  assert(a3[0] == "hello");
  assert(a3[1] == "world");

  /* zero-length array */
  nuostl::NuoArray<int, 0> a4;
  assert(a4.Empty());
  assert(a4.Size() == 0);
  assert(a4.Begin() == a4.End());
}

void TestNuoArray::test_assign()
{
  /* copy assignment */
  nuostl::NuoArray<int, 3> a1 = {1, 2, 3};
  nuostl::NuoArray<int, 3> a2;
  a2 = a1;
  assert(a2 == a1);
  a2[0] = 9;
  assert(a1[0] == 1);
  assert(a2[0] == 9);

  /* move assignment */
  nuostl::NuoArray<std::string, 2> s1;
  s1[0] = "x";
  s1[1] = "y";
  nuostl::NuoArray<std::string, 2> s2;
  s2 = nuostl::NuoMove(s1);
  assert(s2[0] == "x");
  assert(s2[1] == "y");
}

void TestNuoArray::test_capacity_and_access()
{
  nuostl::NuoArray<int, 5> a1 = {1, 2, 3, 4, 5};

  assert(a1.Size() == 5);
  assert(a1.MaxSize() == 5);
  assert(!a1.Empty());

  /* operator[] mutable */
  a1[1] = 20;
  a1[4] = 50;
  assert(a1[1] == 20);
  assert(a1[4] == 50);

  /* operator[] const */
  const nuostl::NuoArray<int, 5>& a1_ref = a1;
  assert(a1_ref[0] == 1);
  assert(a1_ref[2] == 3);

  /* at() in range */
  assert(a1.At(0) == 1);
  assert(a1.At(4) == 50);
  assert(a1_ref.At(3) == 4);

  /* at() out of range throws (mutable version) */
  bool threw = false;
  try
  {
    a1.At(5);
  }
  catch (const std::out_of_range&)
  {
    threw = true;
  }
  assert(threw);

  /* at() out of range throws (const version) */
  bool threw_const = false;
  try
  {
    a1_ref.At(10);
  }
  catch (const std::out_of_range&)
  {
    threw_const = true;
  }
  assert(threw_const);

  /* front/back mutable */
  a1.Front() = -1;
  a1.Back() = -1;
  assert(a1[0] == -1);
  assert(a1[4] == -1);
  assert(a1.Front() == -1);
  assert(a1.Back() == -1);

  /* front/back const */
  const nuostl::NuoArray<int, 3> a2 = {7, 8, 9};
  assert(a2.Front() == 7);
  assert(a2.Back() == 9);

  /* data() */
  assert(a1.Data() == &a1[0]);
  const int* p = a2.Data();
  assert(p[2] == 9);
}

void TestNuoArray::test_iterators()
{
  nuostl::NuoArray<int, 4> a1 = {1, 2, 3, 4};

  /* begin / end forward traversal */
  int sum = 0;
  for (auto it = a1.Begin(); it != a1.End(); it++)
    sum += *it;
  assert(sum == 10);

  /* cbegin / cend */
  const nuostl::NuoArray<int, 4>& a1_ref = a1;
  sum = 0;
  for (auto it = a1_ref.CBegin(); it != a1_ref.CEnd(); it++)
    sum += *it;
  assert(sum == 10);

  /* rbegin / rend reverse traversal */
  nuostl::NuoArray<int, 4> a2 = {10, 20, 30, 40};
  nuostl::NuoArray<int, 4> reversed;
  size_t i = 0;
  for (auto it = a2.RBegin(); it != a2.REnd(); it++, i++)
    reversed[i] = *it;
  assert(reversed[0] == 40);
  assert(reversed[1] == 30);
  assert(reversed[2] == 20);
  assert(reversed[3] == 10);
}

void TestNuoArray::test_swap()
{
  nuostl::NuoArray<int, 3> a1 = {1, 2, 3};
  nuostl::NuoArray<int, 3> a2 = {7, 8, 9};

  /* member swap */
  a1.Swap(a2);
  assert(a1[0] == 7 && a1[1] == 8 && a1[2] == 9);
  assert(a2[0] == 1 && a2[1] == 2 && a2[2] == 3);

  /* free swap */
  nuostl::NuoSwap(a1, a2);
  assert(a1[0] == 1 && a1[1] == 2 && a1[2] == 3);
  assert(a2[0] == 7 && a2[1] == 8 && a2[2] == 9);

  /* noexcept for nothrow_swappable element type */
  static_assert(noexcept(nuostl::NuoSwap(a1, a2)),
                "swap of int array should be noexcept");
}

void TestNuoArray::test_comparison()
{
  /* fill */
  nuostl::NuoArray<int, 5> a1;
  a1.Fill(7);
  for (size_t i = 0; i < a1.Size(); i++)
    assert(a1[i] == 7);

  /* comparisons between equal length arrays */
  nuostl::NuoArray<int, 3> x1 = {1, 2, 3};
  nuostl::NuoArray<int, 3> x2 = {1, 2, 3};
  nuostl::NuoArray<int, 3> x3 = {1, 2, 4};
  assert(x1 == x2);
  assert(!(x1 != x2));
  assert(x1 != x3);
  assert(x1 < x3);
  assert(x1 <= x2);
  assert(x1 <= x3);
  assert(x3 > x1);
  assert(x3 >= x1);
  assert(x3 >= x2);
  assert(!(x3 < x1));
}

void TestNuoArray::test_nuo_array()
{
  test_constructor();
  test_assign();
  test_capacity_and_access();
  test_iterators();
  test_swap();
  test_comparison();
}

} /* namespace test */