#pragma once

#include <stddef.h>

#include <new>
#include <type_traits>

#include "utils/nuo_block_pool.hpp"

namespace nuostl
{

/* An allocator that serves single objects from a shared pool of equally sized
   blocks and falls back to the global allocator for array requests. Every
   specialization keeps its own pool, so blocks of different types never mix. */
template <typename T, size_t BlocksPerChunk = 64>
class NuoPoolAllocator
{
public:
  using value_type      = T;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  using pointer         = T*;
  using const_pointer   = const T*;
  using reference       = T&;
  using const_reference = const T&;

  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  template <typename U>
  struct rebind
  {
    using other = NuoPoolAllocator<U, BlocksPerChunk>;
  };

  NuoPoolAllocator() noexcept = default;

  template <typename U>
  NuoPoolAllocator(const NuoPoolAllocator<U, BlocksPerChunk>&) noexcept
  {
  }

  /* One block comes from the pool, larger requests use the global allocator. */
  pointer allocate(size_type n);
  void deallocate(pointer pointer, size_type n);

  size_type max_size() const noexcept;

  static NuoBlockPool& Pool() noexcept;
};

template <typename T, size_t BlocksPerChunk>
bool operator==(const NuoPoolAllocator<T, BlocksPerChunk>&,
                const NuoPoolAllocator<T, BlocksPerChunk>&) noexcept
{
  return true;
}

template <typename T, size_t BlocksPerChunk>
bool operator!=(const NuoPoolAllocator<T, BlocksPerChunk>&,
                const NuoPoolAllocator<T, BlocksPerChunk>&) noexcept
{
  return false;
}

template <typename T, size_t BlocksPerChunk>
typename NuoPoolAllocator<T, BlocksPerChunk>::pointer
NuoPoolAllocator<T, BlocksPerChunk>::allocate(size_type n)
{
  if (n == 0)
  {
    return nullptr;
  }
  if (n > max_size())
  {
    throw std::bad_array_new_length();
  }
  if (n == 1)
  {
    return static_cast<pointer>(Pool().Allocate());
  }
  return static_cast<pointer>(
    NuoBlockPool::AllocateBytes(n * sizeof(T), alignof(T)));
}

template <typename T, size_t BlocksPerChunk>
void NuoPoolAllocator<T, BlocksPerChunk>::deallocate(pointer pointer,
                                                     size_type n)
{
  if (pointer == nullptr)
  {
    return;
  }
  if (n == 1)
  {
    Pool().Deallocate(pointer);
    return;
  }
  NuoBlockPool::DeallocateBytes(pointer, alignof(T));
}

template <typename T, size_t BlocksPerChunk>
typename NuoPoolAllocator<T, BlocksPerChunk>::size_type
NuoPoolAllocator<T, BlocksPerChunk>::max_size() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(T);
}

template <typename T, size_t BlocksPerChunk>
NuoBlockPool& NuoPoolAllocator<T, BlocksPerChunk>::Pool() noexcept
{
  static NuoBlockPool pool(sizeof(T), alignof(T), BlocksPerChunk);
  return pool;
}

} /* namespace nuostl */
