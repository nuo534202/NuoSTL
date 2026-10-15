#include "core/bit/test_nuo_bit.hpp"

#include <assert.h>

#include <bit>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/bit/nuo_bit.hpp"
#include "nuo_typedefs.hpp"

namespace test
{

namespace
{

struct Pair
{
  std::uint32_t first;
  std::uint32_t second;
};

enum class Flags : std::uint16_t
{
  value = 0
};

template <typename T>
concept HasNuoPopcount = requires(T value)
{
  nuostl::NuoPopcount(value);
};

template <typename T>
concept HasNuoCountlZero = requires(T value)
{
  nuostl::NuoCountlZero(value);
};

template <typename To, typename From>
concept HasNuoBitCast = requires(From value)
{
  nuostl::NuoBitCast<To>(value);
};

template <typename T>
void CheckUnsignedFunctions(T value)
{
  assert(nuostl::NuoPopcount(value) == std::popcount(value));
  assert(nuostl::NuoCountlZero(value) == std::countl_zero(value));
  assert(nuostl::NuoCountrZero(value) == std::countr_zero(value));
  assert(nuostl::NuoCountlOne(value) == std::countl_one(value));
  assert(nuostl::NuoCountrOne(value) == std::countr_one(value));
  assert(nuostl::NuoHasSingleBit(value) == std::has_single_bit(value));
  assert(nuostl::NuoBitWidth(value) == std::bit_width(value));
  assert(nuostl::NuoBitFloor(value) == std::bit_floor(value));
  assert(nuostl::NuoRotl(value, 3) == std::rotl(value, 3));
  assert(nuostl::NuoRotl(value, -5) == std::rotl(value, -5));
  assert(nuostl::NuoRotr(value, 7) == std::rotr(value, 7));
  assert(nuostl::NuoRotr(value, -4) == std::rotr(value, -4));

  if (value <= static_cast<T>(std::numeric_limits<T>::max() / 2 + 1))
  {
    assert(nuostl::NuoBitCeil(value) == std::bit_ceil(value));
  }
}

template <typename T>
void CheckType()
{
  CheckUnsignedFunctions<T>(0);
  CheckUnsignedFunctions<T>(1);
  CheckUnsignedFunctions<T>(2);
  CheckUnsignedFunctions<T>(3);
  CheckUnsignedFunctions<T>(static_cast<T>(1) << 5);
  CheckUnsignedFunctions<T>(static_cast<T>(0x55));
  CheckUnsignedFunctions<T>(static_cast<T>(0x80));
  CheckUnsignedFunctions<T>(static_cast<T>(-1));
}

} /* namespace */

void TestNuoBit::TestEndian()
{
  static_assert((nuostl::NuoEndian::native == nuostl::NuoEndian::little) ==
                (std::endian::native == std::endian::little));
  static_assert((nuostl::NuoEndian::native == nuostl::NuoEndian::big) ==
                (std::endian::native == std::endian::big));
  static_assert(nuostl::NuoEndian::little != nuostl::NuoEndian::big);
}

void TestNuoBit::TestBitCast()
{
  static_assert(nuostl::NuoBitCast<std::uint32_t>(1.0f) ==
                std::bit_cast<std::uint32_t>(1.0f));
  static_assert(nuostl::NuoBitCast<float>(std::uint32_t(0x3F800000u)) ==
                1.0f);
  static_assert(nuostl::NuoBitCast<std::uint64_t>(1.0) ==
                std::bit_cast<std::uint64_t>(1.0));
  static_assert(nuostl::NuoBitCast<std::uint8_t>(std::uint8_t(0xABu)) ==
                0xABu);

  const std::uint64_t bits = 0x1122334455667788ull;
  const Pair pair = nuostl::NuoBitCast<Pair>(bits);
  assert(nuostl::NuoBitCast<std::uint64_t>(pair) == bits);
  assert(sizeof(pair) == sizeof(bits));
}

void TestNuoBit::TestByteswap()
{
  static_assert(nuostl::NuoByteswap<std::uint8_t>(0xABu) == 0xABu);
  static_assert(nuostl::NuoByteswap<std::uint16_t>(0x1234u) == 0x3412u);
  static_assert(nuostl::NuoByteswap<std::uint32_t>(0x12345678u) ==
                0x78563412u);
  static_assert(nuostl::NuoByteswap<std::uint64_t>(0x0123456789ABCDEFull) ==
                0xEFCDAB8967452301ull);
  static_assert(nuostl::NuoByteswap<std::int32_t>(0x12345678) ==
                static_cast<std::int32_t>(0x78563412u));
  static_assert(nuostl::NuoByteswap(Flags(0x1234)) == Flags(0x3412));
  static_assert(nuostl::NuoByteswap(nuostl::uint128(1)) ==
                (nuostl::uint128(1) << 120));
  static_assert(
    nuostl::NuoByteswap(
      nuostl::NuoByteswap(nuostl::uint128(0x0123456789ABCDEFull))) ==
    nuostl::uint128(0x0123456789ABCDEFull));
}

void TestNuoBit::TestStandardUnsignedFunctions()
{
  CheckType<std::uint8_t>();
  CheckType<std::uint16_t>();
  CheckType<std::uint32_t>();
  CheckType<std::uint64_t>();
}

void TestNuoBit::TestExtendedWidthFunctions()
{
  using Uint128 = nuostl::uint128;
  static_assert(nuostl::NuoPopcount(Uint128(0)) == 0);
  static_assert(nuostl::NuoPopcount(Uint128(0xFFFFFFFFFFFFFFFFull)) == 64);
  static_assert(nuostl::NuoPopcount(Uint128(1) << 64) == 1);
  static_assert(nuostl::NuoPopcount((Uint128(1) << 127) | Uint128(1)) == 2);
  static_assert(nuostl::NuoCountlZero(Uint128(0)) == 128);
  static_assert(nuostl::NuoCountlZero(Uint128(1)) == 127);
  static_assert(nuostl::NuoCountrZero(Uint128(0)) == 128);
  static_assert(nuostl::NuoCountrZero(Uint128(1) << 64) == 64);
  static_assert(nuostl::NuoCountlOne(Uint128(0)) == 0);
  static_assert(nuostl::NuoCountrOne(~Uint128(0)) == 128);
  static_assert(nuostl::NuoHasSingleBit(Uint128(1) << 64));
  static_assert(!nuostl::NuoHasSingleBit(Uint128(3)));
  static_assert(nuostl::NuoBitWidth(Uint128(1) << 64) == 65);
  static_assert(nuostl::NuoBitFloor(Uint128(1) << 64) == (Uint128(1) << 64));
  static_assert(nuostl::NuoBitCeil(Uint128(1) << 64) == (Uint128(1) << 64));
  static_assert(nuostl::NuoBitCeil(Uint128(0)) == Uint128(1));
  static_assert(nuostl::NuoRotl(Uint128(1), 1) == Uint128(2));
  static_assert(nuostl::NuoRotr(Uint128(2), 1) == Uint128(1));
  static_assert(nuostl::NuoRotl(Uint128(1) << 127, 1) == Uint128(1));
  static_assert(nuostl::NuoRotl(Uint128(1), -1) ==
                (Uint128(1) << 127));
}

void TestNuoBit::TestConstraints()
{
  static_assert(std::is_same_v<
    decltype(nuostl::NuoPopcount<std::uint32_t>(1u)), int>);
  static_assert(std::is_same_v<
    decltype(nuostl::NuoBitCeil<std::uint32_t>(1u)), std::uint32_t>);
  static_assert(std::is_same_v<
    decltype(nuostl::NuoRotl<std::uint32_t>(1u, 1)), std::uint32_t>);

  static_assert(HasNuoPopcount<unsigned int>);
  static_assert(!HasNuoPopcount<int>);
  static_assert(HasNuoCountlZero<unsigned int>);
  static_assert(!HasNuoCountlZero<int>);
  static_assert(HasNuoBitCast<std::uint64_t, double>);
  static_assert(!HasNuoBitCast<std::uint64_t, float>);
}

void TestNuoBit::TestSuite()
{
  TestEndian();
  TestBitCast();
  TestByteswap();
  TestStandardUnsignedFunctions();
  TestExtendedWidthFunctions();
  TestConstraints();
}

} /* namespace test */
