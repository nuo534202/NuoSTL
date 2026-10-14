#pragma once

#include <type_traits>

namespace nuostl
{

/* The standard library declaration is
   struct allocator_arg_t { explicit allocator_arg_t() = default; };
   cppreference.com/w/cpp/memory/allocator_arg_t.html */
struct NuoAllocatorArgT
{
  explicit NuoAllocatorArgT() noexcept = default;
};

inline constexpr NuoAllocatorArgT NuoAllocatorArg{};

/* The standard library declaration is
   template <typename T, typename Alloc> struct uses_allocator;
   cppreference.com/w/cpp/memory/uses_allocator.html */
template <typename T, typename Alloc, typename = void>
struct NuoUsesAllocator : std::false_type
{
};

template <typename T, typename Alloc>
struct NuoUsesAllocator<
  T, Alloc, std::void_t<typename T::allocator_type>>
  : std::bool_constant<
      std::is_convertible_v<Alloc, typename T::allocator_type>>
{
};

template <typename T, typename Alloc>
inline constexpr bool NuoUsesAllocatorV =
  NuoUsesAllocator<T, Alloc>::value;

} /* namespace nuostl */
