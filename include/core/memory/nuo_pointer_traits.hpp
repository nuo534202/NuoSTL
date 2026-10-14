#pragma once

#include <cstddef>
#include <type_traits>

#include "core/memory/nuo_addressof.hpp"

namespace nuostl
{

namespace detail
{

template <typename Ptr, typename = void>
struct NuoPointerTraitsDifference
{
  using type = std::ptrdiff_t;
};

template <typename Ptr>
struct NuoPointerTraitsDifference<
  Ptr, std::void_t<typename Ptr::difference_type>>
{
  using type = typename Ptr::difference_type;
};

template <typename Ptr, typename U, typename = void>
struct NuoPointerTraitsRebind
{
};

template <typename Ptr, typename U>
struct NuoPointerTraitsRebind<
  Ptr, U, std::void_t<typename Ptr::template rebind<U>>>
{
  using type = typename Ptr::template rebind<U>;
};

} /* namespace detail */

/* The standard library declaration is
   template <typename Ptr> struct pointer_traits;
   template <typename T> struct pointer_traits<T*>;
   cppreference.com/w/cpp/memory/pointer_traits.html */
template <typename Ptr>
struct NuoPointerTraits
{
  using element_type = typename Ptr::element_type;
  using difference_type =
    typename detail::NuoPointerTraitsDifference<Ptr>::type;

  template <typename U>
  using rebind = typename detail::NuoPointerTraitsRebind<Ptr, U>::type;

  static Ptr pointer_to(element_type& reference)
  {
    return Ptr::pointer_to(reference);
  }
};

template <typename T>
struct NuoPointerTraits<T*>
{
  using element_type = T;
  using difference_type = std::ptrdiff_t;

  template <typename U>
  using rebind = U*;

  static T* pointer_to(T& reference) noexcept
  {
    return NuoAddressof(reference);
  }
};

/* The standard library declaration is
   template <typename Ptr> constexpr auto to_address(const Ptr& p) noexcept;
   cppreference.com/w/cpp/memory/to_address.html
   Raw pointers are returned directly; other pointer types are followed
   through operator->. */
template <typename T>
constexpr T* NuoToAddress(T* pointer) noexcept
{
  return pointer;
}

template <typename Ptr>
constexpr auto NuoToAddress(const Ptr& pointer) noexcept
{
  return NuoToAddress(pointer.operator->());
}

} /* namespace nuostl */
