#pragma once

#include <stddef.h>

#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_type_traits.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

/*****************************************************************************/
/* array class definition */
template <class T, size_t N>
struct NuoArray
{
  /* types */
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  using iterator        = T*;
  using const_iterator  = const T*;
  using reverse_iterator = NuoReverseIterator<iterator>;
  using const_reverse_iterator = NuoReverseIterator<const_iterator>;

  /* public `elems` member keeps the class an aggregate, so brace-init
     `NuoArray<int, 3> a = {1, 2, 3};` works like `std::array`.
     When N == 0 the storage is a dummy element of size 1. */
  T elems[N > 0 ? N : 1]; /* underlying element storage */

  /* no explicit construct/copy/destroy for aggregate type */

  /* modifiers */
  constexpr void Fill(const T& value);
  constexpr void Swap(NuoArray& rhs) noexcept(NuoIsNothrowSwappable<T>::kValue);

  /* iterators */
  constexpr iterator Begin() noexcept;
  constexpr const_iterator Begin() const noexcept;
  constexpr iterator End() noexcept;
  constexpr const_iterator End() const noexcept;

  constexpr reverse_iterator RBegin() noexcept;
  constexpr const_reverse_iterator RBegin() const noexcept;
  constexpr reverse_iterator REnd() noexcept;
  constexpr const_reverse_iterator REnd() const noexcept;

  constexpr const_iterator CBegin() const noexcept;
  constexpr const_iterator CEnd() const noexcept;
  constexpr const_reverse_iterator CRBegin() const noexcept;
  constexpr const_reverse_iterator CREnd() const noexcept;

  /* capacity */
  constexpr bool Empty() const noexcept;
  constexpr size_type Size() const noexcept;
  constexpr size_type MaxSize() const noexcept;

  /* element access */
  constexpr reference operator[](size_type n);
  constexpr const_reference operator[](size_type n) const;
  constexpr reference At(size_type n);
  constexpr const_reference At(size_type n) const;
  constexpr reference Front();
  constexpr const_reference Front() const;
  constexpr reference Back();
  constexpr const_reference Back() const;

  constexpr T* Data() noexcept;
  constexpr const T* Data() const noexcept;
};

/* overload NuoSwap */
template <class T, size_t N>
void NuoSwap(
  NuoArray<T, N>& lhs,
  NuoArray<T, N>& rhs) noexcept(NuoIsNothrowSwappable<T>::kValue)
{
  lhs.Swap(rhs);
}

/* overload comparison operators */
template <class T, size_t N>
bool operator==(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return NuoEqual(lhs.Begin(), lhs.End(), rhs.Begin());
}

template <class T, size_t N>
bool operator!=(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return !(lhs == rhs);
}

template <class T, size_t N>
bool operator<(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return NuoLexicographicalCompare(
    lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template <class T, size_t N>
bool operator>(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return rhs < lhs;
}

template <class T, size_t N>
bool operator<=(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return !(rhs < lhs);
}

template <class T, size_t N>
bool operator>=(const NuoArray<T, N>& lhs, const NuoArray<T, N>& rhs)
{
  return !(lhs < rhs);
}

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template <class T, class... U>
NuoArray(T, U...) -> NuoArray<T, 1 + sizeof...(U)>;
#endif

/*****************************************************************************/
/* array class implementation */
template <class T, size_t N>
constexpr void NuoArray<T, N>::Fill(const T& value)
{
  NuoFill(Begin(), End(), value);
}

template <class T, size_t N>
constexpr void NuoArray<T, N>::Swap(NuoArray& rhs) noexcept(
  NuoIsNothrowSwappable<T>::kValue)
{
  for (size_type i = 0; i < N; i++)
    NuoSwap(elems[i], rhs.elems[i]);
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::iterator
NuoArray<T, N>::Begin() noexcept
{
  return elems;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_iterator
NuoArray<T, N>::Begin() const noexcept
{
  return elems;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::iterator
NuoArray<T, N>::End() noexcept
{
  return elems + N;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_iterator
NuoArray<T, N>::End() const noexcept
{
  return elems + N;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reverse_iterator
NuoArray<T, N>::RBegin() noexcept
{
  return reverse_iterator(End());
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reverse_iterator
NuoArray<T, N>::RBegin() const noexcept
{
  return const_reverse_iterator(End());
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reverse_iterator
NuoArray<T, N>::REnd() noexcept
{
  return reverse_iterator(Begin());
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reverse_iterator
NuoArray<T, N>::REnd() const noexcept
{
  return const_reverse_iterator(Begin());
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_iterator
NuoArray<T, N>::CBegin() const noexcept
{
  return Begin();
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_iterator
NuoArray<T, N>::CEnd() const noexcept
{
  return End();
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reverse_iterator
NuoArray<T, N>::CRBegin() const noexcept
{
  return const_reverse_iterator(End());
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reverse_iterator
NuoArray<T, N>::CREnd() const noexcept
{
  return const_reverse_iterator(Begin());
}

template <class T, size_t N>
constexpr bool NuoArray<T, N>::Empty() const noexcept
{
  return Size() == 0;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::size_type
NuoArray<T, N>::Size() const noexcept
{
  return N;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::size_type
NuoArray<T, N>::MaxSize() const noexcept
{
  return N;
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reference
NuoArray<T, N>::operator[](size_type n)
{
  return elems[n];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reference
NuoArray<T, N>::operator[](size_type n) const
{
  return elems[n];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reference
NuoArray<T, N>::At(size_type n)
{
  NUO_THROW_OUT_OF_RANGE_IF(n >= N, "NuoArray: index out of range");
  return elems[n];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reference
NuoArray<T, N>::At(size_type n) const
{
  NUO_THROW_OUT_OF_RANGE_IF(n >= N, "NuoArray: index out of range");
  return elems[n];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reference
NuoArray<T, N>::Front()
{
  return elems[0];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reference
NuoArray<T, N>::Front() const
{
  return elems[0];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::reference
NuoArray<T, N>::Back()
{
  return elems[N - 1];
}

template <class T, size_t N>
constexpr typename NuoArray<T, N>::const_reference
NuoArray<T, N>::Back() const
{
  return elems[N - 1];
}

template <class T, size_t N>
constexpr T* NuoArray<T, N>::Data() noexcept
{
  return elems;
}

template <class T, size_t N>
constexpr const T* NuoArray<T, N>::Data() const noexcept
{
  return elems;
}

} /* namespace nuostl */