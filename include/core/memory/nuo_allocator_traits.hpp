#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_pointer_traits.hpp"
#include "core/memory/nuo_uninitialized.hpp"

namespace nuostl
{

namespace detail
{

template <typename Alloc, typename = void>
struct NuoAllocatorPointer
{
  using type = typename Alloc::value_type*;
};

template <typename Alloc>
struct NuoAllocatorPointer<Alloc, std::void_t<typename Alloc::pointer>>
{
  using type = typename Alloc::pointer;
};

template <typename Alloc, typename = void>
struct NuoAllocatorConstPointer
{
  using type = typename NuoPointerTraits<
    typename NuoAllocatorPointer<Alloc>::type>::template rebind<
      const typename Alloc::value_type>;
};

template <typename Alloc>
struct NuoAllocatorConstPointer<
  Alloc, std::void_t<typename Alloc::const_pointer>>
{
  using type = typename Alloc::const_pointer;
};

template <typename Alloc, typename = void>
struct NuoAllocatorVoidPointer
{
  using type = typename NuoPointerTraits<
    typename NuoAllocatorPointer<Alloc>::type>::template rebind<void>;
};

template <typename Alloc>
struct NuoAllocatorVoidPointer<
  Alloc, std::void_t<typename Alloc::void_pointer>>
{
  using type = typename Alloc::void_pointer;
};

template <typename Alloc, typename = void>
struct NuoAllocatorConstVoidPointer
{
  using type = typename NuoPointerTraits<
    typename NuoAllocatorPointer<Alloc>::type>::template rebind<const void>;
};

template <typename Alloc>
struct NuoAllocatorConstVoidPointer<
  Alloc, std::void_t<typename Alloc::const_void_pointer>>
{
  using type = typename Alloc::const_void_pointer;
};

template <typename Alloc, typename = void>
struct NuoAllocatorDifferenceType
{
  using type = typename NuoPointerTraits<
    typename NuoAllocatorPointer<Alloc>::type>::difference_type;
};

template <typename Alloc>
struct NuoAllocatorDifferenceType<
  Alloc, std::void_t<typename Alloc::difference_type>>
{
  using type = typename Alloc::difference_type;
};

template <typename Alloc, typename = void>
struct NuoAllocatorSizeType
{
  using type = std::make_unsigned_t<
    typename NuoAllocatorDifferenceType<Alloc>::type>;
};

template <typename Alloc>
struct NuoAllocatorSizeType<Alloc, std::void_t<typename Alloc::size_type>>
{
  using type = typename Alloc::size_type;
};

template <typename Alloc, typename = void>
struct NuoAllocatorCopyAssignment
{
  using type = std::false_type;
};

template <typename Alloc>
struct NuoAllocatorCopyAssignment<
  Alloc, std::void_t<
    typename Alloc::propagate_on_container_copy_assignment>>
{
  using type = typename Alloc::propagate_on_container_copy_assignment;
};

template <typename Alloc, typename = void>
struct NuoAllocatorMoveAssignment
{
  using type = std::false_type;
};

template <typename Alloc>
struct NuoAllocatorMoveAssignment<
  Alloc, std::void_t<
    typename Alloc::propagate_on_container_move_assignment>>
{
  using type = typename Alloc::propagate_on_container_move_assignment;
};

template <typename Alloc, typename = void>
struct NuoAllocatorSwap
{
  using type = std::false_type;
};

template <typename Alloc>
struct NuoAllocatorSwap<
  Alloc, std::void_t<typename Alloc::propagate_on_container_swap>>
{
  using type = typename Alloc::propagate_on_container_swap;
};

template <typename Alloc, typename = void>
struct NuoAllocatorAlwaysEqual
{
  using type = std::is_empty<Alloc>;
};

template <typename Alloc>
struct NuoAllocatorAlwaysEqual<
  Alloc, std::void_t<typename Alloc::is_always_equal>>
{
  using type = typename Alloc::is_always_equal;
};

template <typename Alloc, typename U>
struct NuoAllocatorRebindFallback
{
};

template <template <typename, typename...> class Alloc,
          typename T, typename U, typename... Args>
struct NuoAllocatorRebindFallback<Alloc<T, Args...>, U>
{
  using type = Alloc<U, Args...>;
};

template <typename Alloc, typename U, typename = void>
struct NuoAllocatorRebind
{
  using type = typename NuoAllocatorRebindFallback<Alloc, U>::type;
};

template <typename Alloc, typename U>
struct NuoAllocatorRebind<
  Alloc, U,
  std::void_t<typename Alloc::template rebind<U>::other>>
{
  using type = typename Alloc::template rebind<U>::other;
};

} /* namespace detail */

/* The standard library declaration is
   template <typename Alloc> struct allocator_traits;
   cppreference.com/w/cpp/memory/allocator_traits.html
   Optional allocator members fall back to the standard defaults. */
template <typename Alloc>
struct NuoAllocatorTraits
{
  using allocator_type = Alloc;
  using value_type = typename Alloc::value_type;
  using pointer = typename detail::NuoAllocatorPointer<Alloc>::type;
  using const_pointer =
    typename detail::NuoAllocatorConstPointer<Alloc>::type;
  using void_pointer =
    typename detail::NuoAllocatorVoidPointer<Alloc>::type;
  using const_void_pointer =
    typename detail::NuoAllocatorConstVoidPointer<Alloc>::type;
  using difference_type =
    typename detail::NuoAllocatorDifferenceType<Alloc>::type;
  using size_type = typename detail::NuoAllocatorSizeType<Alloc>::type;

  using propagate_on_container_copy_assignment =
    typename detail::NuoAllocatorCopyAssignment<Alloc>::type;
  using propagate_on_container_move_assignment =
    typename detail::NuoAllocatorMoveAssignment<Alloc>::type;
  using propagate_on_container_swap =
    typename detail::NuoAllocatorSwap<Alloc>::type;
  using is_always_equal =
    typename detail::NuoAllocatorAlwaysEqual<Alloc>::type;

  template <typename U>
  using rebind_alloc = typename detail::NuoAllocatorRebind<Alloc, U>::type;

  template <typename U>
  using rebind_traits = NuoAllocatorTraits<rebind_alloc<U>>;

  static pointer allocate(Alloc& allocator, size_type count)
  {
    return allocator.allocate(count);
  }

  static pointer allocate(Alloc& allocator, size_type count,
                          const_void_pointer hint)
  {
    if constexpr (requires { allocator.allocate(count, hint); })
    {
      return allocator.allocate(count, hint);
    }
    else
    {
      return allocator.allocate(count);
    }
  }

  static void deallocate(Alloc& allocator, pointer ptr, size_type count)
  {
    allocator.deallocate(ptr, count);
  }

  template <typename T, typename... Args>
  static void construct(Alloc& allocator, T* ptr, Args&&... args)
  {
    if constexpr (requires
                  {
                    allocator.construct(ptr, std::forward<Args>(args)...);
                  })
    {
      allocator.construct(ptr, std::forward<Args>(args)...);
    }
    else
    {
      NuoConstructAt(ptr, std::forward<Args>(args)...);
    }
  }

  template <typename T>
  static void destroy(Alloc& allocator, T* ptr)
  {
    if constexpr (requires { allocator.destroy(ptr); })
    {
      allocator.destroy(ptr);
    }
    else
    {
      NuoDestroyAt(ptr);
    }
  }

  static size_type max_size(const Alloc& allocator) noexcept
  {
    if constexpr (requires { allocator.max_size(); })
    {
      return allocator.max_size();
    }
    else
    {
      return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
  }

  static Alloc select_on_container_copy_construction(
    const Alloc& allocator)
  {
    if constexpr (requires
                  {
                    allocator.select_on_container_copy_construction();
                  })
    {
      return allocator.select_on_container_copy_construction();
    }
    else
    {
      return allocator;
    }
  }
};

} /* namespace nuostl */
