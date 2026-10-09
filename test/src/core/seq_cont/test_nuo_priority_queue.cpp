#include "core/seq_cont/test_nuo_priority_queue.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

struct StatefulCompare
{
  bool reverse = false;

  bool operator()(int left, int right) const
  {
    return reverse ? left > right : left < right;
  }
};

struct MoveOnlyValue
{
  explicit MoveOnlyValue(int value) : value(value)
  {
  }

  MoveOnlyValue(const MoveOnlyValue&) = delete;
  MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;
  MoveOnlyValue(MoveOnlyValue&&) = default;
  MoveOnlyValue& operator=(MoveOnlyValue&&) = default;

  bool operator<(const MoveOnlyValue& other) const
  {
    return value < other.value;
  }

  int value;
};

} /* namespace */

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

  std::vector<int> seed = {100};
  const std::vector<int> range = {2, 4};
  nuostl::NuoPriorityQueue<int> range_queue(
    range.begin(), range.end(), std::less<int>(), seed);
  assert(range_queue.Size() == 3);
  assert(range_queue.Top() == 100);
  range_queue.Pop();
  assert(range_queue.Top() == 4);

  using StatefulQueue =
    nuostl::NuoPriorityQueue<int, std::vector<int>, StatefulCompare>;
  StatefulQueue max_stateful(StatefulCompare{false});
  StatefulQueue min_stateful(StatefulCompare{true});
  max_stateful.Push(1);
  max_stateful.Push(3);
  min_stateful.Push(1);
  min_stateful.Push(3);
  max_stateful.Swap(min_stateful);
  max_stateful.Push(0);
  min_stateful.Push(4);
  assert(max_stateful.Top() == 0);
  assert(min_stateful.Top() == 4);

  nuostl::NuoPriorityQueue<MoveOnlyValue> move_only_queue;
  move_only_queue.Emplace(3);
  move_only_queue.Emplace(7);
  move_only_queue.Emplace(1);
  assert(move_only_queue.Top().value == 7);
  move_only_queue.Pop();
  assert(move_only_queue.Top().value == 3);
}

} /* namespace test */
