#include "utils/test_nuo_pool_allocator.hpp"

#include <cassert>

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>
#include <new>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

struct OverAligned
{
  alignas(64) unsigned char payload[64];
};

struct Node
{
  Node* next;
  int value;
};

struct TrackedValue
{
  TrackedValue();
  ~TrackedValue();

  static inline int live_count = 0;
  int value = 0;
};

TrackedValue::TrackedValue()
{
  ++live_count;
}

TrackedValue::~TrackedValue()
{
  --live_count;
}

void CheckRawPool()
{
  nuostl::NuoBlockPool pool(16, alignof(void*), 4);
  assert(pool.BlockSize() >= 16);
  assert(pool.BlockSize() % pool.BlockAlignment() == 0);
  assert(pool.BlocksPerChunk() == 4);
  assert(pool.ChunkCount() == 0);
  assert(pool.BlockCount() == 0);
  assert(pool.BlocksInUse() == 0);

  std::vector<void*> blocks;
  for (int index = 0; index < 10; ++index)
  {
    blocks.push_back(pool.Allocate());
  }
  assert(pool.BlocksInUse() == 10);
  assert(pool.BlockCount() == 12);
  assert(pool.ChunkCount() == 3);

  std::set<void*> unique(blocks.begin(), blocks.end());
  assert(unique.size() == blocks.size());
  for (void* block : blocks)
  {
    assert(block != nullptr);
    assert(reinterpret_cast<std::uintptr_t>(block) %
             pool.BlockAlignment() == 0);
  }

  void* recycled = blocks.back();
  pool.Deallocate(recycled);
  assert(pool.BlocksInUse() == 9);
  assert(pool.Allocate() == recycled);
  assert(pool.BlocksInUse() == 10);

  for (void* block : blocks)
  {
    pool.Deallocate(block);
  }
  assert(pool.BlocksInUse() == 0);
  assert(pool.FreeBlockCount() == pool.BlockCount());

  /* A tiny request is rounded up so it can hold a free-list pointer. */
  nuostl::NuoBlockPool small(1, 1, 2);
  assert(small.BlockSize() >= sizeof(void*));
  assert(small.BlockSize() % small.BlockAlignment() == 0);
  void* first = small.Allocate();
  void* second = small.Allocate();
  assert(first != second && small.ChunkCount() == 1);
  small.Deallocate(first);
  small.Deallocate(second);
  assert(small.BlocksInUse() == 0);

  /* Raw byte helpers honour over-aligned requests. */
  void* aligned = nuostl::NuoBlockPool::AllocateBytes(128, 64);
  assert(reinterpret_cast<std::uintptr_t>(aligned) % 64 == 0);
  nuostl::NuoBlockPool::DeallocateBytes(aligned, 64);
  nuostl::NuoBlockPool::DeallocateBytes(nullptr, 64);
}

void CheckAllocatorRequirements()
{
  using Allocator = nuostl::NuoPoolAllocator<int>;
  static_assert(std::is_same_v<Allocator::value_type, int>);
  static_assert(std::is_same_v<
    std::allocator_traits<Allocator>::rebind_alloc<double>,
    nuostl::NuoPoolAllocator<double>>);
  static_assert(std::allocator_traits<Allocator>::is_always_equal::value);
  static_assert(std::is_default_constructible_v<Allocator>);

  Allocator allocator;
  assert(allocator.max_size() > 0);

  int* single = allocator.allocate(1);
  assert(single != nullptr);
  assert(Allocator::Pool().BlocksInUse() == 1);
  *single = 7;
  assert(*single == 7);
  allocator.deallocate(single, 1);
  assert(Allocator::Pool().BlocksInUse() == 0);

  /* Array requests are served by the global allocator. */
  int* array = allocator.allocate(64);
  for (int index = 0; index < 64; ++index)
  {
    array[index] = index;
  }
  assert(array[63] == 63);
  assert(Allocator::Pool().BlocksInUse() == 0);
  allocator.deallocate(array, 64);

  int* nothing = allocator.allocate(0);
  allocator.deallocate(nothing, 0);

  Allocator copy(allocator);
  assert(copy == allocator);
  assert(!(copy != allocator));
  assert(Allocator::Pool().BlocksInUse() == 0);

  /* Rebinding keeps the block count parameter. */
  using DoubleAllocator =
    std::allocator_traits<Allocator>::rebind_alloc<double>;
  DoubleAllocator doubles;
  double* value = doubles.allocate(1);
  *value = 2.5;
  assert(*value == 2.5);
  doubles.deallocate(value, 1);
}

void CheckOverAlignedObjects()
{
  using Allocator = nuostl::NuoPoolAllocator<OverAligned>;
  Allocator allocator;

  OverAligned* single = allocator.allocate(1);
  assert(reinterpret_cast<std::uintptr_t>(single) %
           alignof(OverAligned) == 0);
  single->payload[0] = 1;
  assert(single->payload[0] == 1);
  allocator.deallocate(single, 1);

  OverAligned* array = allocator.allocate(8);
  assert(reinterpret_cast<std::uintptr_t>(array) %
           alignof(OverAligned) == 0);
  allocator.deallocate(array, 8);

  assert(Allocator::Pool().BlocksInUse() == 0);
}

void CheckNodeAllocator()
{
  using NodeAllocator = nuostl::NuoPoolAllocator<Node, 32>;
  static_assert(std::is_same_v<
    std::allocator_traits<NodeAllocator>::rebind_alloc<int>,
    nuostl::NuoPoolAllocator<int, 32>>);

  assert(NodeAllocator::Pool().BlocksInUse() == 0);
  std::vector<Node*> nodes;
  NodeAllocator allocator;
  for (int index = 0; index < 200; ++index)
  {
    Node* node = allocator.allocate(1);
    std::construct_at(node, Node{nullptr, index});
    nodes.push_back(node);
  }
  assert(NodeAllocator::Pool().BlocksInUse() == 200);
  assert(std::all_of(nodes.begin(), nodes.end(),
                     [](const Node* node)
                     {
                       return node != nullptr;
                     }));
  for (Node* node : nodes)
  {
    std::destroy_at(node);
    allocator.deallocate(node, 1);
  }
  assert(NodeAllocator::Pool().BlocksInUse() == 0);
}

void CheckStandardContainers()
{
  using Allocator = nuostl::NuoPoolAllocator<int>;

  const std::size_t outstanding = nuostl::NuoBlockPool::TotalBlocksInUse();
  {
    std::list<int, Allocator> values;
    for (int index = 0; index < 200; ++index)
    {
      values.push_back(index);
    }
    assert(values.size() == 200);
    assert(values.front() == 0 && values.back() == 199);
    values.pop_front();
    values.pop_back();
    assert(values.size() == 198);
    values.clear();
    for (int index = 0; index < 40; ++index)
    {
      values.push_front(index);
    }
    assert(values.size() == 40 && values.front() == 39);
  }
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == outstanding);

  {
    std::vector<int, Allocator> values;
    for (int index = 0; index < 1000; ++index)
    {
      values.push_back(index * 3);
    }
    assert(values.size() == 1000);
    assert(values.front() == 0 && values.back() == 2997);
    values.clear();
    values.shrink_to_fit();
  }
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == outstanding);

  {
    nuostl::nuo_vector<int, Allocator> values;
    for (int index = 0; index < 500; ++index)
    {
      values.push_back(index);
    }
    assert(values.size() == 500);
    assert(values[0] == 0 && values[499] == 499);
    values.shrink_to_fit();
    assert(values[250] == 250);
  }
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == outstanding);

  using TrackedAllocator = nuostl::NuoPoolAllocator<TrackedValue>;
  {
    std::list<TrackedValue, TrackedAllocator> values(64);
    assert(TrackedValue::live_count == 64);
    values.clear();
    assert(TrackedValue::live_count == 0);
  }
  assert(TrackedValue::live_count == 0);
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == outstanding);
}

} /* namespace */

void TestNuoPoolAllocator::TestSuite()
{
  CheckRawPool();
  CheckAllocatorRequirements();
  CheckOverAlignedObjects();
  CheckNodeAllocator();
  CheckStandardContainers();
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == 0);
}

} /* namespace test */
