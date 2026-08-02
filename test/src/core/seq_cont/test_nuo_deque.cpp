#include "core/seq_cont/test_nuo_deque.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>

#include "core/seq_cont/nuo_deque.hpp"

namespace test
{

namespace
{

/* an iterator that only advertises the input iterator tag,
   used to exercise the input-iterator dispatch paths */
class TestInputIterator
{
public:
  using iterator_category = nuostl::NuoInputIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = ptrdiff_t;

public:
  TestInputIterator() = default;
  explicit TestInputIterator(const int* ptr) : ptr_(ptr) {}

  reference operator*() const { return *ptr_; }
  pointer operator->() const { return ptr_; }

  TestInputIterator& operator++() { ++ptr_; return *this; }
  TestInputIterator operator++(int)
  {
    TestInputIterator tmp = *this;
    ++ptr_;
    return tmp;
  }
  TestInputIterator& operator--() { --ptr_; return *this; }
  TestInputIterator operator--(int)
  {
    TestInputIterator tmp = *this;
    --ptr_;
    return tmp;
  }

  bool operator==(const TestInputIterator& rhs) const
  {
    return ptr_ == rhs.ptr_;
  }
  bool operator!=(const TestInputIterator& rhs) const
  {
    return ptr_ != rhs.ptr_;
  }
  bool operator<=(const TestInputIterator& rhs) const
  {
    return ptr_ <= rhs.ptr_;
  }
  bool operator>(const TestInputIterator& rhs) const
  {
    return ptr_ > rhs.ptr_;
  }

private:
  const int* ptr_;
};

} /* anonymous namespace */

void TestNuoDeque::test_constructor()
{
  /* default constructor */
  nuostl::NuoDeque<int> d1;
  assert(d1.Empty());
  assert(d1.Size() == 0);

  /* fill constructor */
  nuostl::NuoDeque<int> d2(3);
  assert(d2.Size() == 3);
  assert(d2.Front() == 0);
  assert(d2.Back() == 0);

  nuostl::NuoDeque<int> d3(3, 7);
  assert(d3.Size() == 3);
  assert(d3[0] == 7 && d3[1] == 7 && d3[2] == 7);

  /* range constructor */
  int a[] = {1, 2, 3, 4, 5};
  nuostl::NuoDeque<int> d4(a, a + 5);
  assert(d4.Size() == 5);
  assert(d4.Front() == 1 && d4.Back() == 5);

  /* range constructor with input iterator */
  int b[] = {9, 8, 7};
  nuostl::NuoDeque<int> d5(TestInputIterator(b), TestInputIterator(b + 3));
  assert(d5.Size() == 3);
  assert(d5.Front() == 9 && d5.Back() == 7);

  /* copy constructor */
  nuostl::NuoDeque<int> d6(d4);
  assert(d6.Size() == d4.Size());
  assert(d6.Front() == d4.Front() && d6.Back() == d4.Back());
  assert(d6 == d4);

  /* move constructor */
  nuostl::NuoDeque<int> d7(nuostl::NuoMove(d4));
  assert(d7.Size() == 5);
  assert(d7.Front() == 1 && d7.Back() == 5);
  assert(d4.Empty());

  /* initializer-list constructor */
  nuostl::NuoDeque<int> d8{1, 2, 3, 4, 5, 6, 7, 8};
  assert(d8.Size() == 8);
  assert(d8.Front() == 1 && d8.Back() == 8);

  /* fill constructor with many elements (crosses buffer boundary) */
  nuostl::NuoDeque<int> d9(3000, 1);
  assert(d9.Size() == 3000);
  assert(d9.Front() == 1 && d9.Back() == 1);

  /* string deque (non-trivial type) */
  nuostl::NuoDeque<std::string> ds{"a", "b", "c"};
  assert(ds.Size() == 3);
  assert(ds.Front() == "a" && ds.Back() == "c");
}

void TestNuoDeque::test_assign()
{
  nuostl::NuoDeque<int> d1;

  /* assign n copies (grow) */
  d1.Assign(5, 3);
  assert(d1.Size() == 5);
  assert(d1.Front() == 3 && d1.Back() == 3);

  /* assign n copies (shrink) */
  d1.Assign(3, 9);
  assert(d1.Size() == 3);
  assert(d1[0] == 9 && d1[1] == 9 && d1[2] == 9);

  /* assign n copies (to empty) */
  d1.Assign(0, 1);
  assert(d1.Empty());

  /* assign by range */
  int a[] = {1, 2, 3, 4, 5};
  d1.Assign(a, a + 5);
  assert(d1.Size() == 5);
  assert(d1.Front() == 1 && d1.Back() == 5);

  /* assign by range with input iterator */
  int b[] = {7, 8, 9};
  d1.Assign(TestInputIterator(b), TestInputIterator(b + 3));
  assert(d1.Size() == 3);
  assert(d1.Front() == 7 && d1.Back() == 9);

  /* assign by initializer list */
  d1.Assign({5, 4, 3, 2, 1});
  assert(d1.Size() == 5);
  assert(d1.Front() == 5 && d1.Back() == 1);

  /* copy assignment */
  nuostl::NuoDeque<int> d2{9, 9, 9};
  d2 = d1;
  assert(d2.Size() == d1.Size());
  assert(d2 == d1);

  /* copy assignment when lhs is larger */
  nuostl::NuoDeque<int> d3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  d3 = d1;
  assert(d3.Size() == d1.Size());
  assert(d3 == d1);

  /* move assignment */
  nuostl::NuoDeque<int> d4{1, 2, 3};
  d4 = nuostl::NuoMove(d1);
  assert(d4.Size() == 5);
  assert(d4.Front() == 5 && d4.Back() == 1);
  assert(d1.Empty());
  d1.push_back(42); /* moved-from deque is still usable */
  assert(d1.Size() == 1 && d1.Front() == 42);

  /* initializer-list assignment */
  d4 = {6, 6};
  assert(d4.Size() == 2);
  assert(d4.Front() == 6 && d4.Back() == 6);
}

void TestNuoDeque::test_iterator_and_access()
{
  nuostl::NuoDeque<int> d1{1, 2, 3, 4, 5};

  assert(d1.Front() == 1);
  assert(d1.Back() == 5);

  d1.Front() = 11;
  d1.Back() = 55;
  assert(d1.Front() == 11);
  assert(d1.Back() == 55);

  /* element access */
  assert(d1[0] == 11 && d1[2] == 3 && d1[4] == 55);
  assert(d1.At(1) == 2 && d1.At(4) == 55);

  bool caught = false;
  try
  {
    d1.At(5);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);

  /* iterator arithmetic and comparison */
  nuostl::NuoDeque<int>::iterator it = d1.Begin();
  assert(*it == 11);
  it++;
  assert(*it == 2);
  it += 2;
  assert(*it == 4);
  assert(*(it - 1) == 3);
  assert(*(it + 1) == 55);
  it--;
  assert(*it == 3);
  it -= 1;
  assert(*it == 2);
  assert(it[2] == 4);
  assert(d1.End() - d1.Begin() == 5);
  assert((d1.Begin() + 2) - d1.Begin() == 2);
  assert(d1.Begin() < d1.End());
  assert(d1.End() > d1.Begin());
  assert(d1.Begin() <= d1.Begin());
  assert(d1.End() >= d1.End());
  assert(d1.Begin() != d1.End());

  /* traverse with ++ */
  int seq[] = {11, 2, 3, 4, 55};
  int idx = 0;
  for (it = d1.Begin(); it != d1.End(); ++it, ++idx)
    assert(*it == seq[idx]);
  assert(idx == 5);

  /* traverse with -- */
  int rseq[] = {55, 4, 3, 2, 11};
  idx = 0;
  it = d1.End();
  while (it != d1.Begin())
  {
    --it;
    assert(*it == rseq[idx]);
    ++idx;
  }
  assert(idx == 5);

  /* const iterator */
  const nuostl::NuoDeque<int> d2{6, 7, 8};
  assert(d2.Front() == 6);
  assert(d2.Back() == 8);
  assert(*d2.Begin() == 6);
  assert(*(d2.End() - 1) == 8);
  assert(*d2.Cbegin() == 6);
  assert(*(d2.Cend() - 1) == 8);
  assert(*d2.Rbegin() == 8);
  assert(*(d2.Rend() - 1) == 6);
  assert(*d2.Crbegin() == 8);
  assert(*(d2.Crend() - 1) == 6);

  /* reverse iterator */
  nuostl::NuoDeque<int> d3{1, 2, 3, 4, 5};
  int expected = 5;
  for (nuostl::NuoDeque<int>::reverse_iterator rit = d3.Rbegin();
       rit != d3.Rend(); ++rit)
  {
    assert(*rit == expected);
    expected -= 1;
  }
  assert(expected == 0);

  /* many elements crossing buffer boundaries */
  nuostl::NuoDeque<int> d4(3000, 1);
  int count = 0;
  for (nuostl::NuoDeque<int>::const_iterator cit = d4.Cbegin();
       cit != d4.Cend(); ++cit)
  {
    assert(*cit == 1);
    count++;
  }
  assert(count == 3000);
  assert(d4.Size() ==
         static_cast<nuostl::NuoDeque<int>::size_type>(d4.End() - d4.Begin()));
  assert(d4.MaxSize() > 0);
}

void TestNuoDeque::test_push_pop()
{
  /* push_back across many buffers */
  nuostl::NuoDeque<int> d1;
  for (int i = 0; i < 5000; i++)
    d1.push_back(i);
  assert(d1.Size() == 5000);
  assert(d1.Front() == 0 && d1.Back() == 4999);
  for (int i = 0; i < 5000; i++)
    assert(d1[i] == i);

  /* push_front across many buffers */
  nuostl::NuoDeque<int> d2;
  for (int i = 0; i < 5000; i++)
    d2.push_front(i);
  assert(d2.Size() == 5000);
  assert(d2.Front() == 4999 && d2.Back() == 0);
  for (int i = 0; i < 5000; i++)
    assert(d2[i] == 4999 - i);

  /* alternate push_front / push_back, forcing map growth on both ends */
  nuostl::NuoDeque<int> d3;
  for (int i = 0; i < 5000; i++)
  {
    d3.push_front(i);
    d3.push_back(i);
  }
  assert(d3.Size() == 10000);
  for (int i = 0; i < 5000; i++)
  {
    assert(d3[i] == 4999 - i);
    assert(d3[5000 + i] == i);
  }

  /* emplace */
  nuostl::NuoDeque<std::string> ds;
  ds.EmplaceFront("front");
  ds.EmplaceBack("back");
  ds.EmplaceBack(2, 'x');
  assert(ds.Size() == 3);
  assert(ds.Front() == "front");
  assert(ds.Back() == "xx");
  assert(ds[1] == "back");

  /* pop_back until empty */
  while (!d1.Empty())
    d1.PopBack();
  assert(d1.Empty());
  assert(d1.Size() == 0);

  /* pop_front until empty */
  while (!d2.Empty())
    d2.PopFront();
  assert(d2.Empty());
  assert(d2.Size() == 0);

  /* pop on buffer boundaries */
  nuostl::NuoDeque<int> d4;
  for (int i = 0; i < 3000; i++)
    d4.push_back(i);
  for (int i = 0; i < 1500; i++)
    d4.PopFront();
  assert(d4.Size() == 1500);
  assert(d4.Front() == 1500 && d4.Back() == 2999);
  for (int i = 0; i < 1499; i++)
    d4.PopBack();
  assert(d4.Size() == 1);
  assert(d4.Front() == 1500 && d4.Back() == 1500);
  d4.PopBack();
  assert(d4.Empty());

  /* usable after emptying */
  d4.push_front(1);
  d4.push_back(2);
  assert(d4.Size() == 2);
  assert(d4.Front() == 1 && d4.Back() == 2);
}

void TestNuoDeque::test_insert_erase()
{
  /* insert single value */
  nuostl::NuoDeque<int> d1{1, 2, 3, 4, 5};
  nuostl::NuoDeque<int>::iterator it = d1.Insert(d1.Begin(), 0);
  assert(*it == 0 && d1.Front() == 0);
  it = d1.Insert(d1.End(), 6);
  assert(*it == 6 && d1.Back() == 6);
  it = d1.Insert(d1.Begin() + 2, 99);
  assert(*it == 99);
  assert(d1.Size() == 8);
  int a[] = {0, 1, 99, 2, 3, 4, 5, 6};
  for (size_t i = 0; i < 8; i++)
    assert(d1[i] == a[i]);

  /* insert n copies */
  nuostl::NuoDeque<int> d2{1, 2, 3};
  it = d2.Insert(d2.Begin() + 1, 3, 7);
  assert(*it == 7);
  assert(d2.Size() == 6);
  int b[] = {1, 7, 7, 7, 2, 3};
  for (size_t i = 0; i < 6; i++)
    assert(d2[i] == b[i]);

  /* insert n copies at front / back */
  d2.Insert(d2.Begin(), 2, 0);
  assert(d2.Front() == 0 && d2.Size() == 8);
  d2.Insert(d2.End(), 2, 9);
  assert(d2.Back() == 9 && d2.Size() == 10);

  /* insert empty range does nothing */
  d2.Insert(d2.Begin(), a, a);
  assert(d2.Size() == 10);

  /* insert range at front / middle / back */
  int c[] = {50, 60, 70};
  nuostl::NuoDeque<int> d3{1, 2, 3};
  d3.Insert(d3.Begin(), c, c + 3);
  assert(d3.Size() == 6);
  assert(d3.Front() == 50 && d3.Back() == 3);
  d3.Insert(d3.Begin() + 3, c, c + 3);
  assert(d3.Size() == 9);
  int d[] = {50, 60, 70, 50, 60, 70, 1, 2, 3};
  for (size_t i = 0; i < 9; i++)
    assert(d3[i] == d[i]);

  /* insert range with input iterator */
  nuostl::NuoDeque<int> d4{1, 2, 3, 4, 5};
  d4.Insert(d4.End(), TestInputIterator(c), TestInputIterator(c + 3));
  assert(d4.Size() == 8);
  assert(d4[5] == 50 && d4[7] == 70);
  d4.Insert(d4.Begin() + 2, TestInputIterator(c), TestInputIterator(c + 3));
  assert(d4.Size() == 11);
  int e[] = {1, 2, 50, 60, 70, 3, 4, 5, 50, 60, 70};
  for (size_t i = 0; i < 11; i++)
    assert(d4[i] == e[i]);

  /* insert initializer list */
  nuostl::NuoDeque<int> d5{1, 2};
  d5.Insert(d5.Begin() + 1, {8, 9});
  assert(d5.Size() == 4);
  assert(d5[0] == 1 && d5[1] == 8 && d5[2] == 9 && d5[3] == 2);

  /* emplace in the middle */
  nuostl::NuoDeque<int> d6{1, 2, 3};
  it = d6.Emplace(d6.Begin() + 1, 42);
  assert(*it == 42 && d6.Size() == 4);
  assert(d6[0] == 1 && d6[1] == 42 && d6[2] == 2 && d6[3] == 3);

  /* erase single element */
  nuostl::NuoDeque<int> d7{1, 2, 3, 4, 5};
  it = d7.Erase(d7.Begin());
  assert(*it == 2 && d7.Size() == 4);
  it = d7.Erase(d7.Begin() + 1);
  assert(*it == 4 && d7.Size() == 3);
  it = d7.Erase(d7.End() - 1);
  assert(it == d7.End() && d7.Size() == 2);
  assert(d7[0] == 2 && d7[1] == 4);

  /* erase range */
  nuostl::NuoDeque<int> d8{1, 2, 3, 4, 5, 6, 7, 8, 9};
  d8.Erase(d8.Begin() + 1, d8.Begin() + 4);
  assert(d8.Size() == 6);
  int f[] = {1, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < 6; i++)
    assert(d8[i] == f[i]);
  d8.Erase(d8.Begin(), d8.Begin() + 2);
  assert(d8.Size() == 4);
  assert(d8.Front() == 6);
  assert(d8.Back() == 9);
  d8.Erase(d8.Begin(), d8.End());
  assert(d8.Empty());

  /* insert and erase across buffer boundaries */
  nuostl::NuoDeque<int> d9;
  for (int i = 0; i < 3000; i++)
    d9.push_back(i);
  d9.Insert(d9.Begin() + 1500, 200, 10000);
  assert(d9.Size() == 3200);
  for (int i = 0; i < 1500; i++)
    assert(d9[i] == i);
  for (int i = 0; i < 200; i++)
    assert(d9[1500 + i] == 10000);
  for (int i = 0; i < 1500; i++)
    assert(d9[1700 + i] == 1500 + i);
  d9.Erase(d9.Begin() + 1400, d9.Begin() + 1800);
  assert(d9.Size() == 2800);
  for (int i = 0; i < 1400; i++)
    assert(d9[i] == i);
  for (int i = 0; i < 1400; i++)
    assert(d9[1400 + i] == 1600 + i);
}

void TestNuoDeque::test_comparison_and_swap()
{
  nuostl::NuoDeque<int> d1{1, 2, 3};
  nuostl::NuoDeque<int> d2{1, 2, 3};
  nuostl::NuoDeque<int> d3{1, 2, 4};
  nuostl::NuoDeque<int> d4{1, 2};

  assert(d1 == d2);
  assert(d1 != d3);
  assert(d1 < d3);
  assert(d1 <= d2);
  assert(d1 <= d3);
  assert(d3 > d1);
  assert(d3 >= d2);
  assert(d3 >= d4);
  assert(d4 < d1);
  assert(d4 <= d1);

  /* swap */
  nuostl::NuoDeque<int> d5{9, 8};
  nuostl::NuoDeque<int> d6{1, 2, 3, 4, 5, 6, 7};
  d5.Swap(d6);
  assert(d5.Size() == 7 && d6.Size() == 2);
  assert(d5.Front() == 1 && d5.Back() == 7);
  assert(d6.Front() == 9 && d6.Back() == 8);

  nuostl::NuoSwap(d5, d6);
  assert(d5.Size() == 2 && d6.Size() == 7);
  assert(d5.Front() == 9 && d6.Back() == 7);

  /* clear */
  d6.Clear();
  assert(d6.Empty());
  assert(d6.Size() == 0);
  d6.push_back(1);
  assert(d6.Size() == 1 && d6.Front() == 1);

  /* resize */
  nuostl::NuoDeque<int> d7{1, 2, 3};
  d7.Resize(5);
  assert(d7.Size() == 5);
  assert(d7[3] == 0 && d7[4] == 0);
  d7.Resize(6, 9);
  assert(d7.Size() == 6);
  assert(d7[5] == 9);
  d7.Resize(2);
  assert(d7.Size() == 2);
  assert(d7.Front() == 1 && d7.Back() == 2);

  /* shrink to fit */
  nuostl::NuoDeque<int> d8;
  for (int i = 0; i < 3000; i++)
    d8.push_back(i);
  for (int i = 0; i < 2998; i++)
    d8.PopBack();
  d8.ShrinkToFit();
  assert(d8.Size() == 2);
  assert(d8.Front() == 0 && d8.Back() == 1);
  d8.push_back(2);
  assert(d8.Back() == 2);

  /* get allocator */
  nuostl::NuoDeque<int>::allocator_type alloc = d8.GetAllocator();
  (void)alloc;
}

void TestNuoDeque::test_nuo_deque()
{
  test_constructor();
  test_assign();
  test_iterator_and_access();
  test_push_pop();
  test_insert_erase();
  test_comparison_and_swap();
}

} /* namespace test */
