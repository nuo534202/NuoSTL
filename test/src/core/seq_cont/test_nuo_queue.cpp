#include "core/seq_cont/test_nuo_queue.hpp"

#include <assert.h>

#include <string>

#include "core/seq_cont/nuo_queue.hpp"

namespace test
{

void TestNuoQueue::test_constructor()
{
  /* default constructor */
  nuostl::NuoQueue<int> q1;
  assert(q1.Empty());
  assert(q1.Size() == 0);

  /* constructor from container (copy) */
  nuostl::NuoDeque<int> dq1{1, 2, 3};
  nuostl::NuoQueue<int> q2(dq1);
  assert(q2.Size() == 3);
  assert(q2.Front() == 1);
  assert(q2.Back() == 3);

  /* constructor from container (move) */
  nuostl::NuoQueue<int> q3(nuostl::NuoDeque<int>{4, 5});
  assert(q3.Size() == 2);
  assert(q3.Front() == 4);
  assert(q3.Back() == 5);

  /* range constructor */
  int arr[] = {6, 7, 8, 9};
  nuostl::NuoQueue<int> q4(arr, arr + 4);
  assert(q4.Size() == 4);
  assert(q4.Front() == 6);
  assert(q4.Back() == 9);

  /* copy constructor */
  nuostl::NuoQueue<int> q5(q4);
  assert(q5 == q4);
  q5.Pop();
  assert(q5.Size() == 3);
  assert(q4.Size() == 4);

  /* move constructor */
  nuostl::NuoQueue<int> q_src;
  q_src.Push(1);
  q_src.Push(2);
  q_src.Push(3);
  nuostl::NuoQueue<int> q6(nuostl::NuoMove(q_src));
  assert(q_src.Empty());
  assert(q6.Size() == 3);
  assert(q6.Front() == 1);
  assert(q6.Back() == 3);

  /* string queue */
  nuostl::NuoQueue<std::string> q7;
  q7.Push("a");
  q7.Push(std::string("b"));
  assert(q7.Size() == 2);
  assert(q7.Front() == "a");
  assert(q7.Back() == "b");
}

void TestNuoQueue::test_assign()
{
  /* copy assignment */
  nuostl::NuoQueue<int> q1;
  for (int i = 0; i < 5; i++)
    q1.Push(i);
  nuostl::NuoQueue<int> q2;
  q2 = q1;
  assert(q2.Size() == 5);
  assert(q2 == q1);

  /* move assignment */
  nuostl::NuoQueue<int> q3;
  nuostl::NuoQueue<int> tmp3;
  tmp3.Push(7);
  q3 = nuostl::NuoMove(tmp3);
  assert(tmp3.Empty());
  assert(q3.Size() == 1);
  assert(q3.Front() == 7);
  assert(q3.Back() == 7);

  /* assign an empty queue to another */
  nuostl::NuoQueue<int> q4;
  nuostl::NuoQueue<int> q5;
  q5.Push(1);
  q5 = q4;
  assert(q5.Empty());
}

void TestNuoQueue::test_capacity_and_access()
{
  nuostl::NuoQueue<int> q1;
  assert(q1.Empty());

  q1.Push(1);
  assert(!q1.Empty());
  assert(q1.Size() == 1);

  q1.Push(2);
  q1.Push(3);
  assert(q1.Size() == 3);

  /* front/back references can be modified */
  q1.Front() = 33;
  q1.Back() = 44;
  assert(q1.Front() == 33);
  assert(q1.Back() == 44);

  /* const access */
  const nuostl::NuoQueue<int> q2 = q1;
  assert(q2.Front() == 33);
  assert(q2.Back() == 44);
  assert(q2.Size() == 3);
  assert(!q2.Empty());
}

void TestNuoQueue::test_push_pop()
{
  /* FIFO order */
  nuostl::NuoQueue<int> q1;
  for (int i = 0; i < 1000; i++)
    q1.Push(i);
  assert(q1.Size() == 1000);
  for (int i = 0; i < 1000; i++)
  {
    assert(q1.Front() == i);
    q1.Pop();
  }
  assert(q1.Empty());

  /* push rvalue */
  nuostl::NuoQueue<std::string> q2;
  std::string val = "old";
  q2.Push(val);
  q2.Push(std::string("new"));
  assert(q2.Front() == "old");
  assert(q2.Back() == "new");
  q2.Pop();
  assert(q2.Front() == "new");

  /* pop all elements */
  nuostl::NuoQueue<int> q3;
  q3.Push(7);
  q3.Push(8);
  q3.Push(9);
  assert(q3.Size() == 3);
  q3.Pop();
  q3.Pop();
  q3.Pop();
  assert(q3.Empty());
}

void TestNuoQueue::test_emplace()
{
  nuostl::NuoQueue<int> q1;
  q1.Emplace(1);
  q1.Emplace(2);
  assert(q1.Size() == 2);
  assert(q1.Front() == 1);
  assert(q1.Back() == 2);

  /* emplace with string arguments in place */
  nuostl::NuoQueue<std::string> q2;
  q2.Emplace(3, 'a');
  assert(q2.Size() == 1);
  assert(q2.Front() == "aaa");
  assert(q2.Back() == "aaa");
}

void TestNuoQueue::test_comparison_and_swap()
{
  nuostl::NuoQueue<int> q1;
  q1.Push(1);
  q1.Push(2);
  q1.Push(3);

  nuostl::NuoQueue<int> q2;
  q2.Push(1);
  q2.Push(2);
  q2.Push(3);

  nuostl::NuoQueue<int> q3;
  q3.Push(1);
  q3.Push(2);
  q3.Push(4);

  nuostl::NuoQueue<int> q4;
  q4.Push(1);
  q4.Push(2);

  /* equal / not equal */
  assert(q1 == q2);
  assert(q1 != q3);
  assert(!(q1 != q2));

  /* relational */
  assert(q1 < q3);
  assert(q1 <= q2);
  assert(q1 <= q3);
  assert(q3 > q1);
  assert(q3 >= q1);
  assert(q3 >= q2);
  assert(!(q3 < q1));

  /* differ in length : prefix rule */
  assert(q4 != q1);
  assert(q4 < q1);
  assert(q1 > q4);

  /* member swap */
  nuostl::NuoQueue<int> a;
  a.Push(1);
  a.Push(2);
  nuostl::NuoQueue<int> b;
  b.Push(9);
  a.Swap(b);
  assert(a.Size() == 1 && a.Front() == 9 && a.Back() == 9);
  assert(b.Size() == 2 && b.Front() == 1 && b.Back() == 2);

  /* free swap */
  nuostl::NuoQueue<int> c;
  c.Push(5);
  nuostl::NuoQueue<int> d;
  d.Push(6);
  d.Push(7);
  nuostl::NuoSwap(c, d);
  assert(c.Size() == 2 && c.Front() == 6 && c.Back() == 7);
  assert(d.Size() == 1 && d.Front() == 5 && d.Back() == 5);
}

void TestNuoQueue::test_nuo_queue()
{
  test_constructor();
  test_assign();
  test_capacity_and_access();
  test_push_pop();
  test_emplace();
  test_comparison_and_swap();
}

} /* namespace test */