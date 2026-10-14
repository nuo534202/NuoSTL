#pragma once

#include <cstddef>
#include <cstdint>

namespace nuostl
{

/* The standard library declaration is
   enum class align_val_t : std::size_t {};
   cppreference.com/w/cpp/memory/align_val_t.html */
enum class NuoAlignValT : std::size_t
{
};

/* The standard library declaration is
   void* align(std::size_t alignment, std::size_t size, void*& ptr,
               std::size_t& space);
   cppreference.com/w/cpp/memory/align.html
   On failure the pointer and the remaining space are left unchanged. */
inline void* NuoAlign(std::size_t alignment, std::size_t size,
                      void*& pointer, std::size_t& space)
{
  const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
  const std::size_t misalignment =
    static_cast<std::size_t>(address % alignment);
  const std::size_t adjustment =
    misalignment == 0 ? 0 : alignment - misalignment;
  if (adjustment > space || size > space - adjustment)
  {
    return nullptr;
  }
  pointer = static_cast<unsigned char*>(pointer) + adjustment;
  space -= adjustment;
  return pointer;
}

} /* namespace nuostl */
