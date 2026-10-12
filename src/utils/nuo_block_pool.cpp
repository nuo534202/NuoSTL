#include "utils/nuo_block_pool.hpp"

#include <atomic>
#include <new>

namespace nuostl
{

namespace
{

std::atomic<size_t>& OutstandingBlocks()
{
  static std::atomic<size_t> outstanding{0};
  return outstanding;
}

size_t RoundUp(size_t value, size_t alignment)
{
  const size_t remainder = value % alignment;
  if (remainder == 0)
  {
    return value;
  }
  return value + (alignment - remainder);
}

} /* namespace */

struct NuoBlockPool::Chunk
{
  void* memory;
  Chunk* next;
};

NuoBlockPool::NuoBlockPool(size_t block_size, size_t block_alignment,
                           size_t blocks_per_chunk)
  : block_size_(0)
  , block_alignment_(0)
  , blocks_per_chunk_(0)
  , chunk_count_(0)
  , block_count_(0)
  , free_count_(0)
  , free_list_(nullptr)
  , chunks_(nullptr)
{
  if (block_alignment < alignof(void*))
  {
    block_alignment = alignof(void*);
  }
  if (blocks_per_chunk == 0)
  {
    blocks_per_chunk = 1;
  }

  /* A block must hold a free-list pointer, so the size is rounded up first. */
  size_t minimum = block_size < sizeof(void*) ? sizeof(void*) : block_size;
  block_size_ = RoundUp(minimum, block_alignment);
  block_alignment_ = block_alignment;
  blocks_per_chunk_ = blocks_per_chunk;
}

NuoBlockPool::~NuoBlockPool()
{
  OutstandingBlocks() -= BlocksInUse();
  while (chunks_ != nullptr)
  {
    Chunk* next = chunks_->next;
    DeallocateBytes(chunks_->memory, block_alignment_);
    ::operator delete(chunks_);
    chunks_ = next;
  }
  free_list_ = nullptr;
  chunk_count_ = 0;
  block_count_ = 0;
  free_count_ = 0;
}

void* NuoBlockPool::Allocate()
{
  if (free_list_ == nullptr)
  {
    Grow();
  }
  void* block = free_list_;
  free_list_ = *static_cast<void**>(block);
  --free_count_;
  ++OutstandingBlocks();
  return block;
}

void NuoBlockPool::Deallocate(void* block)
{
  if (block == nullptr)
  {
    return;
  }
  *static_cast<void**>(block) = free_list_;
  free_list_ = block;
  ++free_count_;
  --OutstandingBlocks();
}

void NuoBlockPool::Grow()
{
  if (blocks_per_chunk_ > static_cast<size_t>(-1) / block_size_)
  {
    throw std::bad_alloc();
  }
  const size_t bytes = block_size_ * blocks_per_chunk_;
  void* memory = AllocateBytes(bytes, block_alignment_);

  Chunk* chunk = static_cast<Chunk*>(::operator new(sizeof(Chunk)));
  chunk->memory = memory;
  chunk->next = chunks_;
  chunks_ = chunk;
  ++chunk_count_;

  unsigned char* cursor = static_cast<unsigned char*>(memory);
  for (size_t index = 0; index < blocks_per_chunk_; ++index)
  {
    void* block = cursor + index * block_size_;
    *static_cast<void**>(block) = free_list_;
    free_list_ = block;
  }
  block_count_ += blocks_per_chunk_;
  free_count_ += blocks_per_chunk_;
}

size_t NuoBlockPool::BlockSize() const noexcept
{
  return block_size_;
}

size_t NuoBlockPool::BlockAlignment() const noexcept
{
  return block_alignment_;
}

size_t NuoBlockPool::BlocksPerChunk() const noexcept
{
  return blocks_per_chunk_;
}

size_t NuoBlockPool::ChunkCount() const noexcept
{
  return chunk_count_;
}

size_t NuoBlockPool::BlockCount() const noexcept
{
  return block_count_;
}

size_t NuoBlockPool::FreeBlockCount() const noexcept
{
  return free_count_;
}

size_t NuoBlockPool::BlocksInUse() const noexcept
{
  return block_count_ - free_count_;
}

size_t NuoBlockPool::TotalBlocksInUse() noexcept
{
  return OutstandingBlocks();
}

void* NuoBlockPool::AllocateBytes(size_t bytes, size_t alignment)
{
  if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
  {
    return ::operator new(bytes, std::align_val_t(alignment));
  }
  return ::operator new(bytes);
}

void NuoBlockPool::DeallocateBytes(void* memory, size_t alignment)
{
  if (memory == nullptr)
  {
    return;
  }
  if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
  {
    ::operator delete(memory, std::align_val_t(alignment));
    return;
  }
  ::operator delete(memory);
}

} /* namespace nuostl */
