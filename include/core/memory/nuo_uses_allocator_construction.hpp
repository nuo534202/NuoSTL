#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_uninitialized.hpp"
#include "core/memory/nuo_uses_allocator.hpp"

namespace nuostl
{

namespace detail
{

template <typename T>
struct NuoAlwaysFalse
{
  static constexpr bool value = false;
};

template <typename T>
struct NuoIsStdPair : std::false_type
{
};

template <typename T1, typename T2>
struct NuoIsStdPair<std::pair<T1, T2>> : std::true_type
{
};

template <typename T, typename Alloc, typename... Args>
struct NuoAllocatorConstruction;

} /* namespace detail */

/* The standard library declaration is
   template <typename T, typename Alloc, typename... Args>
   constexpr auto uses_allocator_construction_args(const Alloc& alloc,
                                                   Args&&... args) noexcept;
   cppreference.com/w/cpp/memory/uses_allocator_construction_args.html */
template <typename T, typename Alloc, typename... Args>
constexpr auto NuoUsesAllocatorConstructionArgs(const Alloc& alloc,
                                                Args&&... args) noexcept
{
  return detail::NuoAllocatorConstruction<T, Alloc, Args...>::Get(
    alloc, std::forward<Args>(args)...);
}

namespace detail
{

/* General case for types that are not std::pair. */
template <typename T, typename Alloc, typename... Args>
struct NuoAllocatorConstruction
{
  static constexpr auto Get(const Alloc& alloc, Args&&... args) noexcept
  {
    static_assert(!NuoIsStdPair<T>::value,
                  "pair requires two arguments or piecewise_construct");
    if constexpr (!NuoUsesAllocatorV<T, Alloc>)
    {
      return std::forward_as_tuple(std::forward<Args>(args)...);
    }
    else if constexpr (std::is_constructible_v<T, NuoAllocatorArgT,
                                               const Alloc&, Args...>)
    {
      return std::tuple<NuoAllocatorArgT, const Alloc&, Args&&...>(
        NuoAllocatorArg, alloc, std::forward<Args>(args)...);
    }
    else if constexpr (std::is_constructible_v<T, std::allocator_arg_t,
                                               const Alloc&, Args...>)
    {
      return std::tuple<std::allocator_arg_t, const Alloc&, Args&&...>(
        std::allocator_arg, alloc, std::forward<Args>(args)...);
    }
    else if constexpr (std::is_constructible_v<T, Args..., const Alloc&>)
    {
      return std::tuple<Args&&..., const Alloc&>(
        std::forward<Args>(args)..., alloc);
    }
    else
    {
      static_assert(NuoAlwaysFalse<T>::value,
                    "T is not allocator constructible from the arguments");
    }
  }
};

template <typename T1, typename T2, typename Alloc>
struct NuoAllocatorConstruction<std::pair<T1, T2>, Alloc>
{
  static constexpr auto Get(const Alloc& alloc) noexcept
  {
    if constexpr (!NuoUsesAllocatorV<T1, Alloc> &&
                  !NuoUsesAllocatorV<T2, Alloc>)
    {
      return std::forward_as_tuple();
    }
    else
    {
      return std::tuple(
        std::piecewise_construct,
        NuoUsesAllocatorConstructionArgs<T1>(alloc),
        NuoUsesAllocatorConstructionArgs<T2>(alloc));
    }
  }
};

template <typename T1, typename T2, typename Alloc, typename U, typename V>
struct NuoAllocatorConstruction<std::pair<T1, T2>, Alloc, U, V>
{
  static constexpr auto Get(const Alloc& alloc, U&& first, V&& second) noexcept
  {
    if constexpr (!NuoUsesAllocatorV<T1, Alloc> &&
                  !NuoUsesAllocatorV<T2, Alloc>)
    {
      return std::forward_as_tuple(std::forward<U>(first),
                                   std::forward<V>(second));
    }
    else
    {
      return std::tuple(
        std::piecewise_construct,
        NuoUsesAllocatorConstructionArgs<T1>(alloc,
                                             std::forward<U>(first)),
        NuoUsesAllocatorConstructionArgs<T2>(alloc,
                                             std::forward<V>(second)));
    }
  }
};

template <typename T1, typename T2, typename Alloc, typename Piecewise,
          typename Tuple1, typename Tuple2>
  requires (std::is_same_v<std::remove_cvref_t<Piecewise>,
                           std::piecewise_construct_t>)
struct NuoAllocatorConstruction<std::pair<T1, T2>, Alloc, Piecewise,
                                Tuple1, Tuple2>
{
  static constexpr auto Get(const Alloc& alloc,
                            Piecewise,
                            Tuple1&& first, Tuple2&& second) noexcept
  {
    return std::tuple(
      std::piecewise_construct,
      std::apply(
        [&alloc](auto&&... args)
        {
          return NuoUsesAllocatorConstructionArgs<T1>(
            alloc, std::forward<decltype(args)>(args)...);
        },
        std::forward<Tuple1>(first)),
      std::apply(
        [&alloc](auto&&... args)
        {
          return NuoUsesAllocatorConstructionArgs<T2>(
            alloc, std::forward<decltype(args)>(args)...);
        },
        std::forward<Tuple2>(second)));
  }
};

} /* namespace detail */

/* The standard library declaration is
   template <typename T, typename Alloc, typename... Args>
   constexpr T make_obj_using_allocator(const Alloc& alloc, Args&&... args);
   cppreference.com/w/cpp/memory/make_obj_using_allocator.html */
template <typename T, typename Alloc, typename... Args>
constexpr T NuoMakeObjUsingAllocator(const Alloc& alloc, Args&&... args)
{
  return std::make_from_tuple<T>(
    NuoUsesAllocatorConstructionArgs<T>(alloc,
                                        std::forward<Args>(args)...));
}

/* The standard library declaration is
   template <typename T, typename Alloc, typename... Args>
   constexpr T* uninitialized_construct_using_allocator(
     T* p, const Alloc& alloc, Args&&... args);
   cppreference.com/w/cpp/memory/uninitialized_construct_using_allocator.html */
template <typename T, typename Alloc, typename... Args>
constexpr T* NuoUninitializedConstructUsingAllocator(
  T* location, const Alloc& alloc, Args&&... args)
{
  return std::apply(
    [location](auto&&... elements)
    {
      return NuoConstructAt(location,
                            std::forward<decltype(elements)>(elements)...);
    },
    NuoUsesAllocatorConstructionArgs<T>(alloc,
                                        std::forward<Args>(args)...));
}

} /* namespace nuostl */
