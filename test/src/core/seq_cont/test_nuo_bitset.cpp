#include "core/seq_cont/test_nuo_bitset.hpp"

#include <assert.h>

#include <cstddef>
#include <functional>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/seq_cont/nuo_bitset.hpp"

namespace test
{

namespace
{

using Bits0 = nuostl::NuoBitset<0>;
using Bits1 = nuostl::NuoBitset<1>;
using Bits4 = nuostl::NuoBitset<4>;
using Bits8 = nuostl::NuoBitset<8>;
using Bits63 = nuostl::NuoBitset<63>;
using Bits64 = nuostl::NuoBitset<64>;
using Bits65 = nuostl::NuoBitset<65>;
using Bits130 = nuostl::NuoBitset<130>;

template <typename Exception, typename Function>
void ExpectThrows(Function function)
{
  bool caught = false;
  try
  {
    function();
  }
  catch (const Exception&)
  {
    caught = true;
  }
  assert(caught);
}

} /* namespace */

void TestNuoBitset::TestConstruction()
{
  const Bits8 empty;
  assert(empty.ToString() == "00000000");
  assert(empty.Count() == 0);

  const Bits8 from_value(0xACull);
  assert(from_value.ToString() == "10101100");
  assert(from_value.ToUlong() == 0xACul);

  const Bits8 implicit_value = 0x5ull;
  assert(implicit_value.ToString() == "00000101");

  const Bits8 masked(0x1FFull);
  assert(masked.ToString() == "11111111");

  const Bits8 short_text(std::string("1010"));
  assert(short_text.ToString() == "00001010");

  const Bits4 truncated(std::string("111000"));
  assert(truncated.ToString() == "1110");

  const Bits8 slice(std::string("1010"), 2, 2);
  assert(slice.ToString() == "00000010");

  const Bits8 empty_slice(std::string("1010"), 4);
  assert(empty_slice.ToString() == "00000000");

  const Bits8 custom(std::string("xxoo"), 0, 4, 'o', 'x');
  assert(custom.ToString() == "00001100");
  assert(custom.ToString('o', 'x') == "ooooxxoo");

  const Bits8 from_pointer("1010");
  assert(from_pointer.ToString() == "00001010");

  const Bits8 from_pointer_count("10101010", 4);
  assert(from_pointer_count.ToString() == "00001010");

  const Bits8 copied(from_value);
  assert(copied == from_value);
  Bits8 assigned;
  assigned = copied;
  assert(assigned == from_value);

  ExpectThrows<std::out_of_range>([]()
  {
    const Bits8 invalid(std::string("1010"), 5);
    (void)invalid;
  });
  ExpectThrows<std::invalid_argument>([]()
  {
    const Bits8 invalid(std::string("10x0"));
    (void)invalid;
  });

  const Bits0 empty_bits;
  assert(empty_bits.ToString().empty());
  assert(empty_bits.Size() == 0);
}

void TestNuoBitset::TestElementAccess()
{
  Bits8 bits;
  assert(!bits[0]);
  bits[0] = true;
  assert(bits[0]);
  assert(bits.Test(0));

  bits[2] = bits[0];
  assert(bits[2]);
  assert(~bits[1]);

  bits[2].Flip();
  assert(!bits[2]);
  bits[1] = ~bits[2];
  assert(bits[1]);

  Bits8::reference first = bits[0];
  assert(static_cast<bool>(first));
  first = false;
  assert(!bits[0]);
  bits[0] = first;
  assert(!bits[0]);

  const Bits8& const_bits = bits;
  assert(const_bits[1]);
  static_assert(std::is_same_v<decltype(bits[0]), Bits8::reference>);
  static_assert(std::is_same_v<decltype(const_bits[0]), bool>);

  ExpectThrows<std::out_of_range>([&]()
  {
    (void)bits.Test(8);
  });
}

void TestNuoBitset::TestObservers()
{
  const Bits8 all(std::string("11111111"));
  assert(all.All());
  assert(all.Any());
  assert(!all.None());
  assert(all.Count() == 8);
  assert(all.Size() == 8);

  const Bits8 none;
  assert(!none.All());
  assert(!none.Any());
  assert(none.None());
  assert(none.Count() == 0);

  const Bits8 mixed(std::string("10101010"));
  assert(!mixed.All());
  assert(mixed.Any());
  assert(!mixed.None());
  assert(mixed.Count() == 4);

  const Bits0 empty;
  assert(empty.Size() == 0);
  assert(empty.Count() == 0);
  assert(empty.All());
  assert(empty.None());
  assert(!empty.Any());
}

void TestNuoBitset::TestModifiers()
{
  Bits8 bits;
  bits.Set();
  assert(bits.All());

  bits.Reset(0);
  assert(!bits[0]);
  bits.Set(0, false);
  assert(!bits[0]);
  bits.Set(0);
  assert(bits[0]);

  bits.Flip();
  assert(!bits[0]);
  bits.Flip(0);
  assert(bits[0]);

  bits.Reset();
  assert(bits.None());
  bits.Flip();
  assert(bits.All());

  Bits8 chain;
  chain.Set().Flip(0);
  assert(!chain[0]);
  assert(chain[1]);

  ExpectThrows<std::out_of_range>([&]()
  {
    bits.Set(8);
  });
  ExpectThrows<std::out_of_range>([&]()
  {
    bits.Reset(8);
  });
  ExpectThrows<std::out_of_range>([&]()
  {
    bits.Flip(8);
  });
}

void TestNuoBitset::TestBitwiseOperators()
{
  const Bits8 first(std::string("11001100"));
  const Bits8 second(std::string("10101010"));

  assert((first & second).ToString() == "10001000");
  assert((first | second).ToString() == "11101110");
  assert((first ^ second).ToString() == "01100110");
  assert((~first).ToString() == "00110011");

  assert((first & first) == first);
  assert((first & second) != first);

  Bits8 compound(first);
  compound &= second;
  assert(compound == (first & second));
  compound = first;
  compound |= second;
  assert(compound == (first | second));
  compound = first;
  compound ^= second;
  assert(compound == (first ^ second));
}

void TestNuoBitset::TestShiftOperators()
{
  const Bits8 pattern(std::string("10000001"));
  assert((pattern << 1).ToString() == "00000010");
  assert((pattern >> 1).ToString() == "01000000");
  assert((pattern << 0) == pattern);
  assert((pattern >> 0) == pattern);
  assert((pattern << 8).None());
  assert((pattern >> 8).None());
  assert((pattern << 100).None());
  assert((pattern >> 100).None());

  Bits8 compound(pattern);
  compound <<= 1;
  assert(compound == (pattern << 1));
  compound = pattern;
  compound >>= 1;
  assert(compound == (pattern >> 1));
  compound = pattern;
  compound <<= 8;
  assert(compound.None());
  compound = pattern;
  compound >>= 8;
  assert(compound.None());

  Bits64 full;
  full.Set();
  const Bits64 shifted_left = full << 1;
  const Bits64 shifted_right = full >> 1;
  assert(shifted_left.Count() == 63);
  assert(shifted_right.Count() == 63);
  assert(!shifted_right[63]);
  assert(shifted_right[0]);

  Bits65 wide;
  wide.Set(63);
  wide.Set(64);
  const Bits65 left = wide << 1;
  const Bits65 right = wide >> 1;
  assert(left.Count() == 1);
  assert(left[64]);
  assert(right.Count() == 2);
  assert(right[62]);
  assert(right[63]);

  Bits65 high;
  high.Set(0);
  assert((high << 64).Count() == 1);
  assert((high << 64)[64]);

  Bits65 top;
  top.Set(64);
  assert((top >> 64).Count() == 1);
  assert((top >> 64)[0]);

  Bits130 crossing;
  crossing.Set(63);
  crossing.Set(64);
  const Bits130 cross_left = crossing << 1;
  const Bits130 cross_right = crossing >> 1;
  assert(cross_left.Count() == 2);
  assert(cross_left[64]);
  assert(cross_left[65]);
  assert(cross_right.Count() == 2);
  assert(cross_right[62]);
  assert(cross_right[63]);

  Bits1 single;
  single.Set();
  assert((single << 1).None());
  assert((single >> 1).None());
  assert((single << 0) == single);
}

void TestNuoBitset::TestConversions()
{
  const Bits8 value(std::string("10101010"));
  assert(value.ToUlong() == 170ul);
  assert(value.ToUllong() == 170ull);
  assert(value.ToString() == "10101010");
  assert(value.ToString('0', '1') == "10101010");
  assert(value.ToString('.', '#') == "#.#.#.#.");

  Bits64 full;
  full.Set();
  assert(full.ToUlong() == std::numeric_limits<unsigned long>::max());
  assert(full.ToUllong() == std::numeric_limits<unsigned long long>::max());

  Bits65 high;
  high.Set(64);
  ExpectThrows<std::overflow_error>([&]()
  {
    (void)high.ToUlong();
  });
  ExpectThrows<std::overflow_error>([&]()
  {
    (void)high.ToUllong();
  });

  Bits65 low;
  low.Set(0);
  assert(low.ToUlong() == 1ul);
  assert(low.ToUllong() == 1ull);

  const Bits63 empty63;
  assert(empty63.Count() == 0);
  assert(empty63.Size() == 63);
}

void TestNuoBitset::TestStreams()
{
  std::ostringstream output;
  output << Bits4(3ull);
  assert(output.str() == "0011");

  std::ostringstream width_output;
  width_output.width(6);
  width_output.fill('*');
  width_output << Bits4(3ull);
  assert(width_output.str() == "**0011");

  std::ostringstream empty_output;
  empty_output << Bits0();
  assert(empty_output.str().empty());

  Bits8 extracted;
  std::istringstream exact("1010x");
  exact >> extracted;
  assert(extracted.ToString() == "00001010");
  assert(exact.peek() == 'x');
  assert(!exact.fail());

  Bits8 short_extract;
  std::istringstream short_stream("101");
  short_stream >> short_extract;
  assert(short_extract.ToString() == "00000101");
  assert(short_stream.eof());

  Bits8 long_extract;
  std::istringstream long_stream("101010101111");
  long_stream >> long_extract;
  assert(long_extract.ToString() == "10101010");
  assert(long_stream.peek() == '1');

  Bits8 spaces_extract;
  std::istringstream spaces_stream("   101");
  spaces_stream >> spaces_extract;
  assert(spaces_extract.ToString() == "00000101");

  Bits8 failed(0xFFull);
  std::istringstream failed_stream("x");
  failed_stream >> failed;
  assert(failed_stream.fail());
  assert(failed.ToString() == "11111111");

  Bits8 empty_stream_bits;
  std::istringstream empty_stream("");
  empty_stream >> empty_stream_bits;
  assert(empty_stream.fail());
  assert(empty_stream.eof());
  assert(empty_stream_bits.None());

  Bits0 empty_bits;
  std::istringstream zero_stream("1");
  zero_stream >> empty_bits;
  assert(!zero_stream.fail());

  std::wostringstream wide_output;
  wide_output << Bits4(3ull);
  assert(wide_output.str() == L"0011");

  Bits8 wide_input;
  std::wistringstream wide_stream(L"1010");
  wide_stream >> wide_input;
  assert(wide_input.ToString() == "00001010");
}

void TestNuoBitset::TestHash()
{
  const Bits8 first(std::string("10101010"));
  const Bits8 second(std::string("10101010"));
  const Bits8 other(std::string("01010101"));

  std::hash<Bits8> hasher;
  assert(hasher(first) == hasher(second));
  (void)hasher(other);
  std::hash<Bits0> empty_hasher;
  (void)empty_hasher(Bits0());
}

void TestNuoBitset::TestConstexprOperations()
{
  constexpr Bits8 bits(5ull);
  static_assert(bits.Size() == 8);
  static_assert(bits.Count() == 2);
  static_assert(bits.Test(0));
  static_assert(!bits.Test(1));
  static_assert(bits[0]);
  static_assert(!bits[1]);
  static_assert(bits.Any());
  static_assert(!bits.None());
  static_assert(!bits.All());
  static_assert((bits << 1).Count() == 2);
  static_assert((bits >> 1).Count() == 1);
  static_assert((~bits).Count() == 6);
  static_assert((bits | Bits8()) == bits);
  static_assert((bits & Bits8()) == Bits8());
  static_assert(bits.ToUlong() == 5ul);
  static_assert(bits.ToUllong() == 5ull);
  static_assert(Bits0().All());
  static_assert(Bits0().None());
  static_assert(!Bits0().Any());
}

void TestNuoBitset::TestSuite()
{
  TestConstruction();
  TestElementAccess();
  TestObservers();
  TestModifiers();
  TestBitwiseOperators();
  TestShiftOperators();
  TestConversions();
  TestStreams();
  TestHash();
  TestConstexprOperations();
}

} /* namespace test */
