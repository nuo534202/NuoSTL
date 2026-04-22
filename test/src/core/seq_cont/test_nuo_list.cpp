#include "core/seq_cont/test_nuo_list.hpp"

#include <assert.h>

#include <string>

#include "core/seq_cont/nuo_list.hpp"

namespace test
{

void TestNuoList::test_constructor()
{
  /* default constructor */
  nuostl::NuoList<int> l1;
  assert(l1.Empty());
  assert(l1.Size() == 0);

  /* fill constructor */
  nuostl::NuoList<int> l2(3, 7);
  assert(!l2.Empty());
  assert(l2.Size() == 3);
  assert(l2.Front() == 7);
  assert(l2.Back() == 7);

  /* initializer-list constructor */
  nuostl::NuoList<int> l3{9, 8, 7};
  assert(l3.Size() == 3);
  assert(l3.Front() == 9);
  assert(l3.Back() == 7);

  /* initializer-list constructor with string */
  nuostl::NuoList<std::string> l4{"a", "b"};
  assert(l4.Size() == 2);
  assert(l4.Front() == "a");
  assert(l4.Back() == "b");
}

void TestNuoList::test_assign()
{
  nuostl::NuoList<int> l1;

  /* assign n copies */
  l1.Assign(4, 5);
  assert(l1.Size() == 4);
  assert(l1.Front() == 5);
  assert(l1.Back() == 5);

  /* assign by initializer list */
  l1.Assign({1, 3, 5, 7});
  assert(l1.Size() == 4);
  assert(l1.Front() == 1);
  assert(l1.Back() == 7);

  /* assign one element */
  l1.Assign(1, 42);
  assert(l1.Size() == 1);
  assert(l1.Front() == 42);
  assert(l1.Back() == 42);
}

void TestNuoList::test_iterator_and_access()
{
  nuostl::NuoList<int> l1{1, 2, 3, 4, 5};

  assert(l1.Front() == 1);
  assert(l1.Back() == 5);

  l1.Front() = 11;
  l1.Back() = 55;
  assert(l1.Front() == 11);
  assert(l1.Back() == 55);

  const nuostl::NuoList<int> l2{6, 7, 8};
  assert(l2.Front() == 6);
  assert(l2.Back() == 8);
}

void TestNuoList::test_push_pop()
{
  nuostl::NuoList<int> l1;

  l1.PushBack(2);
  l1.PushFront(1);
  l1.PushBack(3);

  assert(l1.Size() == 3);
  assert(l1.Front() == 1);
  assert(l1.Back() == 3);

  l1.PopFront();
  assert(l1.Size() == 2);
  assert(l1.Front() == 2);

  l1.PopBack();
  assert(l1.Size() == 1);
  assert(l1.Front() == 2);
  assert(l1.Back() == 2);

  l1.PopBack();
  assert(l1.Empty());
}

void TestNuoList::test_insert_erase()
{
  /* use push/pop to cover insertion/erasure semantics */
  nuostl::NuoList<int> l1;

  l1.PushBack(3);
  l1.PushFront(2);
  l1.PushFront(1);
  assert(l1.Size() == 3);
  assert(l1.Front() == 1);
  assert(l1.Back() == 3);

  l1.PopFront();
  assert(l1.Size() == 2);
  assert(l1.Front() == 2);

  l1.PopBack();
  assert(l1.Size() == 1);
  assert(l1.Front() == 2);
  assert(l1.Back() == 2);
}

void TestNuoList::test_clear_and_swap()
{
  nuostl::NuoList<int> l1{1, 2, 3};
  nuostl::NuoList<int> l2{9, 8};

  l1.Swap(l2);
  assert(l1.Size() == 2);
  assert(l2.Size() == 3);
  assert(l1.Front() == 9);
  assert(l2.Front() == 1);

  l1.Clear();
  assert(l1.Empty());
  assert(l1.Size() == 0);

  l2.Clear();
  assert(l2.Empty());
}

void TestNuoList::test_nuo_list()
{
  test_constructor();
  test_assign();
  test_iterator_and_access();
  test_push_pop();
  test_insert_erase();
  test_clear_and_swap();
}

} /* namespace test */
