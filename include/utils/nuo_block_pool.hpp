#pragma once

#include <stddef.h>

namespace nuostl
{

/* A fixed-size block pool. Chunks are carved into equally sized blocks, and
   free blocks are linked through their own storage. One pool must not be used
   from several threads at the same time. */
class NuoBlockPool
{
public:
  explicit NuoBlockPool(size_t block_size,
                        size_t block_alignment = alignof(void*),
                        size_t blocks_per_chunk = 64);
  ~NuoBlockPool();

  NuoBlockPool(const NuoBlockPool&) = delete;
  NuoBlockPool& operator=(const NuoBlockPool&) = delete;

  /* Every call returns exactly one block of the configured size. */
  void* Allocate();
  void Deallocate(void* block);

  size_t BlockSize() const noexcept;
  size_t BlockAlignment() const noexcept;
  size_t BlocksPerChunk() const noexcept;
  size_t ChunkCount() const noexcept;
  size_t BlockCount() const noexcept;
  size_t FreeBlockCount() const noexcept;
  size_t BlocksInUse() const noexcept;

  /* Blocks handed out by every live pool; zero means nothing is outstanding. */
  static size_t TotalBlocksInUse() noexcept;

  /* Raw allocation helpers shared with the pool based allocator. */
  static void* AllocateBytes(size_t bytes, size_t alignment);
  static void DeallocateBytes(void* memory, size_t alignment);

private:
  struct Chunk;

  void Grow();

  size_t block_size_;
  size_t block_alignment_;
  size_t blocks_per_chunk_;
  size_t chunk_count_;
  size_t block_count_;
  size_t free_count_;
  void* free_list_;
  Chunk* chunks_;
};

} /* namespace nuostl */
