#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "utils/nuo_util.hpp"

namespace nuostl
{

struct NuoNulloptT
{
  explicit constexpr NuoNulloptT(int)
  {
  }
};

inline constexpr NuoNulloptT NuoNullopt{0};

struct NuoInPlaceT
{
  explicit constexpr NuoInPlaceT() = default;
};

inline constexpr NuoInPlaceT NuoInPlace{};

class NuoBadOptionalAccess : public std::logic_error
{
public:
  NuoBadOptionalAccess() : std::logic_error("NuoOptional has no value")
  {
  }
};

template <class T>
class NuoOptional
{
  static_assert(!std::is_reference_v<T>, "NuoOptional cannot hold references");
  static_assert(!std::is_same_v<std::remove_cv_t<T>, NuoNulloptT>,
                "NuoOptional cannot hold NuoNulloptT");
  static_assert(!std::is_same_v<std::remove_cv_t<T>, NuoInPlaceT>,
                "NuoOptional cannot hold NuoInPlaceT");

  union Storage
  {
    unsigned char empty;
    T value;

    constexpr Storage() : empty(0)
    {
    }

    constexpr ~Storage()
    {
    }
  };

public:
  using value_type = T;

  constexpr NuoOptional() noexcept : storage_(), has_value_(false)
  {
  }

  constexpr NuoOptional(NuoNulloptT) noexcept : NuoOptional()
  {
  }

  template <class... Args>
  explicit constexpr NuoOptional(NuoInPlaceT, Args&&... args)
    : storage_(), has_value_(false)
  {
    std::construct_at(&storage_.value, std::forward<Args>(args)...);
    has_value_ = true;
  }

  template <class U = T>
    requires (!std::is_same_v<std::remove_cvref_t<U>, NuoOptional> &&
              !std::is_same_v<std::remove_cvref_t<U>, NuoInPlaceT> &&
              !std::is_same_v<std::remove_cvref_t<U>, NuoNulloptT> &&
              std::is_constructible_v<T, U>)
  constexpr NuoOptional(U&& value)
    : storage_(), has_value_(false)
  {
    std::construct_at(&storage_.value, std::forward<U>(value));
    has_value_ = true;
  }

  constexpr NuoOptional(const NuoOptional& other)
    requires std::is_copy_constructible_v<T>
    : storage_(), has_value_(false)
  {
    if (other.has_value_)
    {
      std::construct_at(&storage_.value, other.storage_.value);
      has_value_ = true;
    }
  }

  constexpr NuoOptional(const NuoOptional&)
    requires (!std::is_copy_constructible_v<T>) = delete;

  constexpr NuoOptional(NuoOptional&& other)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T>
    : storage_(), has_value_(false)
  {
    if (other.has_value_)
    {
      std::construct_at(&storage_.value, std::move(other.storage_.value));
      has_value_ = true;
    }
  }

  constexpr NuoOptional(NuoOptional&&)
    requires (!std::is_move_constructible_v<T>) = delete;

  constexpr ~NuoOptional()
  {
    Reset();
  }

  constexpr NuoOptional& operator=(NuoNulloptT) noexcept
  {
    Reset();
    return *this;
  }

  constexpr NuoOptional& operator=(const NuoOptional& other)
    requires (std::is_copy_constructible_v<T> &&
              std::is_copy_assignable_v<T>)
  {
    if (has_value_ && other.has_value_)
    {
      storage_.value = other.storage_.value;
    }
    else if (other.has_value_)
    {
      std::construct_at(&storage_.value, other.storage_.value);
      has_value_ = true;
    }
    else
    {
      Reset();
    }
    return *this;
  }

  constexpr NuoOptional& operator=(const NuoOptional&)
    requires (!(std::is_copy_constructible_v<T> &&
                std::is_copy_assignable_v<T>)) = delete;

  constexpr NuoOptional& operator=(NuoOptional&& other)
    noexcept(std::is_nothrow_move_constructible_v<T> &&
             std::is_nothrow_move_assignable_v<T>)
    requires (std::is_move_constructible_v<T> &&
              std::is_move_assignable_v<T>)
  {
    if (has_value_ && other.has_value_)
    {
      storage_.value = std::move(other.storage_.value);
    }
    else if (other.has_value_)
    {
      std::construct_at(&storage_.value, std::move(other.storage_.value));
      has_value_ = true;
    }
    else
    {
      Reset();
    }
    return *this;
  }

  template <class U = T>
    requires (std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>)
  constexpr NuoOptional& operator=(U&& value)
  {
    if (has_value_)
    {
      storage_.value = std::forward<U>(value);
    }
    else
    {
      std::construct_at(&storage_.value, std::forward<U>(value));
      has_value_ = true;
    }
    return *this;
  }

  constexpr bool HasValue() const noexcept
  {
    return has_value_;
  }

  constexpr explicit operator bool() const noexcept
  {
    return has_value_;
  }

  constexpr T& Value() &
  {
    if (!has_value_)
    {
      throw NuoBadOptionalAccess();
    }
    return storage_.value;
  }

  constexpr const T& Value() const&
  {
    if (!has_value_)
    {
      throw NuoBadOptionalAccess();
    }
    return storage_.value;
  }

  constexpr T&& Value() &&
  {
    if (!has_value_)
    {
      throw NuoBadOptionalAccess();
    }
    return std::move(storage_.value);
  }

  constexpr const T&& Value() const&&
  {
    if (!has_value_)
    {
      throw NuoBadOptionalAccess();
    }
    return std::move(storage_.value);
  }

  constexpr T& operator*() & noexcept
  {
    return storage_.value;
  }

  constexpr const T& operator*() const& noexcept
  {
    return storage_.value;
  }

  constexpr T&& operator*() && noexcept
  {
    return std::move(storage_.value);
  }

  constexpr const T&& operator*() const&& noexcept
  {
    return std::move(storage_.value);
  }

  constexpr T* operator->() noexcept
  {
    return std::addressof(storage_.value);
  }

  constexpr const T* operator->() const noexcept
  {
    return std::addressof(storage_.value);
  }

  template <class U>
  constexpr T ValueOr(U&& fallback) const&
  {
    return has_value_ ? storage_.value : static_cast<T>(std::forward<U>(fallback));
  }

  template <class U>
  constexpr T ValueOr(U&& fallback) &&
  {
    return has_value_ ? std::move(storage_.value)
                      : static_cast<T>(std::forward<U>(fallback));
  }

  constexpr void Reset() noexcept
  {
    if (has_value_)
    {
      std::destroy_at(&storage_.value);
      has_value_ = false;
    }
  }

  template <class... Args>
  constexpr T& Emplace(Args&&... args)
  {
    Reset();
    std::construct_at(&storage_.value, std::forward<Args>(args)...);
    has_value_ = true;
    return storage_.value;
  }

  constexpr void Swap(NuoOptional& other)
    noexcept(std::is_nothrow_move_constructible_v<T> &&
             std::is_nothrow_swappable_v<T>)
    requires (std::is_move_constructible_v<T> && std::is_swappable_v<T>)
  {
    using std::swap;
    if (has_value_ && other.has_value_)
    {
      swap(storage_.value, other.storage_.value);
    }
    else if (has_value_)
    {
      std::construct_at(&other.storage_.value, std::move(storage_.value));
      other.has_value_ = true;
      Reset();
    }
    else if (other.has_value_)
    {
      std::construct_at(&storage_.value, std::move(other.storage_.value));
      has_value_ = true;
      other.Reset();
    }
  }

private:
  Storage storage_;
  bool has_value_;
};

template <class T>
constexpr bool operator==(const NuoOptional<T>& left,
                          const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a == b; }
{
  return left.HasValue() == right.HasValue() &&
    (!left.HasValue() || *left == *right);
}

template <class T>
constexpr bool operator!=(const NuoOptional<T>& left,
                          const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a == b; }
{
  return !(left == right);
}

template <class T>
constexpr bool operator<(const NuoOptional<T>& left,
                         const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a < b; }
{
  if (!right.HasValue())
  {
    return false;
  }
  return !left.HasValue() || *left < *right;
}

template <class T>
constexpr bool operator>(const NuoOptional<T>& left,
                         const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a < b; }
{
  return right < left;
}

template <class T>
constexpr bool operator<=(const NuoOptional<T>& left,
                          const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a < b; }
{
  return !(right < left);
}

template <class T>
constexpr bool operator>=(const NuoOptional<T>& left,
                          const NuoOptional<T>& right)
  requires requires(const T& a, const T& b) { a < b; }
{
  return !(left < right);
}

template <class T>
constexpr bool operator==(const NuoOptional<T>& value, NuoNulloptT) noexcept
{
  return !value.HasValue();
}

template <class T>
constexpr bool operator==(NuoNulloptT, const NuoOptional<T>& value) noexcept
{
  return !value.HasValue();
}

template <class T>
constexpr bool operator!=(const NuoOptional<T>& value, NuoNulloptT) noexcept
{
  return value.HasValue();
}

template <class T>
constexpr bool operator!=(NuoNulloptT, const NuoOptional<T>& value) noexcept
{
  return value.HasValue();
}

template <class T>
constexpr bool operator<(const NuoOptional<T>&, NuoNulloptT) noexcept
{
  return false;
}

template <class T>
constexpr bool operator<(NuoNulloptT, const NuoOptional<T>& value) noexcept
{
  return value.HasValue();
}

template <class T>
constexpr bool operator>(const NuoOptional<T>& value, NuoNulloptT) noexcept
{
  return value.HasValue();
}

template <class T>
constexpr bool operator>(NuoNulloptT, const NuoOptional<T>&) noexcept
{
  return false;
}

template <class T>
constexpr bool operator<=(const NuoOptional<T>& value, NuoNulloptT) noexcept
{
  return !value.HasValue();
}

template <class T>
constexpr bool operator<=(NuoNulloptT, const NuoOptional<T>&) noexcept
{
  return true;
}

template <class T>
constexpr bool operator>=(const NuoOptional<T>&, NuoNulloptT) noexcept
{
  return true;
}

template <class T>
constexpr bool operator>=(NuoNulloptT, const NuoOptional<T>& value) noexcept
{
  return !value.HasValue();
}

template <class T, class U>
constexpr bool operator==(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element == other; }
{
  return optional.HasValue() && *optional == value;
}

template <class T, class U>
constexpr bool operator==(const U& value, const NuoOptional<T>& optional)
  requires requires(const U& other, const T& element) { other == element; }
{
  return optional == value;
}

template <class T, class U>
constexpr bool operator!=(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element == other; }
{
  return !(optional == value);
}

template <class T, class U>
constexpr bool operator!=(const U& value, const NuoOptional<T>& optional)
  requires requires(const T& element, const U& other) { element == other; }
{
  return !(optional == value);
}

template <class T, class U>
constexpr bool operator<(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element < other; }
{
  return !optional.HasValue() || *optional < value;
}

template <class T, class U>
constexpr bool operator<(const U& value, const NuoOptional<T>& optional)
  requires requires(const U& other, const T& element) { other < element; }
{
  return optional.HasValue() && value < *optional;
}

template <class T, class U>
constexpr bool operator>(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element > other; }
{
  return optional.HasValue() && *optional > value;
}

template <class T, class U>
constexpr bool operator>(const U& value, const NuoOptional<T>& optional)
  requires requires(const U& other, const T& element) { other > element; }
{
  return !optional.HasValue() || value > *optional;
}

template <class T, class U>
constexpr bool operator<=(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element <= other; }
{
  return !optional.HasValue() || *optional <= value;
}

template <class T, class U>
constexpr bool operator<=(const U& value, const NuoOptional<T>& optional)
  requires requires(const U& other, const T& element) { other <= element; }
{
  return optional.HasValue() && value <= *optional;
}

template <class T, class U>
constexpr bool operator>=(const NuoOptional<T>& optional, const U& value)
  requires requires(const T& element, const U& other) { element >= other; }
{
  return !optional.HasValue() || *optional >= value;
}

template <class T, class U>
constexpr bool operator>=(const U& value, const NuoOptional<T>& optional)
  requires requires(const U& other, const T& element) { other >= element; }
{
  return optional.HasValue() && value >= *optional;
}

template <class T>
constexpr void NuoSwap(NuoOptional<T>& left, NuoOptional<T>& right)
  noexcept(noexcept(left.Swap(right)))
  requires requires { left.Swap(right); }
{
  left.Swap(right);
}

} /* namespace nuostl */
