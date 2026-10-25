#include "extra/test_nuo_big_integer.hpp"

#include <algorithm>
#include <climits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "extra/nuo_big_integer.hpp"
#include "test_framework.hpp"

namespace test
{

namespace
{

/* Deterministic pseudo random generator for the differential checks. */
class Random
{
 public:
  explicit Random(unsigned int seed) : engine_(seed)
  {
  }

  unsigned long long Next()
  {
    return engine_();
  }

  long long NextSigned(long long magnitude)
  {
    std::uniform_int_distribution<long long> distribution(-magnitude,
                                                          magnitude);
    return distribution(engine_);
  }

 private:
  std::mt19937_64 engine_;
};

std::string Repeat(char digit, std::size_t count)
{
  return std::string(count, digit);
}

} /* namespace */

void TestNuoBigInteger::TestConstruction()
{
  const nuostl::NuoBigInteger zero;
  NUO_TEST_CHECK(zero.IsZero());
  NUO_TEST_CHECK(!zero.IsNegative());
  NUO_TEST_CHECK(zero.ToString() == "0");
  NUO_TEST_CHECK(zero.DigitCount() == 1);

  NUO_TEST_CHECK(nuostl::NuoBigInteger(0).ToString() == "0");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(7).ToString() == "7");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(-7).ToString() == "-7");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(1000000000).ToString() == "1000000000");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(-1000000001).ToString() ==
                 "-1000000001");

  NUO_TEST_CHECK(nuostl::NuoBigInteger(42u).ToString() == "42");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(42l).ToString() == "42");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(42ul).ToString() == "42");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(42ll).ToString() == "42");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(42ull).ToString() == "42");

  NUO_TEST_CHECK(nuostl::NuoBigInteger(INT_MIN).ToString() ==
                 std::to_string(INT_MIN));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(INT_MAX).ToString() ==
                 std::to_string(INT_MAX));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(LLONG_MIN).ToString() ==
                 std::to_string(LLONG_MIN));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(LLONG_MAX).ToString() ==
                 std::to_string(LLONG_MAX));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(ULLONG_MAX).ToString() ==
                 std::to_string(ULLONG_MAX));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(ULONG_MAX).ToString() ==
                 std::to_string(ULONG_MAX));

  const nuostl::NuoBigInteger copied(zero);
  NUO_TEST_CHECK(copied.IsZero());
  const nuostl::NuoBigInteger moved(nuostl::NuoBigInteger(-5));
  NUO_TEST_CHECK(moved.ToString() == "-5");
}

void TestNuoBigInteger::TestStringParsing()
{
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("0")).ToString() == "0");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("-0")).ToString() == "0");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("+0")).ToString() == "0");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("000123")).ToString() ==
                 "123");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("-000123")).ToString() ==
                 "-123");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("123456789")).ToString() ==
                 "123456789");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("1000000000")).ToString() ==
                 "1000000000");
  const std::string twenty_digits = "98765432109876543210";
  NUO_TEST_CHECK(nuostl::NuoBigInteger(twenty_digits).ToString() ==
                 twenty_digits);
  NUO_TEST_CHECK(nuostl::NuoBigInteger("-" + twenty_digits).ToString() ==
                 "-" + twenty_digits);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("+42")).ToString() == "42");
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger("31415926535897932384626433832795").ToString() ==
      "31415926535897932384626433832795");

  const std::vector<std::string> invalid = {"",    "+",   "-",    " 12", "12 ",
                                            "1a",  "a1",  "--1",  "++1", "-+1",
                                            "+-1", "1.5", "0x10", "١٢٣"};
  for (const std::string& text : invalid)
  {
    bool caught = false;
    try
    {
      nuostl::NuoBigInteger value(text);
      (void)value;
    }
    catch (const std::invalid_argument&)
    {
      caught = true;
    }
    NUO_TEST_CHECK(caught);
  }

  /* Parsing round trips through ToString for long inputs. */
  Random random(2024);
  for (int trial = 0; trial < 32; ++trial)
  {
    std::string text;
    const int length = 1 + static_cast<int>(random.Next() % 200);
    if (random.Next() % 2 == 0)
    {
      text.push_back('-');
    }
    text.push_back(static_cast<char>('1' + random.Next() % 9));
    for (int index = 1; index < length; ++index)
    {
      text.push_back(static_cast<char>('0' + random.Next() % 10));
    }
    const nuostl::NuoBigInteger value(text);
    NUO_TEST_CHECK(value.ToString() == text);
  }
}

void TestNuoBigInteger::TestConversion()
{
  NUO_TEST_CHECK(nuostl::NuoBigInteger(0).ToLongLong() == 0);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(-1).ToLongLong() == -1);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(LLONG_MAX).ToLongLong() == LLONG_MAX);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(LLONG_MIN).ToLongLong() == LLONG_MIN);

  bool caught = false;
  try
  {
    nuostl::NuoBigInteger(std::string("9223372036854775808")).ToLongLong();
  }
  catch (const std::overflow_error&)
  {
    caught = true;
  }
  NUO_TEST_CHECK(caught);

  caught = false;
  try
  {
    nuostl::NuoBigInteger(std::string("-9223372036854775809")).ToLongLong();
  }
  catch (const std::overflow_error&)
  {
    caught = true;
  }
  NUO_TEST_CHECK(caught);

  NUO_TEST_CHECK(nuostl::NuoBigInteger(9).DigitCount() == 1);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(10).DigitCount() == 2);
  NUO_TEST_CHECK(nuostl::NuoBigInteger(-100).DigitCount() == 3);
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger(std::string(Repeat('9', 100))).DigitCount() == 100);
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger(std::string("1" + Repeat('0', 100))).DigitCount() ==
      101);
}

void TestNuoBigInteger::TestComparison()
{
  const nuostl::NuoBigInteger zero;
  const nuostl::NuoBigInteger small(10);
  const nuostl::NuoBigInteger big(1000000000000LL);
  const nuostl::NuoBigInteger negative(-10);

  NUO_TEST_CHECK(zero == nuostl::NuoBigInteger(0));
  NUO_TEST_CHECK(zero != small);
  NUO_TEST_CHECK(negative < zero);
  NUO_TEST_CHECK(negative < small);
  NUO_TEST_CHECK(small < big);
  NUO_TEST_CHECK(big > small);
  NUO_TEST_CHECK(small <= small);
  NUO_TEST_CHECK(small >= small);
  NUO_TEST_CHECK(zero == nuostl::NuoBigInteger(std::string("+0")));
  NUO_TEST_CHECK(nuostl::NuoBigInteger(std::string("-0")) == zero);

  Random random(7);
  for (int trial = 0; trial < 512; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);
    NUO_TEST_CHECK((big_left < big_right) == (left < right));
    NUO_TEST_CHECK((big_left <= big_right) == (left <= right));
    NUO_TEST_CHECK((big_left > big_right) == (left > right));
    NUO_TEST_CHECK((big_left >= big_right) == (left >= right));
    NUO_TEST_CHECK((big_left == big_right) == (left == right));
    NUO_TEST_CHECK((big_left != big_right) == (left != right));
  }

  /* Sorting the big values must match sorting the built in values. */
  std::vector<long long> natives;
  std::vector<nuostl::NuoBigInteger> values;
  for (int trial = 0; trial < 64; ++trial)
  {
    const long long value = random.NextSigned(1000000000000LL);
    natives.push_back(value);
    values.push_back(nuostl::NuoBigInteger(value));
  }
  std::sort(natives.begin(), natives.end());
  std::sort(values.begin(), values.end());
  NUO_TEST_CHECK(values.size() == natives.size());
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    NUO_TEST_CHECK(values[index].ToString() == std::to_string(natives[index]));
  }
}

void TestNuoBigInteger::TestAdditionAndSubtraction()
{
  Random random(11);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    NUO_TEST_CHECK((big_left + big_right).ToString() ==
                   std::to_string(left + right));
    NUO_TEST_CHECK((big_left - big_right).ToString() ==
                   std::to_string(left - right));
    NUO_TEST_CHECK((-big_left).ToString() == std::to_string(-left));

    nuostl::NuoBigInteger accumulated(left);
    accumulated += big_right;
    NUO_TEST_CHECK(accumulated.ToString() == std::to_string(left + right));
    accumulated -= big_right;
    NUO_TEST_CHECK(accumulated.ToString() == std::to_string(left));

    /* Borrowing across many base digits. */
    const nuostl::NuoBigInteger power(std::string("1" + Repeat('0', 40)));
    NUO_TEST_CHECK((power - nuostl::NuoBigInteger(1)).ToString() ==
                   Repeat('9', 40));
    NUO_TEST_CHECK((power + nuostl::NuoBigInteger(1)).ToString() ==
                   "1" + Repeat('0', 39) + "1");
    NUO_TEST_CHECK((nuostl::NuoBigInteger(1) - power).ToString() ==
                   "-" + Repeat('9', 40));
  }

  const nuostl::NuoBigInteger zero;
  NUO_TEST_CHECK((zero - zero).ToString() == "0");
  NUO_TEST_CHECK((zero + zero).IsZero());
  NUO_TEST_CHECK((zero - nuostl::NuoBigInteger(5)).ToString() == "-5");
}

void TestNuoBigInteger::TestMultiplication()
{
  Random random(13);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    NUO_TEST_CHECK((big_left * big_right).ToString() ==
                   std::to_string(left * right));

    nuostl::NuoBigInteger accumulated(left);
    accumulated *= big_right;
    NUO_TEST_CHECK(accumulated.ToString() == std::to_string(left * right));
  }

  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(0) * nuostl::NuoBigInteger(0)).IsZero());
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(-1) * nuostl::NuoBigInteger(0)).IsZero());
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(-2) * nuostl::NuoBigInteger(-3)).ToString() ==
      "6");
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(-2) * nuostl::NuoBigInteger(3)).ToString() ==
      "-6");

  /* (10^50 - 1)^2 has a well known decimal expansion. */
  const std::string nines = Repeat('9', 50);
  const nuostl::NuoBigInteger all_nines(nines);
  const nuostl::NuoBigInteger squared = all_nines * all_nines;
  const std::string expected = Repeat('9', 49) + "8" + Repeat('0', 49) + "1";
  NUO_TEST_CHECK(squared.ToString() == expected);
}

void TestNuoBigInteger::TestDivisionAndRemainder()
{
  Random random(17);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    long long right = random.NextSigned(1000000000);
    if (right == 0)
    {
      right = 1;
    }
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    NUO_TEST_CHECK((big_left / big_right).ToString() ==
                   std::to_string(left / right));
    NUO_TEST_CHECK((big_left % big_right).ToString() ==
                   std::to_string(left % right));

    nuostl::NuoBigInteger quotient(left);
    quotient /= big_right;
    NUO_TEST_CHECK(quotient.ToString() == std::to_string(left / right));
    nuostl::NuoBigInteger remainder(left);
    remainder %= big_right;
    NUO_TEST_CHECK(remainder.ToString() == std::to_string(left % right));

    /* Division identity and the remainder magnitude bound. */
    const nuostl::NuoBigInteger rebuilt = quotient * big_right + remainder;
    NUO_TEST_CHECK(rebuilt == big_left);
    if (!remainder.IsZero())
    {
      NUO_TEST_CHECK(remainder.Abs() < big_right.Abs());
    }
  }

  bool caught = false;
  try
  {
    nuostl::NuoBigInteger(1) / nuostl::NuoBigInteger(0);
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  NUO_TEST_CHECK(caught);

  caught = false;
  try
  {
    nuostl::NuoBigInteger(1) % nuostl::NuoBigInteger(0);
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  NUO_TEST_CHECK(caught);

  /* Long division across many base digits. */
  const std::string long_dividend =
      "123456789012345678901234567890123456789012345678901234567890";
  const nuostl::NuoBigInteger dividend(long_dividend);
  const nuostl::NuoBigInteger divisor(std::string("98765432109876543210"));
  const nuostl::NuoBigInteger quotient = dividend / divisor;
  const nuostl::NuoBigInteger remainder = dividend % divisor;
  NUO_TEST_CHECK(quotient.ToString() ==
                 "1249999988609375000154882812384313964845");
  NUO_TEST_CHECK(remainder.ToString() == "31711154403171115440");
  NUO_TEST_CHECK(quotient * divisor + remainder == dividend);
  NUO_TEST_CHECK(remainder < divisor);

  /* In the current base 1e9 layout these dividends are [5, 0, 0, 1] and
     [5, 0, 0, 0, 1]. A zero block after the most significant block must not
     make the running remainder appear larger than it is. */
  const nuostl::NuoBigInteger zero_gap_dividend(std::string("1") +
                                                Repeat('0', 26) + "5");
  NUO_TEST_CHECK((zero_gap_dividend / nuostl::NuoBigInteger(10)).ToString() ==
                 std::string("1") + Repeat('0', 26));
  NUO_TEST_CHECK((zero_gap_dividend % nuostl::NuoBigInteger(10)).ToString() ==
                 "5");

  const nuostl::NuoBigInteger multi_block_gap_dividend(std::string("1") +
                                                       Repeat('0', 35) + "5");
  NUO_TEST_CHECK(
      (multi_block_gap_dividend / nuostl::NuoBigInteger(10)).ToString() ==
      std::string("1") + Repeat('0', 35));
  NUO_TEST_CHECK(
      (multi_block_gap_dividend % nuostl::NuoBigInteger(10)).ToString() == "5");

  /* The product exactly one below the dividend forces a borrow through two
     zero blocks in the subtraction used by long division. */
  const nuostl::NuoBigInteger borrow_dividend(std::string("1") +
                                              Repeat('0', 18));
  const nuostl::NuoBigInteger borrow_divisor(std::string("999999999999999999"));
  NUO_TEST_CHECK((borrow_dividend / borrow_divisor).ToString() == "1");
  NUO_TEST_CHECK((borrow_dividend % borrow_divisor).ToString() == "1");

  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(5) / nuostl::NuoBigInteger(7)).IsZero());
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(5) % nuostl::NuoBigInteger(7)).ToString() == "5");
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(-7) / nuostl::NuoBigInteger(2)).ToString() ==
      "-3");
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(-7) % nuostl::NuoBigInteger(2)).ToString() ==
      "-1");
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(7) / nuostl::NuoBigInteger(-2)).ToString() ==
      "-3");
  NUO_TEST_CHECK(
      (nuostl::NuoBigInteger(7) % nuostl::NuoBigInteger(-2)).ToString() == "1");
}

void TestNuoBigInteger::TestIncrementAndSign()
{
  nuostl::NuoBigInteger value(9);
  NUO_TEST_CHECK((++value).ToString() == "10");
  NUO_TEST_CHECK((value++).ToString() == "10");
  NUO_TEST_CHECK(value.ToString() == "11");
  NUO_TEST_CHECK((--value).ToString() == "10");
  NUO_TEST_CHECK((value--).ToString() == "10");
  NUO_TEST_CHECK(value.ToString() == "9");

  nuostl::NuoBigInteger zero;
  --zero;
  NUO_TEST_CHECK(zero.ToString() == "-1");
  ++zero;
  NUO_TEST_CHECK(zero.IsZero());

  NUO_TEST_CHECK(nuostl::NuoBigInteger(-15).Abs().ToString() == "15");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(15).Abs().ToString() == "15");
  NUO_TEST_CHECK(nuostl::NuoBigInteger(0).Abs().IsZero());
  NUO_TEST_CHECK(nuostl::NuoBigInteger(-15).IsNegative());
  NUO_TEST_CHECK(!nuostl::NuoBigInteger(-15).Abs().IsNegative());

  NUO_TEST_CHECK(
      nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(0), 0).ToString() ==
      "1");
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(0), 5).ToString() ==
      "0");
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(2), 100).ToString() ==
      "1267650600228229401496703205376");
  NUO_TEST_CHECK(
      nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(-2), 3).ToString() ==
      "-8");

  NUO_TEST_CHECK(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(12),
                                            nuostl::NuoBigInteger(18))
                     .ToString() == "6");
  NUO_TEST_CHECK(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(-12),
                                            nuostl::NuoBigInteger(18))
                     .ToString() == "6");
  NUO_TEST_CHECK(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(0),
                                            nuostl::NuoBigInteger(5))
                     .ToString() == "5");
  NUO_TEST_CHECK(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(0),
                                            nuostl::NuoBigInteger(0))
                     .IsZero());

  Random random(23);
  for (int trial = 0; trial < 256; ++trial)
  {
    const long long left = random.NextSigned(100000);
    long long right = random.NextSigned(100000);
    if (left == 0 && right == 0)
    {
      right = 1;
    }
    long long expected = left < 0 ? -left : left;
    long long candidate = right < 0 ? -right : right;
    while (candidate != 0)
    {
      const long long next = expected % candidate;
      expected = candidate;
      candidate = next;
    }
    NUO_TEST_CHECK(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(left),
                                              nuostl::NuoBigInteger(right))
                       .ToString() == std::to_string(expected));
  }
}

void TestNuoBigInteger::TestLargeValues()
{
  /* 100! computed by repeated multiplication and compared with the exact
     decimal expansion. */
  nuostl::NuoBigInteger factorial(1);
  for (int factor = 2; factor <= 100; ++factor)
  {
    factorial *= nuostl::NuoBigInteger(factor);
  }
  NUO_TEST_CHECK(
      factorial.ToString() ==
      "93326215443944152681699238856266700490715968264381621468592963895217"
      "59999322991560894146397615651828625369792082722375825118521091686400"
      "0000000000000000000000");

  /* Division must invert multiplication for large operands. */
  Random random(29);
  for (int trial = 0; trial < 16; ++trial)
  {
    std::string left_text("1");
    std::string right_text("1");
    for (int index = 0; index < 120; ++index)
    {
      left_text.push_back(static_cast<char>('0' + random.Next() % 10));
      right_text.push_back(static_cast<char>('0' + random.Next() % 10));
    }
    const nuostl::NuoBigInteger left(left_text);
    const nuostl::NuoBigInteger right(right_text);

    const nuostl::NuoBigInteger product = left * right;
    NUO_TEST_CHECK(product / left == right);
    NUO_TEST_CHECK(product / right == left);
    NUO_TEST_CHECK((product % left).IsZero());
    NUO_TEST_CHECK((product % right).IsZero());

    const nuostl::NuoBigInteger quotient = left / right;
    const nuostl::NuoBigInteger remainder = left % right;
    NUO_TEST_CHECK(quotient * right + remainder == left);
    if (!remainder.IsZero())
    {
      NUO_TEST_CHECK(remainder < right);
    }
  }
}

void TestNuoBigInteger::TestStreamOutput()
{
  const nuostl::NuoBigInteger value(std::string("-12345678901234567890123456"));
  std::ostringstream stream;
  stream << value;
  NUO_TEST_CHECK(stream.str() == value.ToString());
  NUO_TEST_CHECK(stream.str() == "-12345678901234567890123456");

  std::ostringstream zero_stream;
  zero_stream << nuostl::NuoBigInteger();
  NUO_TEST_CHECK(zero_stream.str() == "0");
}

void TestNuoBigInteger::TestSuite()
{
  TestConstruction();
  TestStringParsing();
  TestConversion();
  TestComparison();
  TestAdditionAndSubtraction();
  TestMultiplication();
  TestDivisionAndRemainder();
  TestIncrementAndSign();
  TestLargeValues();
  TestStreamOutput();
}

} /* namespace test */
