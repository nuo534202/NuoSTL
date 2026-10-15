#pragma once

#include <climits>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace nuostl
{

/* The standard library declaration is
   enum class endian { little = ..., big = ..., native = ... };
   cppreference.com/w/cpp/types/endian.html */
enum class NuoEndian
{
  little = 0,
  big = 1,
  native =
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    big
#else
    little
#endif
};

/* The standard library declaration is
   template <typename To, typename From>
   constexpr To bit_cast(const From& from) noexcept;
   cppreference.com/w/cpp/numeric/bit_cast.html */
template <typename To, typename From>
  requires (sizeof(To) == sizeof(From) &&
            std::is_trivially_copyable_v<To> &&
            std::is_trivially_copyable_v<From>)
constexpr To NuoBitCast(const From& value) noexcept
{
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bit_cast(To, value);
#else
  To result;
  std::memcpy(&result, &value, sizeof(To));
  return result;
#endif
}

/* The standard library declaration is
   template <typename T> constexpr T byteswap(T value) noexcept;
   cppreference.com/w/cpp/numeric/byteswap.html */
template <typename T>
  requires (std::is_integral_v<T> || std::is_enum_v<T>)
constexpr T NuoByteswap(T value) noexcept
{
  if constexpr (std::is_enum_v<T>)
  {
    using Underlying = std::underlying_type_t<T>;
    return static_cast<T>(NuoByteswap(static_cast<Underlying>(value)));
  }
  else
  {
    using Unsigned = std::make_unsigned_t<T>;
    const Unsigned unsigned_value = static_cast<Unsigned>(value);
    if constexpr (sizeof(T) == 1)
    {
      return static_cast<T>(unsigned_value);
    }
    else if constexpr (sizeof(T) == 2)
    {
      return static_cast<T>(static_cast<Unsigned>(
        __builtin_bswap16(static_cast<std::uint16_t>(unsigned_value))));
    }
    else if constexpr (sizeof(T) == 4)
    {
      return static_cast<T>(static_cast<Unsigned>(
        __builtin_bswap32(static_cast<std::uint32_t>(unsigned_value))));
    }
    else if constexpr (sizeof(T) == 8)
    {
      return static_cast<T>(static_cast<Unsigned>(
        __builtin_bswap64(static_cast<std::uint64_t>(unsigned_value))));
    }
    else if constexpr (sizeof(T) == 16)
    {
      Unsigned result = 0;
      for (int index = 0; index < 16; ++index)
      {
        result = static_cast<Unsigned>(
          (result << 8) |
          ((unsigned_value >> (8 * index)) & static_cast<Unsigned>(0xFF)));
      }
      return static_cast<T>(result);
    }
    else
    {
      static_assert(sizeof(T) == 0, "unsupported byte size");
    }
  }
}

/* The standard library declaration is
   template <typename T> constexpr int popcount(T value) noexcept;
   cppreference.com/w/cpp/numeric/popcount.html */
template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoPopcount(T value) noexcept
{
  if constexpr (sizeof(T) <= 8)
  {
    return __builtin_popcountll(static_cast<unsigned long long>(value));
  }
  else
  {
    const std::uint64_t low = static_cast<std::uint64_t>(value);
    const std::uint64_t high =
      static_cast<std::uint64_t>(value >> 64);
    return __builtin_popcountll(low) + __builtin_popcountll(high);
  }
}

/* The standard library declaration is
   template <typename T> constexpr int countl_zero(T value) noexcept;
   cppreference.com/w/cpp/numeric/countl_zero.html */
template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoCountlZero(T value) noexcept
{
  constexpr int digits = static_cast<int>(sizeof(T) * CHAR_BIT);
  if constexpr (sizeof(T) <= 8)
  {
    if (value == 0)
    {
      return digits;
    }
    return __builtin_clzll(static_cast<unsigned long long>(value)) -
      (64 - digits);
  }
  else
  {
    const std::uint64_t high = static_cast<std::uint64_t>(value >> 64);
    if (high != 0)
    {
      return __builtin_clzll(high);
    }
    const std::uint64_t low = static_cast<std::uint64_t>(value);
    if (low == 0)
    {
      return digits;
    }
    return 64 + __builtin_clzll(low);
  }
}

/* The standard library declaration is
   template <typename T> constexpr int countr_zero(T value) noexcept;
   cppreference.com/w/cpp/numeric/countr_zero.html */
template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoCountrZero(T value) noexcept
{
  constexpr int digits = static_cast<int>(sizeof(T) * CHAR_BIT);
  if constexpr (sizeof(T) <= 8)
  {
    if (value == 0)
    {
      return digits;
    }
    return __builtin_ctzll(static_cast<unsigned long long>(value));
  }
  else
  {
    const std::uint64_t low = static_cast<std::uint64_t>(value);
    if (low != 0)
    {
      return __builtin_ctzll(low);
    }
    const std::uint64_t high = static_cast<std::uint64_t>(value >> 64);
    if (high == 0)
    {
      return digits;
    }
    return 64 + __builtin_ctzll(high);
  }
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoCountlOne(T value) noexcept
{
  return NuoCountlZero(static_cast<T>(~value));
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoCountrOne(T value) noexcept
{
  return NuoCountrZero(static_cast<T>(~value));
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr bool NuoHasSingleBit(T value) noexcept
{
  return value != 0 && (value & static_cast<T>(value - 1)) == 0;
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr int NuoBitWidth(T value) noexcept
{
  if (value == 0)
  {
    return 0;
  }
  return static_cast<int>(sizeof(T) * CHAR_BIT) - NuoCountlZero(value);
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr T NuoBitCeil(T value) noexcept
{
  if (value == 0)
  {
    return 1;
  }
  return static_cast<T>(T(1) << NuoBitWidth(static_cast<T>(value - 1)));
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr T NuoBitFloor(T value) noexcept
{
  if (value == 0)
  {
    return 0;
  }
  return static_cast<T>(T(1) << (NuoBitWidth(value) - 1));
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr T NuoRotl(T value, int shift) noexcept
{
  constexpr int digits = static_cast<int>(sizeof(T) * CHAR_BIT);
  const int amount = shift % digits;
  if (amount == 0)
  {
    return value;
  }
  if (amount > 0)
  {
    return static_cast<T>((value << amount) | (value >> (digits - amount)));
  }
  return NuoRotl(value, digits + amount);
}

template <typename T>
  requires std::is_unsigned_v<T>
constexpr T NuoRotr(T value, int shift) noexcept
{
  constexpr int digits = static_cast<int>(sizeof(T) * CHAR_BIT);
  const int amount = shift % digits;
  if (amount == 0)
  {
    return value;
  }
  if (amount > 0)
  {
    return static_cast<T>((value >> amount) | (value << (digits - amount)));
  }
  return NuoRotr(value, digits + amount);
}

} /* namespace nuostl */
