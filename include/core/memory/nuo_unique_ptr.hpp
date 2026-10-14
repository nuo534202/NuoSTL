#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace nuostl
{

/* The standard library declaration is
   template <typename T> struct default_delete;
   template <typename T> struct default_delete<T[]>;
   cppreference.com/w/cpp/memory/default_delete.html
   This implementation keeps the same scalar and array behavior. */
template <typename T>
struct NuoDefaultDelete
{
  static_assert(!std::is_void_v<T>, "NuoDefaultDelete cannot delete void");

  constexpr NuoDefaultDelete() noexcept = default;

  template <typename U>
    requires std::is_convertible_v<U*, T*>
  constexpr NuoDefaultDelete(const NuoDefaultDelete<U>&) noexcept
  {
  }

  void operator()(T* pointer) const noexcept
  {
    delete pointer;
  }
};

template <typename T>
struct NuoDefaultDelete<T[]>
{
  static_assert(!std::is_void_v<T>, "NuoDefaultDelete cannot delete void");

  constexpr NuoDefaultDelete() noexcept = default;

  template <typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
  constexpr NuoDefaultDelete(const NuoDefaultDelete<U[]>&) noexcept
  {
  }

  template <typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
  void operator()(U* pointer) const noexcept
  {
    delete[] pointer;
  }
};

/* The standard library class template is
   template <typename T, typename Deleter = std::default_delete<T>>
   class unique_ptr;
   cppreference.com/w/cpp/memory/unique_ptr.html
   This implementation stores a raw pointer and a deleter. */
template <typename T, typename Deleter = NuoDefaultDelete<T>>
class NuoUniquePtr
{
  static_assert(!std::is_array_v<T>,
                "NuoUniquePtr<T> requires a non-array type");

public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = Deleter;

  constexpr NuoUniquePtr() noexcept = default;

  constexpr NuoUniquePtr(std::nullptr_t) noexcept
    requires std::is_default_constructible_v<deleter_type>
    : pointer_(nullptr)
    , deleter_()
  {
  }

  explicit constexpr NuoUniquePtr(pointer pointer) noexcept
    requires std::is_default_constructible_v<deleter_type>
    : pointer_(pointer)
    , deleter_()
  {
  }

  template <typename E>
    requires std::is_constructible_v<deleter_type, E&&>
  constexpr NuoUniquePtr(pointer pointer, E&& deleter) noexcept
    : pointer_(pointer)
    , deleter_(std::forward<E>(deleter))
  {
  }

  constexpr NuoUniquePtr(NuoUniquePtr&& other) noexcept
    : pointer_(other.Release())
    , deleter_(std::forward<deleter_type>(other.GetDeleter()))
  {
  }

  template <typename U, typename E>
    requires std::is_convertible_v<U*, pointer> &&
             std::is_constructible_v<deleter_type, E&&>
  constexpr NuoUniquePtr(NuoUniquePtr<U, E>&& other) noexcept
    : pointer_(other.Release())
    , deleter_(std::forward<E>(other.GetDeleter()))
  {
  }

  ~NuoUniquePtr() noexcept
  {
    if (pointer_ != nullptr)
    {
      deleter_(pointer_);
    }
  }

  NuoUniquePtr(const NuoUniquePtr&) = delete;
  NuoUniquePtr& operator=(const NuoUniquePtr&) = delete;

  NuoUniquePtr& operator=(NuoUniquePtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset(other.Release());
      deleter_ = std::forward<deleter_type>(other.GetDeleter());
    }
    return *this;
  }

  template <typename U, typename E>
    requires std::is_convertible_v<U*, pointer> &&
             std::is_assignable_v<deleter_type&, E&&>
  NuoUniquePtr& operator=(NuoUniquePtr<U, E>&& other) noexcept
  {
    Reset(other.Release());
    deleter_ = std::forward<E>(other.GetDeleter());
    return *this;
  }

  constexpr NuoUniquePtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  constexpr pointer Get() const noexcept
  {
    return pointer_;
  }

  constexpr deleter_type& GetDeleter() noexcept
  {
    return deleter_;
  }

  constexpr const deleter_type& GetDeleter() const noexcept
  {
    return deleter_;
  }

  constexpr explicit operator bool() const noexcept
  {
    return pointer_ != nullptr;
  }

  constexpr std::add_lvalue_reference_t<T> operator*() const noexcept
    requires (!std::is_void_v<T>)
  {
    return *pointer_;
  }

  constexpr pointer operator->() const noexcept
    requires (!std::is_void_v<T>)
  {
    return pointer_;
  }

  constexpr pointer Release() noexcept
  {
    pointer released = pointer_;
    pointer_ = nullptr;
    return released;
  }

  void Reset(pointer replacement = pointer()) noexcept
  {
    if (replacement != pointer_)
    {
      pointer previous = pointer_;
      pointer_ = replacement;
      if (previous != nullptr)
      {
        deleter_(previous);
      }
    }
  }

  void Swap(NuoUniquePtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(deleter_, other.deleter_);
  }

private:
  pointer pointer_ = nullptr;
  [[no_unique_address]] deleter_type deleter_;
};

/* The standard library array specialization is
   template <typename T, typename Deleter>
   class unique_ptr<T[], Deleter>;
   The array form owns a dynamic array and exposes operator[]. */
template <typename T, typename Deleter>
class NuoUniquePtr<T[], Deleter>
{
public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = Deleter;

  constexpr NuoUniquePtr() noexcept = default;

  constexpr NuoUniquePtr(std::nullptr_t) noexcept
    requires std::is_default_constructible_v<deleter_type>
    : pointer_(nullptr)
    , deleter_()
  {
  }

  explicit constexpr NuoUniquePtr(pointer pointer) noexcept
    requires std::is_default_constructible_v<deleter_type>
    : pointer_(pointer)
    , deleter_()
  {
  }

  template <typename E>
    requires std::is_constructible_v<deleter_type, E&&>
  constexpr NuoUniquePtr(pointer pointer, E&& deleter) noexcept
    : pointer_(pointer)
    , deleter_(std::forward<E>(deleter))
  {
  }

  constexpr NuoUniquePtr(NuoUniquePtr&& other) noexcept
    : pointer_(other.Release())
    , deleter_(std::forward<deleter_type>(other.GetDeleter()))
  {
  }

  template <typename U, typename E>
    requires std::is_convertible_v<U(*)[], T(*)[]> &&
             std::is_constructible_v<deleter_type, E&&>
  constexpr NuoUniquePtr(NuoUniquePtr<U[], E>&& other) noexcept
    : pointer_(other.Release())
    , deleter_(std::forward<E>(other.GetDeleter()))
  {
  }

  ~NuoUniquePtr() noexcept
  {
    if (pointer_ != nullptr)
    {
      deleter_(pointer_);
    }
  }

  NuoUniquePtr(const NuoUniquePtr&) = delete;
  NuoUniquePtr& operator=(const NuoUniquePtr&) = delete;

  NuoUniquePtr& operator=(NuoUniquePtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset(other.Release());
      deleter_ = std::forward<deleter_type>(other.GetDeleter());
    }
    return *this;
  }

  template <typename U, typename E>
    requires std::is_convertible_v<U(*)[], T(*)[]> &&
             std::is_assignable_v<deleter_type&, E&&>
  NuoUniquePtr& operator=(NuoUniquePtr<U[], E>&& other) noexcept
  {
    Reset(other.Release());
    deleter_ = std::forward<E>(other.GetDeleter());
    return *this;
  }

  constexpr NuoUniquePtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  constexpr pointer Get() const noexcept
  {
    return pointer_;
  }

  constexpr deleter_type& GetDeleter() noexcept
  {
    return deleter_;
  }

  constexpr const deleter_type& GetDeleter() const noexcept
  {
    return deleter_;
  }

  constexpr explicit operator bool() const noexcept
  {
    return pointer_ != nullptr;
  }

  constexpr std::add_lvalue_reference_t<T> operator[](
    std::size_t index) const noexcept
    requires (!std::is_void_v<T>)
  {
    return pointer_[index];
  }

  constexpr pointer Release() noexcept
  {
    pointer released = pointer_;
    pointer_ = nullptr;
    return released;
  }

  void Reset(pointer replacement = pointer()) noexcept
  {
    if (replacement != pointer_)
    {
      pointer previous = pointer_;
      pointer_ = replacement;
      if (previous != nullptr)
      {
        deleter_(previous);
      }
    }
  }

  void Swap(NuoUniquePtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(deleter_, other.deleter_);
  }

private:
  pointer pointer_ = nullptr;
  [[no_unique_address]] deleter_type deleter_;
};

template <typename T, typename D>
void NuoSwap(NuoUniquePtr<T, D>& lhs, NuoUniquePtr<T, D>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/* The standard library factories are
   template <typename T, typename... Args>
   unique_ptr<T> make_unique(Args&&... args);
   template <typename T>
   unique_ptr<T> make_unique(std::size_t size);
   cppreference.com/w/cpp/memory/unique_ptr/make_unique.html */
template <typename T, typename... Args>
  requires (!std::is_array_v<T> && !std::is_void_v<T>)
NuoUniquePtr<T> NuoMakeUnique(Args&&... args)
{
  return NuoUniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
  requires (std::is_array_v<T> && std::extent_v<T> == 0)
NuoUniquePtr<T> NuoMakeUnique(std::size_t size)
{
  using Element = std::remove_extent_t<T>;
  return NuoUniquePtr<T>(new Element[size]());
}

/* The C++20 overwrite factories default-initialize their elements. */
template <typename T>
  requires (!std::is_array_v<T> && !std::is_void_v<T>)
NuoUniquePtr<T> NuoMakeUniqueForOverwrite()
{
  return NuoUniquePtr<T>(new T);
}

template <typename T>
  requires (std::is_array_v<T> && std::extent_v<T> == 0)
NuoUniquePtr<T> NuoMakeUniqueForOverwrite(std::size_t size)
{
  using Element = std::remove_extent_t<T>;
  return NuoUniquePtr<T>(new Element[size]);
}

/* Comparisons follow the standard unique_ptr pointer comparisons. */
template <typename T1, typename D1, typename T2, typename D2>
constexpr bool operator==(const NuoUniquePtr<T1, D1>& lhs,
                          const NuoUniquePtr<T2, D2>& rhs) noexcept
  requires requires
  {
    lhs.Get() == rhs.Get();
  }
{
  return lhs.Get() == rhs.Get();
}

template <typename T1, typename D1, typename T2, typename D2>
constexpr auto operator<=>(const NuoUniquePtr<T1, D1>& lhs,
                           const NuoUniquePtr<T2, D2>& rhs) noexcept
  requires requires
  {
    std::compare_three_way{}(lhs.Get(), rhs.Get());
  }
{
  return std::compare_three_way{}(lhs.Get(), rhs.Get());
}

template <typename T, typename D>
constexpr bool operator==(const NuoUniquePtr<T, D>& pointer,
                          std::nullptr_t) noexcept
{
  return !pointer;
}

template <typename T, typename D>
constexpr bool operator==(std::nullptr_t,
                          const NuoUniquePtr<T, D>& pointer) noexcept
{
  return !pointer;
}

template <typename T, typename D>
constexpr std::strong_ordering operator<=>(const NuoUniquePtr<T, D>& pointer,
                                           std::nullptr_t) noexcept
{
  return pointer ? std::strong_ordering::greater
                 : std::strong_ordering::equal;
}

template <typename T, typename D>
constexpr std::strong_ordering operator<=>(std::nullptr_t,
                                           const NuoUniquePtr<T, D>& pointer)
  noexcept
{
  return pointer ? std::strong_ordering::less
                 : std::strong_ordering::equal;
}

template <typename CharT, typename Traits, typename T, typename D>
std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& stream,
  const NuoUniquePtr<T, D>& pointer)
{
  return stream << pointer.Get();
}

} /* namespace nuostl */

namespace std
{

template <typename T, typename D>
struct hash<nuostl::NuoUniquePtr<T, D>>
{
  size_t operator()(const nuostl::NuoUniquePtr<T, D>& pointer) const noexcept
  {
    return hash<typename nuostl::NuoUniquePtr<T, D>::pointer>{}(pointer.Get());
  }
};

} /* namespace std */
