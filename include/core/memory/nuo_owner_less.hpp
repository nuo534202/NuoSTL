#pragma once

#include "core/memory/nuo_shared_ptr.hpp"

namespace nuostl
{

/* The standard library declaration is
   template <typename T = void> struct owner_less;
   cppreference.com/w/cpp/memory/owner_less.html
   The comparator orders pointers by control block ownership. */
template <typename T = void>
struct NuoOwnerLess;

template <typename T>
struct NuoOwnerLess
{
  bool operator()(const NuoSharedPtr<T>& lhs,
                  const NuoSharedPtr<T>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  bool operator()(const NuoSharedPtr<T>& lhs,
                  const NuoWeakPtr<T>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  bool operator()(const NuoWeakPtr<T>& lhs,
                  const NuoSharedPtr<T>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  bool operator()(const NuoWeakPtr<T>& lhs,
                  const NuoWeakPtr<T>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }
};

template <>
struct NuoOwnerLess<void>
{
  template <typename T, typename U>
  bool operator()(const NuoSharedPtr<T>& lhs,
                  const NuoSharedPtr<U>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  template <typename T, typename U>
  bool operator()(const NuoSharedPtr<T>& lhs,
                  const NuoWeakPtr<U>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  template <typename T, typename U>
  bool operator()(const NuoWeakPtr<T>& lhs,
                  const NuoSharedPtr<U>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }

  template <typename T, typename U>
  bool operator()(const NuoWeakPtr<T>& lhs,
                  const NuoWeakPtr<U>& rhs) const noexcept
  {
    return lhs.OwnerBefore(rhs);
  }
};

} /* namespace nuostl */
