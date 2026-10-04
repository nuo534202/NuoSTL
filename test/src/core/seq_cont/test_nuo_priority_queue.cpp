#include "core/seq_cont/test_nuo_priority_queue.hpp"

#include <cassert>
#include <functional>
#include <vector>

#include "nuostl.hpp"

namespace test
{

void TestNuoPriorityQueue::TestNuoPriorityQueueSuite()
{
  nuostl::NuoPriorityQueue<int> max_queue;
  assert(max_queue.Empty());
  max_queue.Push(3);
  max_queue.Push(1);
  max_queue.Emplace(5);
  max_queue.Push(2);
  assert(max_queue.Size() == 4);
  assert(max_queue.Top() == 5);
  max_queue.Pop();
  assert(max_queue.Top() == 3);
  const int expected_order[] = {3, 2, 1};
  for (int expected : expected_order)
  {
    assert(max_queue.Top() == expected);
    max_queue.Pop();
  }
  assert(max_queue.Empty());

  std::vector<int> unsorted = {2, 10, 4};
  nuostl::NuoPriorityQueue<int> built_from_container(nuostl::NuoMove(unsorted));
  assert(built_from_container.Top() == 10);

  std::vector<int> source = {4, 1, 7, 2, 9};
  nuostl::NuoPriorityQueue<int, nuostl::NuoDeque<int>, std::greater<int>>
    min_queue(source.begin(), source.end());
  assert(min_queue.Size() == source.size());
  assert(min_queue.Top() == 1);
  min_queue.Pop();
  assert(min_queue.Top() == 2);

  nuostl::NuoPriorityQueue<int> copied(built_from_container);
  assert(copied.Top() == built_from_container.Top());
  nuostl::NuoPriorityQueue<int> moved(nuostl::NuoMove(copied));
  assert(moved.Top() == 10);

  nuostl::NuoPriorityQueue<int> other;
  other.Push(100);
  moved.Swap(other);
  assert(moved.Top() == 100);
  assert(other.Top() == 10);

  nuostl::NuoPriorityQueue<int, std::vector<int>> vector_queue;
  vector_queue.Push(8);
  vector_queue.Push(6);
  assert(vector_queue.Top() == 8);
}

} /* namespace test */
