#include "core/seq_cont/test_nuo_stack.hpp"

#include <assert.h>

#include <string>

#include "core/seq_cont/nuo_stack.hpp"

namespace test
{

void TestNuoStack::test_constructor()
{
  /* default constructor */
  nuostl::NuoStack<int> s1;
  assert(s1.Empty());
  assert(s1.Size() == 0);

  /* constructor from container (copy) */
  nuostl::NuoDeque<int> dq1{3, 2, 1};
  nuostl::NuoStack<int> s2(dq1);
  assert(s2.Size() == 3);
  assert(s2.Top() == 1);

  /* constructor from container (move) */
  nuostl::NuoStack<int> s3(nuostl::NuoDeque<int>{5, 4});
  assert(s3.Size() == 2);
  assert(s3.Top() == 4);

  /* range constructor */
  int arr[] = {9, 8, 7, 6};
  nuostl::NuoStack<int> s4(arr, arr + 4);
  assert(s4.Size() == 4);
  assert(s4.Top() == 6);

  /* copy constructor */
  nuostl::NuoStack<int> s5(s4);
  assert(s5 == s4);
  s5.Pop();
  assert(s5.Size() == 3);
  assert(s4.Size() == 4);

  /* move constructor */
  nuostl::NuoStack<int> s_src;
  s_src.Push(1);
  s_src.Push(2);
  s_src.Push(3);
  nuostl::NuoStack<int> s6(nuostl::NuoMove(s_src));
  assert(s_src.Empty());
  assert(s6.Size() == 3);
  assert(s6.Top() == 3);

  /* string stack */
  nuostl::NuoStack<std::string> s7;
  s7.Push("a");
  s7.Push(std::string("b"));
  assert(s7.Size() == 2);
  assert(s7.Top() == "b");
}

void TestNuoStack::test_assign()
{
  /* copy assignment */
  nuostl::NuoStack<int> s1;
  for (int i = 0; i < 5; i++)
    s1.Push(i);
  nuostl::NuoStack<int> s2;
  s2 = s1;
  assert(s2.Size() == 5);
  assert(s2 == s1);

  /* move assignment */
  nuostl::NuoStack<int> s3;
  nuostl::NuoStack<int> tmp3;
  tmp3.Push(7);
  s3 = nuostl::NuoMove(tmp3);
  assert(tmp3.Empty());
  assert(s3.Size() == 1);
  assert(s3.Top() == 7);

  /* assign an empty stack to another */
  nuostl::NuoStack<int> s4;
  nuostl::NuoStack<int> s5;
  s5.Push(1);
  s5 = s4;
  assert(s5.Empty());
}

void TestNuoStack::test_capacity_and_access()
{
  nuostl::NuoStack<int> s1;
  assert(s1.Empty());

  s1.Push(1);
  assert(!s1.Empty());
  assert(s1.Size() == 1);

  s1.Push(2);
  s1.Push(3);
  assert(s1.Size() == 3);

  /* top reference can be modified */
  s1.Top() = 33;
  assert(s1.Top() == 33);

  /* const access */
  const nuostl::NuoStack<int> s2 = s1;
  assert(s2.Top() == 33);
  assert(s2.Size() == 3);
  assert(!s2.Empty());
}

void TestNuoStack::test_push_pop()
{
  /* LIFO order */
  nuostl::NuoStack<int> s1;
  for (int i = 0; i < 1000; i++)
    s1.Push(i);
  assert(s1.Size() == 1000);
  for (int i = 999; i >= 0; i--)
  {
    assert(s1.Top() == i);
    s1.Pop();
  }
  assert(s1.Empty());

  /* push rvalue */
  nuostl::NuoStack<std::string> s2;
  std::string val = "old";
  s2.Push(val);
  s2.Push(std::string("new"));
  assert(s2.Top() == "new");
  s2.Pop();
  assert(s2.Top() == "old");

  /* pop all elements */
  nuostl::NuoStack<int> s3;
  s3.Push(7);
  s3.Push(8);
  s3.Push(9);
  assert(s3.Size() == 3);
  s3.Pop();
  s3.Pop();
  s3.Pop();
  assert(s3.Empty());
}

void TestNuoStack::test_emplace()
{
  nuostl::NuoStack<int> s1;
  s1.Emplace(1);
  s1.Emplace(2);
  assert(s1.Size() == 2);
  assert(s1.Top() == 2);

  /* emplace with string arguments in place */
  nuostl::NuoStack<std::string> s2;
  s2.Emplace(3, 'a');
  assert(s2.Size() == 1);
  assert(s2.Top() == "aaa");
}

void TestNuoStack::test_comparison_and_swap()
{
  nuostl::NuoStack<int> s1;
  s1.Push(1);
  s1.Push(2);
  s1.Push(3);

  nuostl::NuoStack<int> s2;
  s2.Push(1);
  s2.Push(2);
  s2.Push(3);

  nuostl::NuoStack<int> s3;
  s3.Push(1);
  s3.Push(2);
  s3.Push(4);

  nuostl::NuoStack<int> s4;
  s4.Push(1);
  s4.Push(2);

  /* equal / not equal */
  assert(s1 == s2);
  assert(s1 != s3);
  assert(!(s1 != s2));

  /* relational */
  assert(s1 < s3);
  assert(s1 <= s2);
  assert(s1 <= s3);
  assert(s3 > s1);
  assert(s3 >= s1);
  assert(s3 >= s2);
  assert(!(s3 < s1));

  /* differ in length : prefix rule */
  assert(s4 != s1);
  assert(s4 < s1);
  assert(s1 > s4);

  /* member swap */
  nuostl::NuoStack<int> a;
  a.Push(1);
  a.Push(2);
  nuostl::NuoStack<int> b;
  b.Push(9);
  a.Swap(b);
  assert(a.Size() == 1 && a.Top() == 9);
  assert(b.Size() == 2 && b.Top() == 2);

  /* free swap */
  nuostl::NuoStack<int> c;
  c.Push(5);
  nuostl::NuoStack<int> d;
  d.Push(6);
  d.Push(7);
  nuostl::NuoSwap(c, d);
  assert(c.Size() == 2 && c.Top() == 7);
  assert(d.Size() == 1 && d.Top() == 5);
}

void TestNuoStack::test_nuo_stack()
{
  test_constructor();
  test_assign();
  test_capacity_and_access();
  test_push_pop();
  test_emplace();
  test_comparison_and_swap();
}

} /* namespace test */