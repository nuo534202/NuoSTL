#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_pointer_traits.hpp"

namespace nuostl
{

inline constexpr std::size_t NuoDynamicExtent =
  static_cast<std::size_t>(-1);

template <typename T, std::size_t Extent = NuoDynamicExtent>
class NuoSpan;

namespace detail
{

template <typename T>
struct IsNuoSpan : std::false_type
{
};

template <typename T, std::size_t Extent>
struct IsNuoSpan<NuoSpan<T, Extent>> : std::true_type
{
};

template <typename T>
struct IsStdArray : std::false_type
{
};

template <typename T, std::size_t N>
struct IsStdArray<std::array<T, N>> : std::true_type
{
};

template <typename It, typename T>
concept NuoCompatibleContiguousIterator =
  std::contiguous_iterator<It> &&
  std::is_convertible_v<
    std::remove_reference_t<std::iter_reference_t<It>>(*)[], T(*)[]>;

} /* namespace detail */

/* The standard library declaration is
   template <typename T, std::size_t Extent = std::dynamic_extent>
   class span;
   cppreference.com/w/cpp/container/span.html
   A span is a non-owning view over a contiguous sequence of objects. */
template <typename T, std::size_t Extent>
class NuoSpan
{
  static_assert(std::is_object_v<T>,
                "NuoSpan element type must be an object type");
  static_assert(!std::is_abstract_v<T>,
                "NuoSpan element type cannot be abstract");
  static_assert(!std::is_array_v<T>,
                "NuoSpan element type cannot be an array type");

public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;

  static constexpr size_type extent = Extent;

  constexpr NuoSpan() noexcept
    requires (Extent == NuoDynamicExtent || Extent == 0)
    : data_(nullptr)
    , size_(Extent == NuoDynamicExtent ? 0 : Extent)
  {
  }

  template <typename U, size_type N>
    requires (std::is_convertible_v<U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  constexpr NuoSpan(U (&array)[N]) noexcept
    : data_(array)
    , size_(N)
  {
  }

  template <typename U, size_type N>
    requires (std::is_convertible_v<U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  constexpr NuoSpan(std::array<U, N>& array) noexcept
    : data_(array.data())
    , size_(N)
  {
  }

  template <typename U, size_type N>
    requires (std::is_convertible_v<const U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  constexpr NuoSpan(const std::array<U, N>& array) noexcept
    : data_(array.data())
    , size_(N)
  {
  }

  template <typename U, size_type N>
    requires (std::is_convertible_v<U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  NuoSpan(std::array<U, N>&&) = delete;

  template <typename U, size_type N>
    requires (std::is_convertible_v<const U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  NuoSpan(const std::array<U, N>&&) = delete;

  template <typename U, size_type N>
    requires (std::is_convertible_v<U(*)[], T(*)[]> &&
              (Extent == NuoDynamicExtent || N == Extent))
  constexpr NuoSpan(const NuoSpan<U, N>& other) noexcept
    : data_(other.Data())
    , size_(other.Size())
  {
  }

  constexpr NuoSpan(pointer pointer, size_type count) noexcept
    requires (Extent == NuoDynamicExtent)
    : data_(pointer)
    , size_(count)
  {
  }

  template <typename It, typename End>
    requires (Extent == NuoDynamicExtent &&
              detail::NuoCompatibleContiguousIterator<It, T> &&
              std::sized_sentinel_for<End, It>)
  constexpr NuoSpan(It first, End last) noexcept
    : data_(NuoToAddress(first))
    , size_(static_cast<size_type>(last - first))
  {
  }

  template <typename Container>
    requires (Extent == NuoDynamicExtent &&
              !detail::IsNuoSpan<std::remove_cvref_t<Container>>::value &&
              !detail::IsStdArray<std::remove_cvref_t<Container>>::value &&
              requires(Container& container)
              {
                { container.data() } -> std::convertible_to<pointer>;
                { container.size() } -> std::convertible_to<size_type>;
              })
  constexpr NuoSpan(Container& container) noexcept
    : data_(container.data())
    , size_(static_cast<size_type>(container.size()))
  {
  }

  template <typename Container>
    requires (Extent == NuoDynamicExtent &&
              !detail::IsNuoSpan<std::remove_cvref_t<Container>>::value &&
              !detail::IsStdArray<std::remove_cvref_t<Container>>::value &&
              requires(const Container& container)
              {
                { container.data() } -> std::convertible_to<pointer>;
                { container.size() } -> std::convertible_to<size_type>;
              })
  constexpr NuoSpan(const Container& container) noexcept
    : data_(container.data())
    , size_(static_cast<size_type>(container.size()))
  {
  }

  template <typename Container>
    requires (Extent == NuoDynamicExtent &&
              !std::is_lvalue_reference_v<Container> &&
              !detail::IsNuoSpan<std::remove_cvref_t<Container>>::value &&
              !detail::IsStdArray<std::remove_cvref_t<Container>>::value &&
              requires(Container&& container)
              {
                { container.data() } -> std::convertible_to<pointer>;
                { container.size() } -> std::convertible_to<size_type>;
              })
  NuoSpan(Container&&) = delete;

  constexpr size_type Size() const noexcept
  {
    return size_;
  }

  constexpr size_type SizeBytes() const noexcept
  {
    return size_ * sizeof(element_type);
  }

  constexpr bool Empty() const noexcept
  {
    return size_ == 0;
  }

  constexpr pointer Data() const noexcept
  {
    return data_;
  }

  constexpr iterator Begin() const noexcept
  {
    return data_;
  }

  constexpr iterator End() const noexcept
  {
    return data_ + size_;
  }

  constexpr reverse_iterator RBegin() const noexcept
  {
    return reverse_iterator(End());
  }

  constexpr reverse_iterator REnd() const noexcept
  {
    return reverse_iterator(Begin());
  }

  constexpr reference operator[](size_type index) const noexcept
  {
    return data_[index];
  }

  constexpr reference Front() const noexcept
  {
    return data_[0];
  }

  constexpr reference Back() const noexcept
  {
    return data_[size_ - 1];
  }

  constexpr NuoSpan<T, NuoDynamicExtent> First(size_type count) const noexcept
  {
    return NuoSpan<T, NuoDynamicExtent>(data_, count);
  }

  constexpr NuoSpan<T, NuoDynamicExtent> Last(size_type count) const noexcept
  {
    return NuoSpan<T, NuoDynamicExtent>(data_ + (size_ - count), count);
  }

  constexpr NuoSpan<T, NuoDynamicExtent> Subspan(
    size_type offset,
    size_type count = NuoDynamicExtent) const noexcept
  {
    return NuoSpan<T, NuoDynamicExtent>(
      data_ + offset,
      count == NuoDynamicExtent ? size_ - offset : count);
  }

private:
  pointer data_;
  size_type size_;
};

template <typename T, std::size_t N>
NuoSpan(T (&)[N]) -> NuoSpan<T, N>;

template <typename T, std::size_t N>
NuoSpan(std::array<T, N>&) -> NuoSpan<T, N>;

template <typename T, std::size_t N>
NuoSpan(const std::array<T, N>&) -> NuoSpan<const T, N>;

template <typename Container>
  requires (!detail::IsStdArray<std::remove_cvref_t<Container>>::value &&
            !detail::IsNuoSpan<std::remove_cvref_t<Container>>::value)
NuoSpan(Container&) -> NuoSpan<typename Container::value_type,
                               NuoDynamicExtent>;

template <typename Container>
  requires (!detail::IsStdArray<std::remove_cvref_t<Container>>::value &&
            !detail::IsNuoSpan<std::remove_cvref_t<Container>>::value)
NuoSpan(const Container&) -> NuoSpan<const typename Container::value_type,
                                     NuoDynamicExtent>;

} /* namespace nuostl */
