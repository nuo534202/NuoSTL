#include "core/seq_cont/test_nuo_forward_list.hpp"

#include <assert.h>

#include <string>

#include "core/seq_cont/nuo_forward_list.hpp"

namespace test
{

void TestNuoForwardList::test_constructor()
{
  /* default constructor */
  nuostl::NuoForwardList<int> l1;
  assert(l1.Empty());
  assert(l1.Size() == 0);

  /* fill constructor */
  nuostl::NuoForwardList<int> l2(3, 7);
  assert(!l2.Empty());
  assert(l2.Size() == 3);
  assert(l2.Front() == 7);

  /* initializer-list constructor */
  nuostl::NuoForwardList<int> l3{9, 8, 7};
  assert(l3.Size() == 3);
  assert(l3.Front() == 9);

  /* initializer-list constructor with string */
  nuostl::NuoForwardList<std::string> l4{"a", "b"};
  assert(l4.Size() == 2);
  assert(l4.Front() == "a");
}

void TestNuoForwardList::test_assign()
{
  nuostl::NuoForwardList<int> l1;

  /* assign n copies */
  l1.Assign(4, 5);
  assert(l1.Size() == 4);
  assert(l1.Front() == 5);

  /* assign by initializer list */
  l1.Assign({1, 3, 5, 7});
  assert(l1.Size() == 4);
  assert(l1.Front() == 1);

  /* assign one element */
  l1.Assign(1, 42);
  assert(l1.Size() == 1);
  assert(l1.Front() == 42);
}

void TestNuoForwardList::test_iterator_and_access()
{
  nuostl::NuoForwardList<int> l1{1, 2, 3, 4, 5};

  assert(l1.Front() == 1);

  l1.Front() = 11;
  assert(l1.Front() == 11);

  /* iterate forward with ordinary iterators */
  int expected[] = {11, 2, 3, 4, 5};
  int index = 0;
  for (nuostl::NuoForwardList<int>::iterator it = l1.Begin(); it != l1.End(); ++it)
    assert(*it == expected[index++]);
  assert(index == 5);

  /* const iterators */
  const nuostl::NuoForwardList<int> l2{6, 7, 8};
  assert(l2.Front() == 6);
  nuostl::NuoForwardList<int>::const_iterator cit = l2.Begin();
  assert(*cit == 6);
}

void TestNuoForwardList::test_push_front()
{
  nuostl::NuoForwardList<int> l1;

  l1.PushFront(2);
  l1.PushFront(1);
  l1.PushFront(0);

  assert(l1.Size() == 3);
  assert(l1.Front() == 0);

  l1.PopFront();
  assert(l1.Size() == 2);
  assert(l1.Front() == 1);

  l1.PopFront();
  l1.PopFront();
  assert(l1.Empty());
}

void TestNuoForwardList::test_insert_after()
{
  nuostl::NuoForwardList<int> l1{1, 2, 3};

  /* insert after begin */
  nuostl::NuoForwardList<int>::iterator it = l1.Begin();
  l1.InsertAfter(it, 10);
  assert(l1.Size() == 4);

  /* insert at end (End() == nullptr position) */
  l1.InsertAfter(l1.End(), 99);
  assert(l1.Size() == 5);
}

void TestNuoForwardList::test_erase_after()
{
  nuostl::NuoForwardList<int> l1{1, 2, 3, 4, 5};

  /* erase the element after begin: removes 2 -> {1,3,4,5} */
  l1.EraseAfter(l1.Begin());
  assert(l1.Size() == 4);

  /* erase half-open range (first, last): removes 3 -> {1,4,5} */
  nuostl::NuoForwardList<int>::iterator first = l1.Begin();
  nuostl::NuoForwardList<int>::iterator last = l1.Begin();
  ++last;
  ++last;
  l1.EraseAfter(first, last);
  assert(l1.Size() == 3);

  /* empty range (first == last) is a no-op */
  l1.EraseAfter(l1.Begin(), l1.Begin());
  assert(l1.Size() == 3);

  /* erase all remaining via before-begin */
  while (l1.EraseAfter(l1.BeforeBegin()) != l1.End())
    ;
  assert(l1.Empty());
  assert(l1.Size() == 0);
}

void TestNuoForwardList::test_clear_and_swap()
{
  nuostl::NuoForwardList<int> l1{1, 2, 3};
  nuostl::NuoForwardList<int> l2{9, 8};

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

void TestNuoForwardList::test_splice()
{
  nuostl::NuoForwardList<int> l1{1, 2, 3};
  nuostl::NuoForwardList<int> l2{9, 8, 7};

  /* splice all of l2 after end of l1 */
  l1.SpliceAfter(l1.End(), l2);
  assert(l1.Size() == 6);
  assert(l2.Empty());

  /* move assignment splices at end */
  nuostl::NuoForwardList<int> l3;
  l3 = nuostl::NuoMove(l1);
  assert(l3.Size() == 6);
  assert(l1.Empty());
}

void TestNuoForwardList::test_remove_and_unique()
{
  nuostl::NuoForwardList<int> l1{1, 2, 2, 3, 3, 3, 4};

  size_t removed = l1.Remove(3);
  assert(removed == 3);
  assert(l1.Size() == 4);

  l1.Unique();
  assert(l1.Size() == 3);
}

void TestNuoForwardList::test_merge_and_sort_reverse()
{
  nuostl::NuoForwardList<int> l1{2, 4, 6};
  nuostl::NuoForwardList<int> l2{1, 3, 5};

  l1.Merge(l2);
  assert(l1.Size() == 6);
  assert(l1.Front() == 1);

  l1.Reverse();
  assert(l1.Front() == 6);

  nuostl::NuoForwardList<int> l3{5, 1, 3, 2, 4};
  l3.Sort();
  assert(l3.Front() == 1);
}

void TestNuoForwardList::test_nuo_forward_list()
{
  test_constructor();
  test_assign();
  test_iterator_and_access();
  test_push_front();
  test_insert_after();
  test_erase_after();
  test_clear_and_swap();
  test_splice();
  test_remove_and_unique();
  test_merge_and_sort_reverse();
}

} /* namespace test */